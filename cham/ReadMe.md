### [pattern_backwards.py](./pattern_backwards.py)

This scripts expands the input difference 4 rounds backwards in the 52-round attack on CHAM64, yield all trail counts (Table 2 (a)) and the number of active bits.
The number of active bits is counted for 3 rounds only, extension for 1 more round is by noting that the word B is moved unmodified:

```
Activity pattern after 3 backward rounds from the difference DeltaIn = (0x0020, 0x0010, 0x1020, 0x2800) after 4 initial rounds

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