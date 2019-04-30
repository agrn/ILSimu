#!/usr/bin/env python3

from contextlib import suppress

import array
import asyncio
import struct


BASE_CHANNEL = 10000
CHANNEL_AMOUNT = 2


async def listener(reader, writer):
    ip, port = writer.get_extra_info("peername")
    print("Connection of {}:{}".format(ip, port))

    try:
        with open("results.csv", "w") as csv:
            while True:
                header = await reader.read(9)
                if not header:
                    break

                decoded_header = struct.unpack_from("<Q?", header)

                data = await reader.read(decoded_header[0])
                if not data:
                    break

                if decoded_header[1]:
                    print("Saturation")

                decoded = array.array("h", data)
                i = 0
                while i < len(decoded):
                    n = abs(complex(decoded[i], decoded[i + 1]))
                    csv.write("{},\n".format(int(n)))
                    i += 2

    finally:
        print("{}:{} disconnected".format(ip, port))
        writer.close()


def main():
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
        print("KeyboardInterrupt")
        for server in servers:
            server.close()

        for task in asyncio.Task.all_tasks():
            task.cancel()
            with suppress(asyncio.CancelledError):
                loop.run_until_complete(task)
    finally:
        loop.stop()
        loop.run_forever()
        loop.close()


if __name__ == "__main__":
    main()
