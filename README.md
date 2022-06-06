# Meet-in-the-Filter

### gain-experiments
Files for experimental verification of gain in Table 8

### cham-exp.cpp 
Cham experiments (Table 2 and 3) - Success rate over random keys and key-dependent differential probabilities

Compilation:
```
g++ cham-exp.cpp -o cham.elf
```

To generate results in Table 2 (success rate over random keys):
```
./cham.elf <rounds (12 to 24)>
```

To generate results in Table 3 (key-dependent differentials):
```
./cham.elf 1 <key selection (1 to 5)>
```
Keys (MSB -> LSB from right to left):

  1 - 22a9 a4b4 c8fe 4cba d139 c314 e672 9f90
  
  2 - 9e96 c292 ca49 4101 97aa a4b9 7cf6 e794
  
  3 - 214b 2fd7 16a9 9cd6 d003 dba8 0c87 835e
  
  4 - 4661 480e 53e9 5fda 0f46 6169 3044 d509
  
  5 - 23a8 d74f 7698 65ef 11a4 7198 6ff1 0c34
  
### katan-expand.cpp
For KATAN-48 and KATAN-64: Propagates and counts the number of trails from a given output difference.
Compilation:
```
g++ katan-expand.cpp -o kmf.elf
```
Usage example: Propagate 2 rounds starting from a 77th round output difference for KATAN64
```
./kmf.elf 01000400200040A2 77 2 64
```
Usage example: Propagate 7 rounds starting from an 87th round output difference for KATAN48
```
./kmf.elf 0000400000002092 87 7 48
```

