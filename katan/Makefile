all: kmf.elf kmf32.elf kmf-mif.elf

clean:
	rm -f kmf*.elf

kmf32.elf: katan32-expand.cpp
	g++ katan32-expand.cpp -fconcepts-ts -O3 -o kmf32.elf

kmf.elf: katan-expand.cpp
	g++ katan-expand.cpp -O3 -o kmf.elf

kmf-mif.elf: katan32-mif.cpp
	g++ katan32-mif.cpp -O3 -o kmf-mif.elf