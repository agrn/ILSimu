from .constants import MAX_RECV


async def async_recv_data(socket, size):
    data = b""
    l = size

    while l > 0:
        data += await socket.read(min(l, MAX_RECV))
        l = size - len(data)

    return data


def recv_data(socket, size):
    data = b""
    l = size

    while l > 0:
        data += socket.recv(min(l, MAX_RECV))
        l = size - len(data)

    return data
