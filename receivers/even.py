#!/usr/bin/env python3

import numpy as np

import matplotlib.pyplot as plt
import matplotlib.animation as animation

from libils.constants import PACKET_SIZE
from libils.receiver import SynchronousReceiver

fig, ax = plt.subplots()
line, = ax.plot([], [])
l2, = ax.plot([], [], 'rx')
ax.grid()
xdata, ydata = [], []

NUM_DELTA = 8


def get_phase_shift(angle):
    return 1.8 * np.pi * np.sin(np.multiply(angle, np.pi) / 180)


def update(client, iterator):
    i, sangle = iterator
    angles = []
    for angle in sangle:
        angles.append(-angle)
        angles.append(angle)

    phase = get_phase_shift(angles)
    client.send_phase_shifts(phase)
    complex_data = client.wait_and_receive()

    for j, angle in enumerate(sangle):
        if i * NUM_DELTA + j < len(xdata):
            xdata[i * NUM_DELTA + j] = angle
            ydata[i * NUM_DELTA + j] = np.average(np.absolute(complex_data[:PACKET_SIZE]))
        else:
            xdata.append(angle)
            ydata.append(np.average(np.absolute(complex_data[:PACKET_SIZE])))

        xmin, xmax = ax.get_xlim()
        update = False
        if angle >= xmax:
            xmax = xmin + abs(xmax - xmin) * 2
            ax.set_xlim(xmin, xmax)
            update = True
        elif angle < xmin:
            ax.set_xlim(angle, xmax)
            update = True

        ymin, ymax = ax.get_ylim()
        if ydata[-1] >= ymax:
            ax.set_ylim(ymin, ymax * 2)
            update = True
        elif ydata[-1] < ymin:
            ax.set_ylim(ydata[-1], ymax)
            update = True

        if update:
            ax.figure.canvas.draw()
        line.set_data(xdata, ydata)
        l2.set_data([angle], np.average(np.absolute(complex_data[:PACKET_SIZE])))

        del complex_data[:PACKET_SIZE]

    assert len(complex_data) == 0
    return line, l2,


def currified_update(client):
    def internal(iterator):
        return update(client, iterator)
    return internal


import itertools
def custom_enumerate(e):
    for c in itertools.count():
        v = c % len(e)
        yield v, e[v]


try:
    client = SynchronousReceiver("localhost")
    l = np.linspace(-90, 90, 801)
    ll = custom_enumerate([l[i:i + NUM_DELTA] for i in range(0, len(l), NUM_DELTA)])
    ani = animation.FuncAnimation(fig, currified_update(client), ll,
                                  blit=True, interval=10, repeat_delay=3000,
                                  repeat=True)
    plt.show()
finally:
    client.close()
