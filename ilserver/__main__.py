#!/usr/bin/env python3

import array
import asyncio
import struct


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
    coro = asyncio.start_server(listener, "127.0.0.1", 10000, loop=loop)
    server = loop.run_until_complete(coro)

    print("Listening…")
    try:
        loop.run_forever()
    except KeyboardInterrupt:
        pass

    server.close()
    loop.run_until_complete(server.wait_closed())
    loop.close()


if __name__ == "__main__":
    main()
