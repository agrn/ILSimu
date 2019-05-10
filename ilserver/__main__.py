#!/usr/bin/env python3

from contextlib import suppress

import array
import asyncio
import struct

import cmath

import numpy as np

from channel import Channel, ReferenceChannel
from complex_helpers import compensate_cpx
from constants import BASE_CHANNEL

# Amount of servers to open.  The first server will have the port
# BASE_CHANNEL, the second BASE_CHANNEL + 1, etc.
CHANNEL_AMOUNT = 2

# List of active channels.
channels = [None] * CHANNEL_AMOUNT
# A lock to avoid write conflicts in the list of active channels.
channels_mutex = asyncio.Lock()


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

        if channel_id > 0:
            channel = Channel(channel_id)
        else:
            channel = ReferenceChannel()
            csv = open("results.csv", "w")

        channels[channel_id] = channel

    try:
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

            await channel.process_buffer(decoded, channels[0])

            if channel_id == 0:
                parcours_max = min([len(ch)
                                    if ch is not None and ch.synchronised
                                    else 0
                                    for ch in channels])

                if parcours_max < 1024:
                    parcours_max = 0
                else:
                    parcours_max = 1024

                    i = 0
                    res = np.array(channel.buffer[:1024])
                    del channel.buffer[:1024]

                    for ch in channels[1:]:
                        c = np.array([0j] * 1024)
                        for i in range(1024):
                            c[i] = compensate_cpx(ch.buffer[i],
                                                  ch.level,
                                                  ch.phase_delta)

                        res += c
                        del ch.buffer[:1024]

                    for v in res:
                        r, p = cmath.polar(v)
                        csv.write("{},{}\n".format(r, p))

    finally:
        print("{}:{} disconnected".format(ip, port))
        writer.close()

        # Remove this current channel from the list to allow another rasseiver
        # to connect.
        with (await channels_mutex):
            channels[channel_id] = None

        if channel_id == 0:
            csv.close()


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
