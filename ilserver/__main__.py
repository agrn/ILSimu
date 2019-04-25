#!/usr/bin/env python3

import array
import socket
import threading


class ClientThread(threading.Thread):
    def __init__(self, ip, port, clientSocket):
        threading.Thread.__init__(self)
        self.ip = ip
        self.port = port
        self.clientSocket = clientSocket

    def run(self):
        print("Connection of {}:{}".format(self.ip, self.port))

        with open("results-{}:{}.csv".format(self.ip, self.port), "w") as csv:
            while True:
                data = self.clientSocket.recv(2048) # 65536 * 4 / 60 = 4369.066…
                if not data:
                    break

                decoded = array.array('h', data)
                i = 0
                while i < len(decoded):
                    n = abs(complex(decoded[i], decoded[i + 1]))
                    csv.write("{},\n".format(int(n)))
                    i += 2

        print("Client {}:{} disconnected".format(self.ip, self.port))


def main():
    tcpSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcpSock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    tcpSock.bind(("", 10000))

    print("Listening…")

    while True:
        tcpSock.listen(10)
        clientSocket, (ip, port) = tcpSock.accept()

        newThread = ClientThread(ip, port, clientSocket)
        newThread.start()


if __name__ == "__main__":
    main()
