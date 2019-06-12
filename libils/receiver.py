import array
import socket
import struct

from .constants import PACKET_SIZE, RECEIVER_PORT
from .complex_helpers import flat_list_to_complex
from .helpers import recv_data


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

    def __decode_packet(self, raw_data):
        data = array.array("d", raw_data)
        return flat_list_to_complex(data)

    def __receive_packet(self):
        status = struct.unpack("<I?", self.sock.recv(5))
        raw_data = recv_data(self.sock, status[0] * 8)

        return status[1], raw_data

    def wait_and_receive(self):
        updated = False
        while not updated:
            updated, raw_data = self.__receive_packet()

        return self.__decode_packet(raw_data)

    def receive(self):
        _, raw_data = self.__receive_packet()
        return self.__decode_packet(raw_data)
