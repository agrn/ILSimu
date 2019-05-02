import cmath


def complex_at(samples, index):
    """Returns a complex number using index and index + 1 in sample as
    cartesian coordinates."""
    return complex(samples[index], samples[index + 1])


def polar_at(samples, index):
    """Computes the polar coordinates of an IQ sample located at index,
    index + 1 in samples."""
    return cmath.polar(complex_at(samples, index))


def modulus_at(samples, index):
    """Computes the modulus of an IQ sample located at index, index + 1 in
    samples."""
    modulus, _ = polar_at(samples, index)
    return modulus


def phase_at(samples, index):
    """Computes the phase of an IQ sample located at index, index + 1 in
    samples."""
    _, phase = polar_at(samples, index)
    return phase


def change_phase(cpx, new_phase):
    """Change the phase of a complex number without changing its
    modulus."""
    m = abs(cpx)
    return complex(m * cmath.cos(new_phase), m * cmath.sin(new_phase))
