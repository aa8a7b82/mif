att18: attack18.elf

attack18.elf: mifattack.cpp cham.cpp recursive.cpp common.h
	g++ mifattack.cpp recursive.cpp cham.cpp -DCHAM64_ATTACK=18 -std=c++17 -march=native -O3 -o attack18.elf

clean:
	rm -f attack18.elf
