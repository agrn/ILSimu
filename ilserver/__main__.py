#!/usr/bin/env python3

from contextlib import suppress
from statistics import median

import array
import asyncio
import struct


BASE_CHANNEL = 10000     # First port to listen on
CHANNEL_AMOUNT = 2       # Amount of servers to open.  The first server will
                         # have the port BASE_CHANNEL, the second BASE_CHANNEL +
                         # 1, etc.
CARRIER_THRESHOLD = 1000 # Above this value, the carrier is supposed to be
                         # turned on.


def modulus_at(samples, index):
    """Computes the modulus of an IQ sample located at index, index + 1 in
    samples."""
    # abs() of a complex is the modulus of a complex number.
    return abs(complex(samples[index], samples[index + 1]))


class Channel:
    def __init__(self):
        # Represents the factor by which the values of this channel should be
        # multiplied by to be equivalent to those of the reference channel.
        # Should be 1 for the reference channel.
        self.level = 1

        # The number of samples to remove to be time synced to the reference
        # channel.  When this number is positive, the start of this channel came
        # earlier than the start of the reference channel.  When it is negative,
        # fake samples should be added.  After the samples have been dropped or
        # added, the offset should be reset to 0.  Should be 0 for the reference
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


# List of active channels.
channels = [None] * CHANNEL_AMOUNT
# A lock to avoid write conflicts in the list of active channels.
channels_mutex = asyncio.Lock()
# A mutex to protect the reference channel.  Multiple async functions may try to
# find the threshold of the reference channel, so it must be protected against
# concurrent accesses.  This is not the case of other channels.
reference_mutex = asyncio.Lock()


async def try_to_synchronise(channel):
    # Get the reference channel if it exists
    reference = channels[0]

    if reference is None or len(reference) == 0:
        return

    # Get the latest moduli of the current and reference channel.
    c_last_mod = channel.last_modulus()
    r_last_mod = reference.last_modulus()

    # If the carrier started on both channels, do the actual synchronisation.
    if c_last_mod > CARRIER_THRESHOLD and r_last_mod > CARRIER_THRESHOLD:
        with (await reference_mutex):
            reference.find_start()
        channel.find_start()

        # Compute the median of the modulis of the two channels *after* the
        # start of the carrier
        ref_moduli = []
        chan_moduli = []
        i = reference.start_at * 2
        while i < len(reference.buffer):
            ref_moduli.append(modulus_at(reference.buffer, i))
            i += 2

        i = channel.start_at * 2
        while i < len(channel.buffer):
            chan_moduli.append(modulus_at(channel.buffer, i))
            i += 2

        m_ref = median(ref_moduli)
        m_chan = median(chan_moduli)

        channel.level = m_ref / m_chan

        # Compute the difference in offset between the peak of the reference
        # channel and that of the current channel.
        channel.offset = reference.start_at - channel.start_at
        # Mark the channel as synchronised.
        channel.synchronised = True

        print(channel.offset, channel.level, len(ref_moduli), len(chan_moduli))


async def listener(reader, writer):
    ip, port = writer.get_extra_info("peername")
    _, host_port = writer.get_extra_info("sockname")
    channel_id = host_port - BASE_CHANNEL

    print("Connection of {}:{} (channel {})".format(ip, port, channel_id))

    with (await channels_mutex):
        # One connection per channel.
        if channels[channel_id] is not None:
            print("Channel is already in use.".format(channel_id))
            writer.close()
            return

        channel = Channel()
        channels[channel_id] = channel

        # The reference channel is synchronised with itself.
        if channel_id == 0:
            channel.synchronised = True

    try:
        with open("results-{}.csv".format(channel_id), "w") as csv:
            while True:
                # Receive the header and decode it
                header = await reader.read(9)
                if not header:
                    break

                # The header received contains an uint64_t (number of bytes of
                # data) and a boolean in little endian.
                decoded_header = struct.unpack_from("<Q?", header)

                # Read the specified amount of bytes
                data = await reader.read(decoded_header[0])
                if not data:
                    break

                # If the data is saturating, print a warning
                if decoded_header[1]:
                    print("Saturation")

                # Decode the data as an array of int16_t
                decoded = array.array("h", data)

                # Drop samples (or add them if this number is negative).
                i = channel.get_index_to_sync()
                while i < len(decoded):
                    # Compute the modulus of each IQ sample, and write it to a
                    # CSV file.

                    if i >= 0:
                        n = modulus_at(decoded, i) * channel.level
                    else:
                        # If the channel started after the reference channel,
                        # write zeros until it is synced.
                        n = 0
                    csv.write("{},\n".format(int(n)))
                    i += 2

                # Insert the values in the current channel if it has not been
                # synchronised, or if it is the reference channel and its start
                # has not yet been found.
                if not channel.synchronised or \
                   (channel_id == 0 and not channel.start_found):
                    channel.put(decoded)

                if not channel.synchronised:
                    await try_to_synchronise(channel)

    finally:
        print("{}:{} disconnected".format(ip, port))
        writer.close()

        # Remove this current channel from the list to allow another rasseiver
        # to connect.
        with (await channels_mutex):
            channels[channel_id] = None


def main():
    # Open servers
    loop = asyncio.get_event_loop()
    servers = []

    for i in range(CHANNEL_AMOUNT):
        coro = asyncio.start_server(listener, "127.0.0.1", BASE_CHANNEL + i,
                                    loop=loop)
        servers.append(loop.run_until_complete(coro))

    print("Listening…")
    try:
        loop.run_forever()
    except KeyboardInterrupt:
        # Interruption from the user
        print("KeyboardInterrupt")

        # Close servers
        for server in servers:
            server.close()

        # Stop tasks
        for task in asyncio.Task.all_tasks():
            task.cancel()
            with suppress(asyncio.CancelledError):
                loop.run_until_complete(task)
    finally:
        # Close the event loop
        loop.stop()
        loop.run_forever()
        loop.close()


if __name__ == "__main__":
    main()
