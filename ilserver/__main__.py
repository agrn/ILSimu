#!/usr/bin/env python3

from contextlib import suppress

import array
import asyncio
import struct


BASE_CHANNEL = 10000     # First port to listen on
CHANNEL_AMOUNT = 2       # Amount of servers to open.  The first server will
                         # have the port BASE_CHANNEL, the second BASE_CHANNEL +
                         # 1, etc.
CARRIER_THRESHOLD = 1000


async def listener(reader, writer):
    ip, port = writer.get_extra_info("peername")
    print("Connection of {}:{}".format(ip, port))

    try:
        with open("results.csv", "w") as csv:
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
                i = 0
                while i < len(decoded):
                    # Compute the modulus of each IQ sample, and write it to a
                    # CSV file.
                    n = abs(complex(decoded[i], decoded[i + 1]))
                    csv.write("{},\n".format(int(n)))
                    i += 2

    finally:
        print("{}:{} disconnected".format(ip, port))
        writer.close()


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
