### [katan32-expand.cpp](./katan32-expand.cpp)
For KATAN-32: Propagates and counts the number of trails from a given output difference.

Usage example: MiF number of trails (exhaustive) from differences 21000004 and 21000006 before round 108, up to +50 rounds:

**Warning**: requires about 400 GiB of RAM (it uses 2^32 x 2 128-bit integers for precise probability computations, allowing reasonable # of rounds; for the sake of the paper's results - counting trails - the code can be simplified to use much less RAM and not compute the accumulated probabilities).

Timewise, takes about an hour on a single core.

```bash
$ ./kmf32.elf 21000004 108 50 | tee log_21000004_108.txt
...
$ ./kmf32.elf 21000006 108 50 | tee log_21000006_108.txt
...
$ grep '+ 23' log_2100000[46]_108.txt
log_21000004_108.txt:rno 130 (+ 23) (IR=1): avg trail -30.36 avg diff -27.59 (avg trail wt / round  -1.32) (round wt  -1.81) max trails   4.58
log_21000006_108.txt:rno 130 (+ 23) (IR=1): avg trail -32.87 avg diff -29.04 (avg trail wt / round  -1.43) (round wt  -1.80) max trails   5.86

$ grep '+ 42' log_2100000[46]_108.txt
log_21000004_108.txt:rno 149 (+ 42) (IR=1): avg trail -64.46 avg diff -32.00 (avg trail wt / round  -1.53) (round wt  -1.79) max trails  34.22
log_21000006_108.txt:rno 149 (+ 42) (IR=1): avg trail -66.97 avg diff -32.00 (avg trail wt / round  -1.59) (round wt  -1.79) max trails  36.73
````

- 2^30.36 and 2^32.87 trails respectively in the cluster (23 rounds, 108-130).
- 2^62.66 and 2^65.18 trails respectively in the cluster (23+19 rounds, 108-149).


### [katan-expand.cpp](./katan-expand.cpp)
For KATAN-48 and KATAN-64: Propagates and counts the number of trails from a given output difference.

Usage example: Propagate 2 rounds starting from a 77th round output difference for KATAN64:

```bash
./kmf.elf 01000400200040A2 77 2 64
```
Usage example: Propagate 7 rounds starting from an 87th round output difference for KATAN48:

```bash
./kmf.elf 0000400000002092 87 7 48
```


### [katan32-mif.cpp](./katan32-mif.cpp)
Performs the MiF phase of the KATAN-32 attack. Difference and round information has been hardcoded. 

Usage example: Run MiF over 100 pairs with random seed 44:
```bash
./kmf-mif.elf 100 44
```


### [katan_subkey_round_dependencies.py](./katan_subkey_round_dependencies.py)

Generates dependency matrix of feedback functions on subkey bits from involved rounds. Result available in [katan_subkey_round_dependencies.txt](./katan_subkey_round_dependencies.txt)



### [katan_free_pt_rounds.py](./katan_free_pt_rounds.py)

Computes free rounds that can be decrypted under any key and reencrypted under any (other) key keeping the chosen difference.

```bash
$ pypy3 katan_free_pt_rounds.py
free rounds,steps in the KATAN32 attack (pt side): (4, 0)
free rounds,steps in the KATAN48 attack (pt side): (7, 1)
free rounds,steps in the KATAN64 attack (pt side): (5, 1)
````