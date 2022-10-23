from binteger import Bin
from arx import ARX16


def CHAM64_expand_back(a, b, c, d, rno):
    """rno: 0-based"""
    a, b, c, d = d, a, b, c
    if rno % 2 == 0:
        a = Bin(a, 16).ror(8).int
        bb = Bin(b, 16).rol(1).int
    else:
        a = Bin(a, 16).ror(1).int
        bb = Bin(b, 16).rol(8).int

    for a2 in ARX16.expand_sub(a, bb):
        yield (a2, b, c, d)


def CHAM64_expand_forward(a, b, c, d, rno):
    """rno: 0-based"""
    if rno % 2 == 0:
        bb = Bin(b, 16).rol(1).int
    else:
        bb = Bin(b, 16).rol(8).int

    for a2 in ARX16.expand_add(a, bb):
        wt = ARX16.diffw_force(a, bb, a2)
        if rno % 2 == 0:
            a2 = Bin(a2, 16).rol(8).int
        else:
            a2 = Bin(a2, 16).rol(1).int
        yield (b, c, d, a2), wt


def CHAM64_expand_forward_count_only(a, b, c, d, rno):
    """rno: 0-based"""
    if rno % 2 == 0:
        bb = Bin(b, 16).rol(1).int
    else:
        bb = Bin(b, 16).rol(8).int
    return ARX16.count_dx(a, bb)


def CHAM64_expand_back_count_only(a, b, c, d, rno):
    """rno: 0-based"""
    a, b, c, d = d, a, b, c
    if rno % 2 == 0:
        a = Bin(a, 16).ror(8).int
        bb = Bin(b, 16).rol(1).int
    else:
        a = Bin(a, 16).ror(1).int
        bb = Bin(b, 16).rol(8).int

    return ARX16.count_dx(a, bb)
