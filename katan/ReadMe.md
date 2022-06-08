### [katan-expand.cpp](./katan-expand.cpp)
For KATAN-48 and KATAN-64: Propagates and counts the number of trails from a given output difference.

Usage example: Propagate 2 rounds starting from a 77th round output difference for KATAN64:
```
./kmf.elf 01000400200040A2 77 2 64
```
Usage example: Propagate 7 rounds starting from an 87th round output difference for KATAN48:
```
./kmf.elf 0000400000002092 87 7 48
```

### [katan32-mif.cpp](./katan32-mif.cpp)
Performs the MiF phase of the KATAN-32 attack. Difference and round information has been hardcoded. 

Usage example: Run MiF over 100 pairs with random seed 44:
```
./kmf-mif.elf 100 44
```
