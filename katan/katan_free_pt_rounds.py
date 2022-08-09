'''
$ time pypy3 katan_free_pt_rounds.py
free rounds,steps in the KATAN32 attack (pt side): (4, 0)
free rounds,steps in the KATAN48 attack (pt side): (7, 1)
free rounds,steps in the KATAN64 attack (pt side): (5, 1)

________________________________________________________
Executed in    8,45 secs
'''

from katan import *

def determine_free_rounds_top(cipher, rno, diff, itr=10):
    """rno: first round to decrypt (1-based)"""
    best = rno, 1
    for it in range(itr):
        m1 = randrange(2**cipher.N)
        m2 = m1 ^ diff
        p1, p2 = m1, m2
        assert m1 ^ m2 == diff

        cur = None
        CTdec = cipher(key=randrange(2**80))
        # CTdec = cipher(key=0)
        for nr_dec in range(best[0]+1):
            for steps_dec in range(cipher.STEPS):
                good = 1
                for _ in range(16):
                    CTenc = cipher(key=randrange(2**80))

                    t1, t2 = m1, m2
                    for _ in range(steps_dec):
                        t1 = CTenc.enc_step(t1, rno-nr_dec)
                        t2 = CTenc.enc_step(t2, rno-nr_dec)

                    for nr_enc in reversed(range(nr_dec)):
                        for _ in range(cipher.STEPS):
                            t1 = CTenc.enc_step(t1, rno-nr_enc)
                            t2 = CTenc.enc_step(t2, rno-nr_enc)

                    if t1 ^ t2 != diff:
                        good = 0
                        break

                if not good:
                    # print("fail at", nr_dec, steps_dec)
                    break

                cur = nr_dec, steps_dec
                # print("itr", it, "dec", nr_dec, steps_dec, "ok")

                m1 = CTdec.dec_step(m1, rno-nr_dec)
                m2 = CTdec.dec_step(m2, rno-nr_dec)
            if not good:
                break

        assert cur is not None
        best = min(best, cur)
        # print("itr", it, "cur", cur, "best", best)
    return best


if __name__ == '__main__':
    print(
        "free rounds,steps in the KATAN32 attack (pt side):",
        determine_free_rounds_top(
            cipher=KATAN32,
            rno=33,
            diff=0x40028200,
        )
    )

    print(
        "free rounds,steps in the KATAN48 attack (pt side):",
        determine_free_rounds_top(
            cipher=KATAN48,
            rno=34,
            diff=0x000001008000,
        )
    )

    print(
        "free rounds,steps in the KATAN64 attack (pt side):",
        determine_free_rounds_top(
            cipher=KATAN64,
            rno=33,
            diff=0x0080402010000000,
        )
    )
