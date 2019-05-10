import cmath

import numpy as np


def compensate_cpx(cpx, amplification, phase):
    r, p = cmath.polar(cpx)
    r *= amplification
    p = p + phase
    return complex(r * np.cos(p), r * np.sin(p))


def flat_list_to_complex(lst):
    return [complex(lst[i], lst[i + 1]) for i in range(0, len(lst), 2)]
