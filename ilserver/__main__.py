#!/usr/bin/env python3

from contextlib import suppress
from itertools import chain
from concurrent.futures import ThreadPoolExecutor

import array
import asyncio
import struct

import numpy as np

from libils.complex_helpers import compensate_cpx
from libils.constants import PACKET_SIZE, RECEIVER_PORT
from libils.helpers import async_recv_data

from .channel import Channel, ReferenceChannel
from .constants import BASE_CHANNEL

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

    packet_to_send_size = PACKET_SIZE * (len(phase_shift) // CHANNEL_AMOUNT)
    assert len(packet) == packet_to_send_size

    if writer is None:
        return

    try:
        to_send = chain.from_iterable((c.real, c.imag) for c in packet)
        data = struct.pack("<I?{}d".format(2 * packet_to_send_size),
                           len(packet) * 2, old_phase_shift != shift, *to_send)

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
            shift = phase_shift[:]
            to_send = []

            for i in range(len(shift) // CHANNEL_AMOUNT):
                res = np.array([0j] * PACKET_SIZE)
                for j, ch in enumerate(channels):
                    c = compensate_cpx(ch.buffer[:PACKET_SIZE], ch.level,
                                       ch.phase_delta + \
                                       shift[i * CHANNEL_AMOUNT + j])

                    res += c
                to_send = np.concatenate((to_send, res))

            for ch in channels:
                del ch.buffer[:PACKET_SIZE]

            asyncio.run_coroutine_threadsafe(send_back_data(controller_writer,
                                                            to_send, shift),
                                             loop)


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
            data = await async_recv_data(reader, decoded_header[0])

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
            header = await reader.read(4)
            if not header:
                break

            decoded_header = struct.unpack_from("<I", header)
            raw_phase_shift = await async_recv_data(reader,
                                                    8 * CHANNEL_AMOUNT * \
                                                    decoded_header[0])

            phase_shift = array.array("d", raw_phase_shift)

    finally:
        print("Controller disconnected")
        writer.close()
        controller_writer = None
        phase_shift = [0] * CHANNEL_AMOUNT


def main():
    # Open servers
    loop = asyncio.get_event_loop()
    servers = []

    coro = asyncio.start_server(controller_listener, "0.0.0.0",
                                RECEIVER_PORT, loop=loop)
    servers.append(loop.run_until_complete(coro))

    for i in range(CHANNEL_AMOUNT):
        coro = asyncio.start_server(channel_listener, "0.0.0.0",
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
