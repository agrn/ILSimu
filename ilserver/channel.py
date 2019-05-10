import asyncio

import numpy as np

from complex_helpers import flat_list_to_complex
from constants import CARRIER_THRESHOLD


class Channel:
    def __init__(self, num):
        # Represents the factor by which the values of this channel should be
        # multiplied by to be equivalent to those of the reference channel.
        # Should be 1 for the reference channel.
        self.level = 1

        # The number of samples to remove to be time synced to the
        # reference channel.  When this number is positive, the start
        # of this channel came earlier than the start of the reference
        # channel.  When it is negative, fake samples should be added.
        # After the samples have been dropped or added, the offset
        # should be reset to 0.  Should be 0 for the reference
        # channel.
        self.offset = 0

        # Stores all of the samples necessary for synchronisation.  For the
        # reference channel, stores everything until all channels have been
        # synchronised.
        self.buffer = []

        # Indicates whether this channel is synchronised with the reference
        # channel.  Should be true for the reference channel.
        self.synchronised = False

        # Indicates whether the start of the carrier has been found or not.
        self.start_found = False

        # The position of the start of the carrier in the buffer, divided by
        # two.
        self.start_at = 0

        self.median = 0
        self.phase_delta = 0

        self.num = num

    def __len__(self):
        """Returns the amount of IQ samples in the buffer."""
        return len(self.buffer)

    def put(self, buffer):
        """Insert buffer at the end of the channel buffer.."""
        self.buffer += flat_list_to_complex(buffer)

    def last_modulus(self):
        """Returns the modulus of the last sample in the buffer."""
        return abs(self.buffer[-1])

    def find_start(self):
        """Finds the start of the carrier in the buffer.  If found,
        threshold_found is set to True, and threshold_at is set to the position
        of the IQ sample.  To get the position of the sample in the list, it
        should then be multiplied by two.

        Due to the operating mode of the generator, when the carrier starts,
        there is a peak current.  This functions searches for this peak, and
        considers it as the start of the carrier.  The peak should exceed
        CARRIER_THRESHOLD.

        This allow synchronisation with the reference channel in time, but not
        in intensity."""
        i, m = 0, 0

        # Don't run again if the start has already been found.
        if not self.start_found:
            # Basically a max function, but with modulus of IQ samples and a
            # threshold.
            for i, modulus in enumerate(np.absolute(self.buffer)):
                if modulus > CARRIER_THRESHOLD and modulus > m:
                    self.start_found = True
                    self.start_at = i
                    m = modulus

            if self.start_found:
                self.__median_of_buffer()

    def get_index_to_sync(self):
        start = self.offset
        self.offset = 0
        return start

    def __median_of_buffer(self):
        moduli = np.absolute(self.buffer[self.start_at:])
        self.median = np.median(moduli)

    def __delta_of_phase(self, reference):
        ref_phase = np.array(np.angle(reference.buffer[reference.start_at:]))
        ch_phase = np.array(np.angle(self.buffer[self.start_at:]))
        length = min(len(ref_phase), len(ch_phase))
        self.phase_delta = np.median(ref_phase[:length] - ch_phase[:length])

    async def process_buffer(self, buffer, reference):
        if not self.synchronised:
            self.put(buffer)

            if reference is not None and len(reference) > 0 and \
               reference.start_found:
                self.__try_to_synchronise(reference)
        else:
            i = self.get_index_to_sync()
            if i > 0:
                self.buffer += [0j] * i
                self.put(buffer)
            else:
                self.put(buffer[-2 * i:])

    def __try_to_synchronise(self, reference):
        c_last_mod = self.last_modulus()
        r_last_mod = reference.last_modulus()

        if c_last_mod > CARRIER_THRESHOLD and r_last_mod > CARRIER_THRESHOLD:
            self.find_start()
            self.__delta_of_phase(reference)

            self.level = reference.median / self.median

            self.offset = reference.start_at - self.start_at
            self.synchronised = True

            print("Channel", self.num, self.offset, self.level,
                  self.phase_delta)


class ReferenceChannel(Channel):
    def __init__(self):
        super(ReferenceChannel, self).__init__(0)
        self.synchronised = True
        self.mutex = asyncio.Lock()

    async def find_start(self, *args, **kwargs):
        with (await self.mutex):
            super(ReferenceChannel, self).find_start(*args, **kwargs)

    async def process_buffer(self, buffer, reference):
        with (await self.mutex):
            assert reference == self

            self.put(buffer)
            if self.start_found:
                self.start_found = \
                    np.average(np.absolute(buffer)) >= CARRIER_THRESHOLD / 2
            else:
                # bypass the mutex as it is already acquired
                super(ReferenceChannel, self).find_start()
