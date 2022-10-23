"""
This script generates latex code for Table 4 - Guessing procedure for the 52-round attack on CHAM64.
It also performs checks that guessed words are in fact computable, according to Observation 2.
"""

from collections import namedtuple

if 1:
    # 52-r attack (Table 3)
    data = """
    0   49  11,87   10,34
    1   50  11,91   10,06
    2   51  7,72    12,62
    3   52  4,17    12,27
    4   46  1,00    6,54
    5   45  2,00    1,58
    6   48  3,00    4,09
    7   47  2,00    7,34
    """.strip().replace(",", ".")
    ID = 0
    CUR = 61.05+35.67-1 + 0.84
    R = 52

Row = namedtuple("Row", ("round", "filt", "ki"))

by_round = {}
for line in data.splitlines():
    top, bot, ftop, fbot = line.split()
    top, bot = map(int, (top, bot))
    ki = top
    top += 1
    ftop, fbot = map(float, (ftop, fbot))
    by_round[bot] = Row(bot, fbot, ki)
    by_round[top] = Row(top, ftop, ki)


KNOWN = {-3, -2, -1, 0, R+1, R+2, R+3, R+4}

def check(r):
    if r < 20:
        assert (r - 3) in KNOWN
        assert (r - 4) in KNOWN
    else:
        assert (r + 1) in KNOWN
        assert (r + 4) in KNOWN


def guess(r):
    global ID, CUR
    if r < 20: r += 1
    check(r)
    ID += 1
    v = by_round[r]
    CUR = CUR + 16 - v.filt
    print(f"{ID} & R{v.round} : $K[{v.ki}]$ & & $2^{{ -{v.filt:.2f} }}$ & $2^{{ {CUR:.2f} }}$ & $2^{{ {CUR:.2f} }}$ \\\\")
    KNOWN.add(r)

def verify(r):
    global ID, CUR
    if r < 20: r += 1
    check(r)
    ID += 1
    v = by_round[r]
    prev = CUR
    CUR = CUR - v.filt
    print(f"{ID} & & R{v.round} : $K[{v.ki}]$ & $2^{{ -{v.filt:.2f} }}$ & $2^{{ {prev:.2f} }}$ & $2^{{ {CUR:.2f} }}$ \\\\")
    KNOWN.add(r)

# print(f"{0} & & & & & $2^{{ {CUR:.2f} }}$ \\\\")

print(r"""
0 & \multicolumn{2}{c}{initial (after MiF)} & & & $2^{ %.2f }$ \\
\midrule
""" % CUR
)

# top half
guess(52)
guess(51)
verify(2)
guess(1)
verify(50)
guess(0)
verify(49)
verify(3)

# bot half
guess(48)
verify(6)

guess(47)
guess(46)
verify(4)
verify(7)

guess(5)
verify(45)
