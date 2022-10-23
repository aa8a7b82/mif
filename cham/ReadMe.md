### [pattern_backwards.py](./pattern_backwards.py)

This scripts expands the input difference 4 rounds backwards in the 52-round attack on CHAM64, yield all trail counts (Table 2 (a)) and the number of active bits.
The number of active bits is counted for 3 rounds only, extension for 1 more round is by noting that the word B is moved unmodified:

Here, `DeltaIn = (0x0020, 0x0010, 0x1020, 0x2800)`.

```
Activity pattern after 3 backward rounds from the difference DeltaIn after 4 initial rounds

A: 1111111111111111
B: 1111111111111111
C: 1111110000000000
D: 0000000000100000
```

Run using python or pypy3 (faster):

```sh
$ pypy3 pattern_backwards.py
after 1 backward rounds:
unique differences: 18 = 2^4.1699
trails: 18 = 2^4.1699
active bits: 10 / 64
active bits in +1 round (upper bound): 24 / 64

after 2 backward rounds:
unique differences: 3808 = 2^11.8948
trails: 3808 = 2^11.8948
active bits: 24 / 64
active bits in +1 round (upper bound): 39 / 64

after 3 backward rounds:
unique differences: 14662416 = 2^23.8056
trails: 14662416 = 2^23.8056
active bits: 39 / 64
active bits in +1 round (upper bound): 54 / 64

after 4 backward rounds:
trails: 55000885722 = 2^35.6787

(30 seconds)
```

### [trails_forward.py](./trails_forward.py)

This script counts trails in the 8-round forward extension from `DeltaOUT = (0x2000, 0x1000, 0x2810, 0x0020)`.

This is used in the 52-round attack on CHAM64 (see Table 2 (b)).

The first 5 rounds are enumerated exhaustively, the next 3 rounds are counted independently for each 5-round difference, the counts are then computed by multiplications.

```sh
$ pypy3 trails_forward.py 4  # 4 + 3 rounds, fast
0 :   1.58 step   1.58 prob  -1.58
1 :   8.12 step   6.54 prob  -6.54
2 :  15.46 step   7.34 prob  -7.34
...
3 :  19.55 step   4.09 prob  -4.09
4 :  29.89 step  10.34
5 :  39.95 step  10.06
6 :  52.57 step  12.62

$ pypy3 trails_forward.py 5  # 5 + 3 rounds, slower (~2 hours)
0 :   1.58 step   1.58 prob  -1.58
1 :   8.12 step   6.54 prob  -6.54
2 :  15.46 step   7.34 prob  -7.34
3 :  19.55 step   4.09 prob  -4.09
...
4 :  29.89 step  10.34
5 :  39.95 step  10.06
6 :  52.57 step  12.62
7 :  64.84 step  12.27
```


### [cham-exp.cpp](./cham-exp.cpp)

Cham experiments - Success rate over random keys and key-dependent differential probabilities in Appendix D

To generate results for success rate over random keys:
```
./cham.elf <rounds (12 to 24)>
```

To generate results  for key-dependent differentials:
```
./cham.elf 1 <key selection (1 to 5)>
```
Keys (MSB -> LSB from right to left):
1. 22a9 a4b4 c8fe 4cba d139 c314 e672 9f90
2. 9e96 c292 ca49 4101 97aa a4b9 7cf6 e794
3. 214b 2fd7 16a9 9cd6 d003 dba8 0c87 835e
4. 4661 480e 53e9 5fda 0f46 6169 3044 d509
5. 23a8 d74f 7698 65ef 11a4 7198 6ff1 0c34

---

[gain](./gain) contains code for experimental verification of gain theory.

---

[cham-key-dependent-20r](./cham-key-dependent-20r.stp)

Performs CHAM search for 20r key-dependent trail (Master Key: 4661 480e 53e9 5fda 0f46 6169 3044 d509). Note that for up to 19 rounds, the search completes in less than a minute. No solution was found for 20r even after 1 week.

To execute the search, STP and cryptominisat should be installed. Then run:
```
stp cham-key-dependent-20r.stp --CVC
```

### [latex_attack_table.py](./latex_attack_table.py)

This script generates latex code for Table 4 - Guessing procedure for the 52-round attack on CHAM64.
It also performs checks that guessed words are in fact computable, according to Observation 2.
And it computes complexities according to the analysis method from the paper.

```sh
$ pypy3 latex_attack_table.py 

0 & \multicolumn{2}{c}{initial (after MiF)} & & & $2^{ 96.56 }$ \\
\midrule

1 & R52 : $K[3]$ & & $2^{ -12.27 }$ & $2^{ 100.29 }$ & $2^{ 100.29 }$ \\
2 & R51 : $K[2]$ & & $2^{ -12.62 }$ & $2^{ 103.67 }$ & $2^{ 103.67 }$ \\
3 & & R3 : $K[2]$ & $2^{ -7.72 }$ & $2^{ 103.67 }$ & $2^{ 95.95 }$ \\
4 & R2 : $K[1]$ & & $2^{ -11.91 }$ & $2^{ 100.04 }$ & $2^{ 100.04 }$ \\
5 & & R50 : $K[1]$ & $2^{ -10.06 }$ & $2^{ 100.04 }$ & $2^{ 89.98 }$ \\
6 & R1 : $K[0]$ & & $2^{ -11.87 }$ & $2^{ 94.11 }$ & $2^{ 94.11 }$ \\
7 & & R49 : $K[0]$ & $2^{ -10.34 }$ & $2^{ 94.11 }$ & $2^{ 83.77 }$ \\
8 & & R4 : $K[3]$ & $2^{ -4.17 }$ & $2^{ 83.77 }$ & $2^{ 79.60 }$ \\
9 & R48 : $K[6]$ & & $2^{ -4.09 }$ & $2^{ 91.51 }$ & $2^{ 91.51 }$ \\
10 & & R7 : $K[6]$ & $2^{ -3.00 }$ & $2^{ 91.51 }$ & $2^{ 88.51 }$ \\
11 & R47 : $K[7]$ & & $2^{ -7.34 }$ & $2^{ 97.17 }$ & $2^{ 97.17 }$ \\
12 & R46 : $K[4]$ & & $2^{ -6.54 }$ & $2^{ 106.63 }$ & $2^{ 106.63 }$ \\
13 & & R5 : $K[4]$ & $2^{ -1.00 }$ & $2^{ 106.63 }$ & $2^{ 105.63 }$ \\
14 & & R8 : $K[7]$ & $2^{ -2.00 }$ & $2^{ 105.63 }$ & $2^{ 103.63 }$ \\
15 & R6 : $K[5]$ & & $2^{ -2.00 }$ & $2^{ 117.63 }$ & $2^{ 117.63 }$ \\
16 & & R45 : $K[5]$ & $2^{ -1.58 }$ & $2^{ 117.63 }$ & $2^{ 116.05 }$ \\
```