import array
import socket
import struct

from .constants import MAX_RECV, PACKET_SIZE, RECEIVER_PORT
from .complex_helpers import flat_list_to_complex


class SynchronousReceiver:
    def __init__(self, host, port=RECEIVER_PORT, packet_size=PACKET_SIZE):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((host, port))

        self.channel_count = int.from_bytes(self.sock.recv(1), "little")
        self.packet_size = packet_size

    def close(self):
        self.sock.shutdown(socket.SHUT_RDWR)
        self.sock.close()

    def send_phase_shifts(self, phases):
        self.sock.sendall(
            struct.pack("<I{}d".format(len(phases)),
                        len(phases) // self.channel_count, *phases))

    def receive(self):
        status = (0, False,)

        while not status[1]:
            status = struct.unpack("<I?", self.sock.recv(5))
            l = status[0] * 8
            raw_data = b""

            while l > 0:
                raw_data += self.sock.recv(min(l, MAX_RECV))
                l = (status[0] * 8) - len(raw_data)

        data = array.array("d", raw_data)
        return flat_list_to_complex(data)
