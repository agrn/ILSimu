import numpy as np


def compensate_cpx(cpx, amplification, phase):
    r, p = np.absolute(cpx), np.angle(cpx)
    r *= amplification
    p = p + phase
    i, q = r * np.cos(p), r * np.sin(p)

    return [complex(a, b) for a, b in zip(i, q)]


def flat_list_to_complex(lst):
    return [complex(lst[i], lst[i + 1]) for i in range(0, len(lst), 2)]
