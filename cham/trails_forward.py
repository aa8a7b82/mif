"""
For 5 rounds we expand differences exhaustively,
for the next 3 rounds we use the ARX counting technique separately
and combine the counts by multiplications.
"""
import sys
import math
from tqdm import tqdm
from binteger import Bin
from collections import Counter
from cham import CHAM64_expand_forward, ARX16, ATTACK52_DeltaOUT

# ATTACK52_DeltaOUT = (0x2000, 0x1000, 0x2810, 0x0020)

diffs = [
    {ATTACK52_DeltaOUT},
]

cnt5 = 0
cnt6 = 0
cnt7 = 0

sum5 = 0
sum6 = 0
sum7 = 0

# manual expansion technique
# (+3 free rounds)
def diff_expand_counts_3R(diff, rno):
    a, b, c, d = diff
    rno += 1
    if rno % 2 == 0:
        bb = Bin(b, 16).rol(1).int
    else:
        bb = Bin(b, 16).rol(8).int

    rno += 1
    if rno % 2 == 0:
        cc = Bin(c, 16).rol(1).int
    else:
        cc = Bin(c, 16).rol(8).int

    rno += 1
    if rno % 2 == 0:
        dd = Bin(d, 16).rol(1).int
    else:
        dd = Bin(d, 16).rol(8).int

    n5 = ARX16.count_dz(dx=a, dy=bb)
    n6 = ARX16.count_dz(dx=b, dy=cc)
    n7 = ARX16.count_dz(dx=c, dy=dd)

    global cnt5, cnt6, cnt7
    global sum5, sum6, sum7
    cnt5 += n5
    cnt6 += n5 * n6
    cnt7 += n5 * n6 * n7

    sum5 += n5
    sum6 += n6
    sum7 += n7


NR = int(sys.argv[1]) if sys.argv[1:] else 5
assert 0 <= NR <= 5, "given "
prev = 0
for rno in range(NR):
    diffs1 = diffs[-1]
    diffs2 = []
    by_wt = [0] * 16
    itr = diffs1 if rno < NR - 1 else tqdm(diffs1)
    num = 0
    for diff1 in itr:
        for diff2, wt in CHAM64_expand_forward(*diff1, rno):
            if rno < NR - 1:
                diffs2.append(diff2)
            else:
                diff_expand_counts_3R(diff2, rno=rno)
            num += 1
            by_wt[wt] += 1
    diffs.append(diffs2)
    prob = sum(2.0**-wt * c for wt, c in enumerate(by_wt)) / sum(by_wt)

    cur = math.log(num, 2)
    print(
        rno, ":", "%6.2f" % cur,
        "step %6.2f" % (cur - prev),
        "prob %6.2f" % math.log(prob, 2),
    )
    prev = cur


rno = NR
for cn in (cnt5, cnt6, cnt7):
    cur = math.log(cn, 2)
    print(rno, ":", "%6.2f" % cur, "step %6.2f" % (cur - prev))
    prev = cur
    rno += 1
