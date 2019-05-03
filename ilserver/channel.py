from statistics import median

import asyncio

from complex_helpers import modulus_at
from constants import CARRIER_THRESHOLD


class Channel:
    def __init__(self):
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
        self.processed_buffer = []

        # Indicates whether this channel is synchronised with the reference
        # channel.  Should be true for the reference channel.
        self.synchronised = False

        # Indicates whether the start of the carrier has been found or not.
        self.start_found = False

        # The position of the start of the carrier in the buffer, divided by
        # two.
        self.start_at = 0

        self.processed_until = 0

    def __len__(self):
        """Returns the amount of IQ samples in the buffer."""
        return len(self.buffer) // 2

    def put(self, buffer):
        """Insert buffer at the end of the channel buffer.."""
        self.buffer += buffer

    def last_modulus(self):
        """Returns the modulus of the last sample in the buffer."""
        return modulus_at(self.buffer, -2)

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
            while i < len(self.buffer):
                modulus = modulus_at(self.buffer, i)
                if modulus > CARRIER_THRESHOLD and modulus > m:
                    self.start_found = True
                    self.start_at = i // 2
                    m = modulus
                i += 2

    def get_index_to_sync(self):
        start = self.offset * -2
        self.offset = 0
        return start


    async def process_buffer(self, buffer, reference):
        self.buffer += buffer

        if reference is None or len(reference) == 0:
            return

        if not self.synchronised:
            await self.__try_to_synchronise(reference)
        else:
            pass

    async def __try_to_synchronise(self, reference):
        c_last_mod = self.last_modulus()
        r_last_mod = reference.last_modulus()

        if c_last_mod > CARRIER_THRESHOLD and r_last_mod > CARRIER_THRESHOLD:
            await reference.find_start()
            self.find_start()

            ref_moduli = []
            chan_moduli = []

            i = reference.start_at * 2
            while i < len(reference.buffer):
                ref_moduli.append(modulus_at(reference.buffer, i))
                i += 2

            i = self.start_at * 2
            while i < len(self.buffer):
                chan_moduli.append(modulus_at(self.buffer, i))
                i += 2

            m_ref = median(ref_moduli)
            m_chan = median(chan_moduli)

            self.level = m_ref / m_chan

            self.offset = reference.start_at - self.start_at
            self.synchronised = True

            print(self.offset, self.level)


class ReferenceChannel(Channel):
    def __init__(self):
        super(ReferenceChannel, self).__init__()
        self.synchronised = True
        self.mutex = asyncio.Lock()

    async def find_start(self, *args, **kwargs):
        with (await self.mutex):
            super(ReferenceChannel, self).find_start(*args, **kwargs)

    async def process_buffer(self, buffer, reference):
        with (await self.mutex):
            assert reference == self
            self.buffer += buffer
