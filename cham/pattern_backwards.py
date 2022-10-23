"""
For 3 rounds we expand differences exhaustively,
for the last round we use the ARX counting technique.
"""
import math
from tqdm import tqdm
from binteger import Bin
from collections import Counter
from cham import CHAM64_expand_back, CHAM64_expand_back_count_only

DeltaIN = (0x0020, 0x0010, 0x1020, 0x2800)

diffs = [
    Counter([DeltaIN]),
]
NR = 4
n_expanded = 0
for rno in reversed(range(1, NR)):
    n_expanded += 1
    print()
    print("after", n_expanded, "backward rounds:")
    diffs1 = diffs[-1]

    mask = [0, 0, 0, 0]
    num = 0
    total = 0
    diffs2 = Counter()
    for diff1, diff1_cnt in tqdm(diffs1.items()):
        for diff2 in CHAM64_expand_back(*diff1, rno):
            mask[0] |= diff2[0]
            mask[1] |= diff2[1]
            mask[2] |= diff2[2]
            mask[3] |= diff2[3]
            num += 1
            diffs2[diff2] += diff1_cnt
            total += diff1_cnt

    print("unique differences:", num, "=", "2^%.4f" % math.log(num, 2))
    print("trails:", total, "= 2^%.4f" % math.log(total, 2))
    print("A:", Bin(mask[0], 16).str)
    print("B:", Bin(mask[1], 16).str)
    print("C:", Bin(mask[2], 16).str)
    print("D:", Bin(mask[3], 16).str)
    n_active = sum(Bin(m).wt for m in mask)
    n_active_next_ub = sum(Bin(m).wt for m in mask[:3]) + 16
    print("active bits:", n_active, "/", 64)
    print("active bits in +1 round (upper bound):", n_active_next_ub, "/", 64)
    print()

    diffs.append(diffs2)

print()
print("after", NR, "backward rounds:")
rno = 0
total = 0
for diff1, diff1_cnt in diffs[-1].items():
    total += CHAM64_expand_back_count_only(*diff1, rno) * diff1_cnt

print("trails:", total, "= 2^%.4f" % math.log(total, 2))
