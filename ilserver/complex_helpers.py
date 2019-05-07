def change_phase(cpx, new_phase):
    """Change the phase of a complex number without changing its
    modulus."""
    m = abs(cpx)
    return complex(m * cmath.cos(new_phase), m * cmath.sin(new_phase))


def flat_list_to_complex(lst):
    return [complex(lst[i], lst[i + 1]) for i in range(0, len(lst), 2)]
