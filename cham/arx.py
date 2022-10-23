import math
from random import randrange, shuffle
from collections import Counter
from itertools import product
from binteger import Bin
from tqdm import tqdm


class ARX:
    """Class for analysis of modular addition on n-bit words."""

    def __init__(self, n: int):
        self.n = int(n)
        self.mask = (1 << n) - 1
        self.mod = 2**n

        self._precompute_expand()

    def eq(self, a, b, c):
        """Bit-wise function: eq_i = 1  iff a_i = b_i = c_i"""
        return (~a ^ b) & (~a ^ c) & self.mask

    def G(self, a, b, c):
        """Function required for testing the Lipmaa-Moriai condition."""
        return self.eq(a<<1, b<<1, c<<1) & (a^b^c^(c<<1)) & self.mask

    def hw(self, a):
        """Hamming weight of the word `a`"""
        return sum(map(int, f"{a:b}"))

    def diffw_force(self, a, b, c):
        assert not self.G(a, b, c)
        return self.hw((~self.eq(a, b, c)) & (self.mask >> 1))

    def _precompute_expand(self):
        """Precompute the table needed to expand a difference through SUB/ADD."""
        self._tab_expand_sub = {}
        for carrysets, dz, dy in product(range(2**4), range(2), range(2)):
            carrysets2 = [0, 0]
            for c1, c2 in product(range(2), range(2)):
                if (carrysets >> (c1*2 + c2)) & 1:
                    for z1, y1 in product(range(2), range(2)):
                        z2 = (dz ^ z1) & 1
                        y2 = (dy ^ y1) & 1

                        v1 = (z1 & 1) - (y1 & 1) - c1
                        v2 = (z2 & 1) - (y2 & 1) - c2

                        x1 = v1 & 1
                        c1new = (v1 & 2) >> 1

                        x2 = v2 & 1
                        c2new = (v2 & 2) >> 1

                        carrysets2[x1 ^ x2] |= 1 << (c1new * 2 + c2new)
            self._tab_expand_sub[carrysets, dz, dy] = carrysets2

    def expand_sub(self, dz, dy, dx=0, pos=0, carrysets=1, ret=None):
        """Compute possible dx for x = z - y"""
        if ret is None:
            ret = []

        if pos == self.n:
            ret.append(dx)
            return ret

        todo = self._tab_expand_sub[carrysets, dz & 1, dy & 1]
        for dxbit in range(2):
            if todo[dxbit]:
                dx2 = dx | (dxbit << pos)
                self.expand_sub(
                    dz >> 1, dy >> 1, dx2,
                    pos=pos+1,
                    carrysets=todo[dxbit],
                    ret=ret,
                )
        return ret

    def expand_add(self, dx, dy):
        return self.expand_sub(dx, dy)

    def count_dz(self, dx, dy):
        """Count possible differences dz in (dx, dy) -> dz, where z = x [+] y. (or SUB, it's the same)"""
        a, b = dx, dy
        xor = a ^ b

        cnt_eq0 = cnt_eq1 = cnt_neq = 0
        if xor & 1 == 0 and a & 1 == 0:
            # 0 0 0
            cnt_eq0 = 1
        else:
            # 1 1 0 or 0 1 1 or 1 0 1
            cnt_neq = 1

        for _ in range(1, self.n):
            xor >>= 1
            a >>= 1
            b >>= 1

            xor1 = xor & 1
            a1 = a & 1
            b1 = b & 1

            # condensed version:

            if a1 == b1 == 0:
                (cnt_eq0, cnt_eq1, cnt_neq) = \
                    (cnt_eq0 + cnt_neq, 0, cnt_eq1 + cnt_neq)
            elif a1 == b1 == 1:
                (cnt_eq0, cnt_eq1, cnt_neq) = \
                    (0, cnt_eq1 + cnt_neq, cnt_eq0 + cnt_neq)
            else:
                (cnt_eq0, cnt_eq1, cnt_neq) = \
                    (0, 0, cnt_eq0 + cnt_eq1 + 2*cnt_neq)
            continue

            # old unpacked version:

            # new_cnt_eq0 = new_cnt_eq1 = new_cnt_neq = 0
            # prev: 000 (counts[0])
            # new:  000 110
            # new:  011 101
            if xor1 == 0 and a1 == 0:
                new_cnt_eq0 += cnt_eq0
            else:
                new_cnt_neq += cnt_eq0

            # prev: 111 (counts[1])
            # new:  001 111
            # new:  010 100
            if xor1 == 0 and a1 == 1:
                new_cnt_eq1 += cnt_eq1
            else:
                new_cnt_neq += cnt_eq1

            # prev: neq (counts[2])
            if xor1 == 0:
                if a1 == 0:
                    new_cnt_eq0 += cnt_neq
                else:
                    new_cnt_eq1 += cnt_neq
                new_cnt_neq += cnt_neq
            else:
                new_cnt_neq += 2*cnt_neq

            cnt_eq0 = new_cnt_eq0
            cnt_eq1 = new_cnt_eq1
            cnt_neq = new_cnt_neq
        return cnt_eq0 + cnt_eq1 + cnt_neq

    def count_dx(self, dy, dz):
        return self.count_dz(dy, dz)

    def sub_pattern_z(self, dx, y1, y2, dz, c1=0, c2=0, pos=0, suf="", ret=None):
        if ret is None:
            ret = []
        if pos == self.n:
            ret.append(suf)
            return ret

        base_diff = ((dx ^ y1 ^ y2 ^ dz) >> 1) & 1
        cur = [None, None]
        for z1 in range(2):
            z2 = (dz ^ z1) & 1

            v1 = (z1 & 1) - (y1 & 1) - c1
            v2 = (z2 & 1) - (y2 & 1) - c2

            c1new = (v1 & 2) >> 1
            c2new = (v2 & 2) >> 1

            # current carry only can affect the next bit difference
            is_ok = (base_diff ^ c1new ^ c2new) == 0

            if pos == self.n - 1 or is_ok:
                cur[z1] = c1new, c2new


        if cur[0] is cur[1] is None:
            return ret

        star = cur[0] == cur[1] or pos == self.n-1
        star = star or (cur[0] and cur[1] and cur[0][0] == 0)
        if star:
            self.sub_pattern_z(
                dx=dx >> 1,
                y1=y1 >> 1,
                y2=y2 >> 1,
                dz=dz >> 1,
                c1=cur[0][0],
                c2=cur[0][1],
                pos=pos+1,
                suf="*" + suf,
                ret=ret,
            )
        else:
            for val, row in enumerate(cur):
                if row:
                    self.sub_pattern_z(
                        dx=dx >> 1,
                        y1=y1 >> 1,
                        y2=y2 >> 1,
                        dz=dz >> 1,
                        c1=row[0],
                        c2=row[1],
                        pos=pos+1,
                        suf="%d" % val + suf,
                        ret=ret,
                    )
        return ret

    def sub_pattern_y(self, dx, dy, z1, z2):
        pats = self.sub_pattern_z(
            dx=dx,
            y1=(~z1)&self.mask,
            y2=(~z2)&self.mask,
            dz=dy,
        )
        pats2 = []
        for pat in pats:
            pat = pat.replace("0", "+")
            pat = pat.replace("1", "0")
            pat = pat.replace("+", "1")
            pats2.append(pat)
        return pats2

    def add_pattern_y(self, x1, x2, dy, dz):
        return self.sub_pattern_y(
            dx=dz,
            dy=dy,
            z1=(-x1-1) & self.mask,
            z2=(-x2-1) & self.mask,
        )

    def pattern_match(self, pat: str, vec: int) -> bool:
        vec = Bin(vec, self.n).str
        for a, b in zip(vec, pat):
            if a != b and b != "*":
                return False
        return True

    def patterns_mergeable(self, pi, pj):
        for i, (a, b) in enumerate(zip(pi, pj)):
            if a > b:
                a, b = b, a
            if (a, b) == ("0", "1") :
                if pi[i+1:] == pj[i+1:]:
                    assert pi[:i] == pj[:i]
                    return pi[:i] + "*" + pi[i+1:]
                return

            if a != b:
                return
        return

    def patterns_simplify(self, pats):
        pats = list(pats)

        merged = True
        while merged:
            merged = False

            for i in range(len(pats)):
                for j in range(i + 1, len(pats)):
                    pi = pats[i]
                    pj = pats[j]
                    m = self.patterns_mergeable(pi, pj)
                    if m:
                        del pats[j]
                        pats[i] = m
                        merged = True
                        break
                if merged:
                    break
        return sorted(set(pats))


ARX16 = ARX(n=16)
ARX32 = ARX(n=32)


def test_sub_expand():
    A = ARX(n=8)

    n = A.n
    MASK = A.mask

    cnt_pat = Counter()
    total = 1_0_000
    for _ in tqdm(range(total)):
        x1 = randrange(2**n)
        x2 = randrange(2**n)

        y1 = randrange(2**n)
        y2 = randrange(2**n)

        z1 = (x1 + y1) & MASK
        z2 = (x2 + y2) & MASK

        if 1:
            dy = y1 ^ y2
            dz = z1 ^ z2
            dxs = A.expand_sub(dz, dy)

            dxs2 = set()
            for z1 in range(2**n):
                for y1 in range(2**n):
                    z2 = z1 ^ dz
                    y2 = y1 ^ dy

                    x1 = (z1 - y1) & MASK
                    x2 = (z2 - y2) & MASK
                    dxs2.add(x1 ^ x2)

            assert len(dxs2) == A.count_dx(dy, dz)

            print(len(dxs), len(set(dxs)), "=?", len(dxs2))
            print()
            assert set(dxs) == dxs2

        rand = randrange(2**n)
        x1 ^= rand
        x2 ^= rand
        rand = randrange(2**n)
        y1 ^= rand
        y2 ^= rand
        rand = randrange(2**n)
        z1 ^= rand
        z2 ^= rand

        if 0:
            pats = A.sub_pattern_z(
                dx=x1^x2,
                y1=y1, y2=y2,
                dz=z1^z2,
            )
            pats2 = A.patterns_simplify(pats)
            print("z patterns", len(pats), "->", len(pats2))
            n_keys = sum(2**pat.count("*") for pat in pats2)
            print("keys", n_keys)
            n_keys2 = 0
            for k in range(2**n):
                xx1 = ((z1 ^ k) - y1) & MASK
                xx2 = ((z2 ^ k) - y2) & MASK
                if xx1 ^ xx2 == x1 ^ x2:
                    kpat = k ^ z1
                    assert any(A.pattern_match(pat, kpat) for pat in pats)
                    assert any(A.pattern_match(pat, kpat) for pat in pats2)
                    n_keys2 += 1
            assert n_keys == n_keys2
            print()

        if 0:
            pats = A.sub_pattern_y(
                dx=x1^x2,
                dy=y1^y2,
                z1=z1, z2=z2,
            )
            pats2 = A.patterns_simplify(pats)
            print("y patterns", len(pats), "->", len(pats2))
            n_keys = sum(2**pat.count("*") for pat in pats2)
            print("keys", n_keys)
            n_keys2 = 0
            for k in range(2**n):
                xx1 = (z1 - (y1 ^ k)) & MASK
                xx2 = (z2 - (y2 ^ k)) & MASK
                if xx1 ^ xx2 == x1 ^ x2:
                    kpat = k ^ y1
                    assert any(A.pattern_match(pat, kpat) for pat in pats)
                    assert any(A.pattern_match(pat, kpat) for pat in pats2)
                    n_keys2 += 1
            assert n_keys == n_keys2
            print()

        if 1:
            pats = A.add_pattern_y(
                x1=x1, x2=x2,
                dy=y1^y2,
                dz=z1^z2,
            )
            pats2 = A.patterns_simplify(pats)
            # print("dx", Bin(x1 ^ x2, A.n))
            # print("dy", Bin(y1 ^ y2, A.n))
            # print("dz", Bin(z1 ^ z2, A.n))
            eq = A.mask & ~A.eq(x1 ^ x2, y1 ^ y2, z1 ^ z2)
            # print("eq", Bin(eq, A.n))
            # for pat in pats2:
            #     print("pt", pat)
            # print("x+y patterns", len(pats), "->", len(pats2))

            cnt_pat[len(pats2)] += 1

            n_keys = sum(2**pat.count("*") for pat in pats2)
            # print("keys", n_keys)
            if 0:
                n_keys2 = 0
                for k in range(2**n):
                    zz1 = (x1 + (y1 ^ k)) & MASK
                    zz2 = (x2 + (y2 ^ k)) & MASK
                    if zz1 ^ zz2 == z1 ^ z2:
                        kpat = k ^ y1
                        assert any(A.pattern_match(pat, kpat) for pat in pats)
                        assert any(A.pattern_match(pat, kpat) for pat in pats2)
                        n_keys2 += 1
                assert n_keys == n_keys2
            # print()

    print("pattern count stat:")
    for npat, cnt in sorted(cnt_pat.items()):
        print(npat, ":", cnt)
    print("avg", sum(npat * cnt for npat, cnt in cnt_pat.items()) / total)
    print("avg+", sum(npat * cnt for npat, cnt in cnt_pat.items() if npat) / (total - cnt_pat[0]))
    filter = 1 - cnt_pat[0] / total
    print("filter", 1 - cnt_pat[0] / total, f"= 2^{math.log(filter, 2):.5f}")


def test_sub_neg():
    for n in (4, 5, 6, 7, 8, 12, 16, 32, 64):
        x = randrange(2**n)
        y = randrange(2**n)
        sub1 = (x - y) % 2**n

        x = (2**n-1)^x  # NOT
        y = (2**n-1)^y  # NOT
        sub2 = (y - x) % 2**n

        assert sub1 == sub2


if __name__ == '__main__':
    test_sub_neg()
    test_sub_expand()
