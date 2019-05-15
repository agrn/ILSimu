#!/usr/bin/env python3

from contextlib import suppress
from itertools import chain
from concurrent.futures import ThreadPoolExecutor

import array
import asyncio
import struct

import cmath

import numpy as np

from channel import Channel, ReferenceChannel
from complex_helpers import compensate_cpx
from constants import BASE_CHANNEL, PACKET_SIZE

# Amount of servers to open.  The first server will have the port
# BASE_CHANNEL, the second BASE_CHANNEL + 1, etc.
CHANNEL_AMOUNT = 2

# List of active channels.
channels = [None] * CHANNEL_AMOUNT
# A lock to avoid write conflicts in the list of active channels.
channels_mutex = asyncio.Lock()

executor = ThreadPoolExecutor(max_workers=1)

controller_writer = None
phase_shift = [0] * CHANNEL_AMOUNT
old_phase_shift = [0] * CHANNEL_AMOUNT


async def send_back_data(writer, packet, shift):
    global old_phase_shift
    assert len(packet) == PACKET_SIZE

    if writer is None:
        return

    try:
        to_send = chain.from_iterable((c.real, c.imag) for c in packet)
        data = struct.pack("<?{}d".format(PACKET_SIZE * 2),
                           old_phase_shift != shift,
                           *to_send)
        writer.write(data)
        await writer.drain()
        old_phase_shift = shift
    except Exception as e:
        print(e)
        writer.close()


def process_channel(loop, decoded, channel):
    channel.process_buffer(decoded, channels[0])

    if channel.num == 0:
        parcours_max = min([len(ch)
                            if ch is not None and ch.synchronised
                            else 0
                            for ch in channels])

        nb_it = parcours_max // PACKET_SIZE
        for _ in range(nb_it):
            shift = phase_shift.copy()

            res = np.array([0j] * PACKET_SIZE)
            for i, ch in enumerate(channels):
                c = compensate_cpx(ch.buffer[:PACKET_SIZE], ch.level,
                                   ch.phase_delta + shift[i])

                res += c
                del ch.buffer[:PACKET_SIZE]

            asyncio.run_coroutine_threadsafe(send_back_data(controller_writer,
                                                            res, shift), loop)


async def channel_listener(reader, writer):
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

            loop = asyncio.get_event_loop()
            loop.run_in_executor(executor, process_channel,
                                 loop, decoded, channel)

    finally:
        print("{}:{} disconnected".format(ip, port))
        writer.close()

        # Remove this current channel from the list to allow another rasseiver
        # to connect.
        with (await channels_mutex):
            channels[channel_id] = None


async def controller_listener(reader, writer):
    global controller_writer, phase_shift
    print("Connection of controller")

    writer.write(struct.pack("<B", CHANNEL_AMOUNT))
    await writer.drain()

    controller_writer = writer

    try:
        while True:
            raw_phase_shift = await reader.read(8 * CHANNEL_AMOUNT)
            if not raw_phase_shift:
                break

            phase_shift = array.array("d", raw_phase_shift)

    finally:
        print("Controller disconnected")
        writer.close()
        controller_writer = None


def main():
    # Open servers
    loop = asyncio.get_event_loop()
    servers = []

    coro = asyncio.start_server(controller_listener, "127.0.0.1",
                                BASE_CHANNEL - 1, loop=loop)
    servers.append(loop.run_until_complete(coro))

    for i in range(CHANNEL_AMOUNT):
        coro = asyncio.start_server(channel_listener, "127.0.0.1",
                                    BASE_CHANNEL + i, loop=loop)
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
