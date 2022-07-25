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
