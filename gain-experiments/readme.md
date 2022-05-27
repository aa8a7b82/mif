#Section 5.4 Experimental Verification of Gain
The following generates results in Table 8.
Seed 1 = Key 1
Seed 5 = Key 2

Compile with:
```
make att18
```

Usage:
```ruby
Format: ./attack18.elf [data] [c] [seed] [key bits to recover] [trailID]
```
```
./attack18.elf 21.87 1 1 32 517
./attack18.elf 21.87 1 1 32 941
./attack18.elf 24.68 1 5 32 695
./attack18.elf 24.68 1 5 32 5702
```
