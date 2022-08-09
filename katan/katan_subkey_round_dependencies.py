# pypy3 katan_subkey_round_dependencies.py | tee katan_subkey_round_dependencies.txt

import sys
from katan import *

NROUNDS = 50

def trace(c, s, rno, is_dec):
    trace = []
    for i in range(NROUNDS):
        for _ in range(c.STEPS):
            if is_dec:
                s = c.dec_step(s, rno=rno-i)
            else:
                s = c.enc_step(s, rno=rno+i)

            t = []
            t.append(c._last_fa)
            t.append(c._last_fb)
            trace.append(Bin(t))
    return trace


for CIPHER in (KATAN32, KATAN48, KATAN64):
    for dec in range(2):
        sk = []
        for i in range(50):
            sk.append(("ka-%02d" % i, i, 0))
            sk.append(("kb-%02d" % i, i, 1))
        fmt = "%" + str(CIPHER.STEPS+1) + "d"
        if CIPHER is KATAN32:
            fmt = "%3d"

        print(" "*22, end="")
        for i in range(NROUNDS):
            print(fmt % (i+1), end="")
        if dec:
            print(" (decrypted rounds)")
        else:
            print(" (encrypted rounds)")

        deps = []
        for skname, skrno, skindex in sk:
            print(CIPHER.__name__ + ["enc", "dec"][dec], skname, "%2d" % skrno, skindex, end="")
            affects = [0] * NROUNDS * CIPHER.STEPS
            for _ in range(30):
                c = CIPHER(key=randrange(2**80))
                s0 = randrange(2**c.N)
                s1 = randrange(2**c.N)

                # consider different round offsets
                rno = randrange(NROUNDS, 250-NROUNDS)

                trace0 = trace(c, s0, rno, dec)
                trace1 = trace(c, s1, rno, dec)

                if dec:
                    c.subkeys[rno-skrno] = list(c.subkeys[rno-skrno])
                    c.subkeys[rno-skrno][skindex] ^= 1
                else:
                    c.subkeys[rno+skrno] = list(c.subkeys[rno+skrno])
                    c.subkeys[rno+skrno][skindex] ^= 1

                trace0x = trace(c, s0, rno, dec)
                trace1x = trace(c, s1, rno, dec)

                for i in range(len(affects)):
                    affects[i] |= int( (trace0[i] ^ trace1[i]) != (trace0x[i] ^ trace1x[i]))

            deps.append(list(affects))

            print(": ", end="")
            st = "".join(map(str, affects)).replace("0", ".").replace("1", "x")
            for i in range(0, len(st), c.STEPS):
                print(st[i:i+c.STEPS], end=" " if c.N != 32 else "  ")
            print()
            sys.stdout.flush()

        depsum = [0] * (len(affects) // c.STEPS)
        for row in deps:
            row = [int(bool(Bin(row[i:i+c.STEPS]))) for i in range(0, len(row), c.STEPS)]
            for i in range(1, len(row)):
                row[i] |= row[i-1]
            for i in range(len(row)):
                # i//2-th step depends on how many subkeys?
                depsum[i] += row[i]

        print(" "*22, end="")
        for n in depsum:
            print(fmt % n, end="")
        print(" (number of involved subkey bits)")

        print(" "*22, end="")
        for i in range(NROUNDS):
            print(fmt % (i+1), end="")
        if dec:
            print(" (decrypted rounds)")
        else:
            print(" (encrypted rounds)")

        print()
