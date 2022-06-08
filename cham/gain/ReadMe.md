### Experimental Verification of Gain
The following generates the experimental gain results in the paper.

Keys (MSB -> LSB from right to left):

Seed 1 = 58ec 944a 5cff dc51 4873 9869 23c6 4567

Seed 5 = fc99 9a30 cbf0 776c 7008 9a6a a7dd dd1b

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
