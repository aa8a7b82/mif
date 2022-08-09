'''
See https://www.cs.technion.ac.il/~orrd/KATAN/katan.c
'''
import math
from binteger import Bin  # pip install binteger
from tqdm import tqdm
from random import randrange
from collections import Counter


class KATAN:
    IR = [
        None, 1,1,1,1,1,1,1,0,0,0,1,1,0,1,0,1,0,1,0,1,1,1,1,0,1,1,0,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,1,1,1,1,0,
        0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,1,1,
        0,1,1,1,1,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,0,1,0,0,1,1,1,0,0,1,1,0,1,1,0,0,0,1,0,1,1,1,0,1,1,0,1,1,1,1,0,0,1,0,1,1,
        0,1,1,0,1,0,1,1,1,0,0,1,0,0,1,0,0,1,1,0,1,0,0,0,1,1,1,0,0,0,1,0,0,1,1,1,1,0,1,0,0,0,0,1,1,1,0,1,0,1,1,0,0,0,0,0,
        1,0,1,1,0,0,1,0,0,0,0,0,0,1,1,0,1,1,1,0,0,0,0,0,0,0,1,0,0,1,0
    ]

    N = NotImplemented
    NA = NotImplemented
    NB = NotImplemented

    STEPS = NotImplemented

    AI_IR = NotImplemented
    AI_XOR = NotImplemented
    AI_AND = NotImplemented

    BI_XOR = NotImplemented
    BI_AND = NotImplemented

    COUNTER_POLY = sum(2**i for i in [0, 3, 5, 7, 8])
    LFSR_POLY = Bin(sum(2**(80-i) for i in (80, 61, 50, 13)), 80)
    ROUNDS = 254

    MASK = NotImplemented
    BMASK = NotImplemented

    AMASK_AND = NotImplemented
    BMASK_AND = NotImplemented

    @classmethod
    def show_diff(cls, diff):
        L2 = diff & cls.BMASK
        L1 = (diff & cls.MASK) >> cls.NB
        return Bin(L1, cls.NA).str[::-1] + ":" + Bin(L2, cls.NB).str

    def ks_KTANTAN(self):
        self.SUBKEY_INDEXES = [None]
        self.subkeys = [None]

        for i in range(1, self.ROUNDS + 1):
            T = self.counters[i]

            i16 = int(T >> 4)
            T2 = (T >> 2) & 1
            T3 = (T >> 3) & 1

            # print("%3d" % i, Bin(T, 8).str, i16, [int(w >> i16) & 1 for w in self.ws])

            if T2 == T3 == 0:
                ai = 0
            else:
                ai = 1 + (T & 3)

            if T2 == 1 and T3 == 0:
                bi = 4
            else:
                bi = (~T) & 3

            ai = ai * 16 + i16
            bi = bi * 16 + i16
            self.SUBKEY_INDEXES.append((ai, bi))
            self.subkeys.append((
                int((self.key >> ai) & 1),
                int((self.key >> bi) & 1),
            ))

    def ks_KATAN(self):
        self.SUBKEY_MASKS = [None]
        self.subkeys = [None]

        #masks = [Bin.unit(i, 80) for i in range(80)]

        # DRAFT, NOT TESTED! (not used in the paper)

        k = int(self.key)
        for i in range(40):
            self.subkeys.append((
                (k >> 1) & 1,
                k & 1,
            ))
            k >>= 1
            # print(i+1, self.subkeys[-1])

        k = int(self.key)
        for i in range(len(self.subkeys), self.ROUNDS + 1):
            o1 = (self.LFSR_POLY & k).parity
            k = (k >> 1) + (o1 << 79)
            o2 = (self.LFSR_POLY & k).parity
            k = (k >> 1) + (o2 << 79)
            self.subkeys.append((o1, o2))
            # print(i, self.subkeys[-1])

    KEY_SCHEDULE = ks_KATAN

    def __init_subclass__(cls):
        cls.MASK = 2**cls.N-1
        cls.BMASK = 2**cls.NB-1

        cls.AMASK_AND = (1 << cls.AI_AND[0]) | (1 << cls.AI_AND[1])
        cls.BMASK_AND = (
            (1 << cls.BI_AND[0]) | (1 << cls.BI_AND[1])
            | (1 << cls.BI_AND[2]) | (1 << cls.BI_AND[3])
        )

    def __init__(self, key):
        self.key = Bin(key, 80)
        self.ws = self.key.split(n=16)
        self.counters = [0xff]

        for i in range(1, self.ROUNDS + 1):
            T = self.counters[-1]
            T <<= 1
            T ^= Bin(T & self.COUNTER_POLY).parity
            T &= 0xff
            self.counters.append(T)
            assert T >> 7 == self.counters[i] >> 7 == self.IR[i]

        self.KEY_SCHEDULE()
        # print(self.subkeys)

    def enc_step(self, x, rno):
        xa = x >> self.NB

        ka, kb = fa, fb = self.subkeys[rno]

        fa ^= (xa >> self.AI_IR) & self.IR[rno] & 1
        fa ^= (xa >> self.AI_XOR) & 1
        fa ^= ((xa >> self.AI_AND[0]) & 1) & ((xa >> self.AI_AND[1]) & 1)

        fb ^= (x >> self.BI_XOR) & 1
        fb ^= ((x >> self.BI_AND[0]) & 1) & ((x >> self.BI_AND[1]) & 1)
        fb ^= ((x >> self.BI_AND[2]) & 1) & ((x >> self.BI_AND[3]) & 1)

        y = ((x << 1) | (x >> (self.N-1))) & self.MASK
        y ^= fa ^ (fb << self.NB)
        self._last_fa = int(fa & 1)
        self._last_fb = int(fb & 1)
        # print("%03d" % rno, ":", Bin(y, self.N).str, ka, kb)
        return y

    def dec_step(self, y, rno):
        x = ((y >> 1) | (y << (self.N-1))) & self.MASK
        xa = x >> self.NB

        ka, kb = fa, fb = self.subkeys[rno]

        fa ^= (xa >> self.AI_IR) & self.IR[rno] & 1
        fa ^= (xa >> self.AI_XOR) & 1
        fa ^= ((xa >> self.AI_AND[0]) & 1) & ((xa >> self.AI_AND[1]) & 1)
        l = ((xa >> self.AI_AND[0]) & 1)
        r = ((xa >> self.AI_AND[1]) & 1)
        # if rno == 28: print(rno, l, r, "=", l & r)

        fb ^= (x >> self.BI_XOR) & 1
        fb ^= ((x >> self.BI_AND[0]) & 1) & ((x >> self.BI_AND[1]) & 1)
        fb ^= ((x >> self.BI_AND[2]) & 1) & ((x >> self.BI_AND[3]) & 1)
        l = ((x >> self.BI_AND[0]) & 1)
        r = ((x >> self.BI_AND[1]) & 1)
        # if rno == 28: print(rno, l, r, "=", l & r)
        l = ((x >> self.BI_AND[2]) & 1)
        r = ((x >> self.BI_AND[3]) & 1)
        # if rno == 28: print(rno, l, r, "=", l & r)
        # if rno == 28: print()
        self._last_fa = int(fa & 1)
        self._last_fb = int(fb & 1)

        x ^= (fa << (self.N - 1)) ^ (fb << (self.NB - 1))
        # print("%03d" % rno, ":", Bin(x, 32).str, ka, kb)
        return x

    def enc(self, x, rounds=None):
        if rounds is None:
            rounds = self.ROUNDS
        for rno in range(1, rounds+1):
            for _ in range(self.STEPS):
                x = self.enc_step(x, rno)
        return x

    def dec(self, x, rounds=None):
        if rounds is None:
            rounds = self.ROUNDS
        for rno in reversed(range(1, rounds+1)):
            for _ in range(self.STEPS):
                x = self.dec_step(x, rno)
        return x

    @classmethod
    def extend_fw_round_list(cls, diffs, rno):
        s = diffs
        del diffs
        for _ in range(cls.STEPS):
            s2 = []
            for diff in s:
                for diff2 in cls.extend_fw_step(diff, rno):
                    s2.append(diff2)
            s = s2
        return s

    @classmethod
    def extend_bk_round_list(cls, diffs, rno):
        s = diffs
        del diffs
        for _ in range(cls.STEPS):
            s2 = []
            for diff in s:
                for diff2 in cls.extend_bk_step(diff, rno):
                    s2.append(diff2)
            s = s2
        return s

    @classmethod
    def extend_fw_step(cls, diff, rno):
        diff2 = ((diff << 1) | (diff >> (cls.N - 1))) & cls.MASK

        da = diff >> cls.NB
        db = diff & cls.BMASK

        a = (da >> cls.AI_XOR)
        if cls.IR[rno]:
            a ^= da >> cls.AI_IR
        a &= 1

        b = db >> cls.BI_XOR
        b &= 1

        diff2 ^= (b << cls.NB) ^ a # swapped!

        afork = bool(da & cls.AMASK_AND)
        bfork = bool(db & cls.BMASK_AND)

        # shift = afork + bfork  # weight

        ret = [diff2]
        if afork:
            ret.extend([w ^ 1 for w in ret])
        if bfork:
            ret.extend([w ^ (1 << cls.NB) for w in ret])
        return ret

    @classmethod
    def extend_bk_step(cls, diff, rno):
        diff2 = ((diff >> 1) | (diff << (cls.N - 1))) & cls.MASK

        da = diff2 >> cls.NB
        db = diff2 & cls.BMASK

        a = da >> cls.AI_XOR
        if cls.IR[rno]:
            a ^= da >> cls.AI_IR
        a &= 1

        b = db >> cls.BI_XOR
        b &= 1

        diff2 ^= (b << (cls.NB - 1)) ^ (a << (cls.N - 1)) # swapped!

        afork = bool(da & cls.AMASK_AND)
        bfork = bool(db & cls.BMASK_AND)

        # shift = afork + bfork  # weight

        ret = [diff2]
        if afork:
            ret.extend([w ^ (1 << (cls.N - 1)) for w in ret])
        if bfork:
            ret.extend([w ^ (1 << (cls.NB - 1)) for w in ret])
        return ret

    @classmethod
    def extend_bk_rounds_list(cls, diff, rno, todo, func):
        info = []
        for i in range(todo[0]):
            info.extend([rno - i] * cls.STEPS)
        info.extend([rno - todo[0]] * todo[1])
        info = tuple(info)
        return cls._extend_bk_rounds_list(diff, func, info)

    @classmethod
    def _extend_bk_rounds_list(cls, diff, func, info):
        if not info:
            func(diff)
            return

        for diff2 in cls.extend_bk_step(diff, info[0]):
            cls._extend_bk_rounds_list(diff2, func, info[1:])


class KATAN32(KATAN):
    STEPS = 1

    N, NA, NB = 32, 13, 19

    AI_IR = 3
    AI_XOR = 7
    AI_AND = 5, 8

    BI_XOR = 7
    BI_AND = 3, 8, 10, 12


class KATAN48(KATAN):
    STEPS = 2

    N, NA, NB = 48, 19, 29

    AI_IR = 6
    AI_XOR = 12
    AI_AND = 7, 15

    BI_XOR = 19
    BI_AND = 6, 15, 13, 21


class KATAN64(KATAN):
    STEPS = 3

    N, NA, NB = 64, 25, 39

    AI_IR = 9
    AI_XOR = 15
    AI_AND = 11, 20

    BI_XOR = 25
    BI_AND = 9, 14, 21, 33


class KTANTAN32(KATAN32):
    KEY_SCHEDULE = KATAN.ks_KTANTAN

class KTANTAN48(KATAN48):
    KEY_SCHEDULE = KATAN.ks_KTANTAN

class KTANTAN64(KATAN64):
    KEY_SCHEDULE = KATAN.ks_KTANTAN


def test_expansion():
    for CIPHER in (KATAN32, KATAN48, KATAN64):
        print(CIPHER)
        C = CIPHER(key=randrange(2**80))

        for itr in range(1000):
            diff = randrange(1, 2**C.N)
            rno = randrange(1, 200)
            fws = C.extend_fw_step(diff, rno)
            for fw in fws:
                bks = C.extend_bk_step(fw, rno)
                assert diff in bks

            bks = C.extend_bk_step(diff, rno)
            for bk in bks:
                fws = C.extend_fw_step(bk, rno)
                assert diff in fws


def test_vectors():
    '''
    katan32_encrypt(key=11..11, plain=00.00) = 01111110000111111111100101000101
    katan32_encrypt(key=00..00, plain=11.11) = 01000011001011100110000111011010

    katan48_encrypt(key=11..11, plain=00.00) = 010010110111111011111100111110111000011001011001
    katan48_encrypt(key=00..00, plain=11.11) = 101001001011110100011001011011010000101110000101

    katan64_encrypt(key=11..11, plain=00.00) = 0010000111110010111010011001110000001111101010111000001010001010
    katan64_encrypt(key=00..00, plain=11.11) = 1100100101010110000100000000110110111110101101100100101110101000

    ktantan32_encrypt(key=11..11, plain=00.00) = 00100010111010100011100110001000
    ktantan32_encrypt(key=00..00, plain=11.11) = 01000011001011100110000111011010

    ktantan48_encrypt(key=11..11, plain=00.00) = 100100110110110100001111101000110011101000000101
    ktantan48_encrypt(key=00..00, plain=11.11) = 101001001011110100011001011011010000101110000101

    ktantan64_encrypt(key=11..11, plain=00.00) = 1100000000101101111000000101101111111010000110010100101100010110
    ktantan64_encrypt(key=00..00, plain=11.11) = 1100100101010110000100000000110110111110101101100100101110101000
    '''
    # tbd check endianness / even / odd bits
    # v = 0xcafecafefaceface
    # for i in range(80):
    #     k |= ((v >> 31) & 1) << i
    #     v = v * 0xabcdefabcdef % 2**64
    #     v ^= v >> 11
    #     v ^= v >> 17

    assert KATAN32(key=Bin.full(80)).enc(0) \
        == 0b01111110000111111111100101000101
    assert KATAN32(key=Bin(0, n=80)).enc(Bin.full(32)) \
        == 0b01000011001011100110000111011010

    assert KATAN48(key=Bin.full(80)).enc(0) \
        == 0b010010110111111011111100111110111000011001011001
    assert KATAN48(key=Bin(0, n=80)).enc(Bin.full(48)) \
        == 0b101001001011110100011001011011010000101110000101

    assert KATAN64(key=Bin.full(80)).enc(0) \
        == 0b0010000111110010111010011001110000001111101010111000001010001010
    assert KATAN64(key=Bin(0, n=80)).enc(Bin.full(64)) \
        == 0b1100100101010110000100000000110110111110101101100100101110101000

    assert KTANTAN32(key=Bin.full(80)).enc(0) \
        == 0b00100010111010100011100110001000
    assert KTANTAN32(key=Bin(0, n=80)).enc(Bin.full(32)) \
        == 0b01000011001011100110000111011010

    assert KTANTAN48(key=Bin.full(80)).enc(0) \
        == 0b100100110110110100001111101000110011101000000101
    assert KTANTAN48(key=Bin(0, n=80)).enc(Bin.full(48)) \
        == 0b101001001011110100011001011011010000101110000101

    assert KTANTAN64(key=Bin.full(80)).enc(0) \
        == 0b1100000000101101111000000101101111111010000110010100101100010110
    assert KTANTAN64(key=Bin(0, n=80)).enc(Bin.full(64)) \
        == 0b1100100101010110000100000000110110111110101101100100101110101000
