/*
Implementation of KATAN48 MiF to count number of trails
Start from an input difference, propagate all the way until an output difference. 
Increment a counter for the output difference
*/

#include <unordered_map>
#include <cstdio>
#include <vector>
#include <math.h>
#include <cassert>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <cstdint> // include this header for uint64_t
#include <random>

using namespace std;

#define word_t uint64_t
#define size_t uint32_t
#define ROTL32(x,r) (((x)<<(r)) | (x>>(32-(r))))
#define ROTR32(x,r) (((x)>>(r)) | ((x)<<(32-(r))))

#define ROTL48(x,r) (((x)<<(r)) | (x>>(48-(r)))) & 0xFFFFFFFFFFFF
#define ROTR48(x,r) (((x)>>(r)) | ((x)<<(48-(r)))) & 0xFFFFFFFFFFFF

#define ROTL64(x,r) (((x)<<(r)) | (x>>(64-(r)))) 
#define ROTR64(x,r) (((x)>>(r)) | ((x)<<(64-(r)))) 


word_t numtrails = 0;

unordered_map <word_t, word_t> cluster_trails;
unordered_map <word_t, word_t> filter_trails;
vector <vector<size_t>> ddt{{0},{0,1}};
static bool IR[254] = {1,1,1,1,1,1,1,0,0,0,1,1,0,1,0,1,0,1,0,1,1,1,1,0,1,1,0,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,1,1,1,1,0,
        0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,1,1,
        0,1,1,1,1,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,0,1,0,0,1,1,1,0,0,1,1,0,1,1,0,0,0,1,0,1,1,1,0,1,1,0,1,1,1,1,0,0,1,0,1,1,
        0,1,1,0,1,0,1,1,1,0,0,1,0,0,1,0,0,1,1,0,1,0,0,0,1,1,1,0,0,0,1,0,0,1,1,1,1,0,1,0,0,0,0,1,1,1,0,1,0,1,1,0,0,0,0,0,
        1,0,1,1,0,0,1,0,0,0,0,0,0,1,1,0,1,1,1,0,0,0,0,0,0,0,1,0,0,1,0};

//Given an input difference, traverse all possible trails and update the trails map in forward direction
void traverse_forward(word_t input, int r, int sr, int endr) {
	//sr = 0: First sub-round (increment sr and recurse)
	//sr = 1: Second sub-round (increment r and recurse)
	word_t output;
	size_t a0, a1, a2; //Active ANDs
	word_t y19, y28; //Variables for fb
	word_t x35, x41, x47; //Variables for fa
	word_t fa = 0, fb = 0; //Variables to hold output of nonlinear functions

	//First Sub-round
	//Check if ANDs are active
	//6 and 15
	a0 = ((input>>6)&0x1)|((input>>15)&0x1);
	//13 and 21
	a1 = ((input>>13)&0x1)|((input>>21)&0x1);
	//36 and 44
	a2 = ((input>>36)&0x1)|((input>>44)&0x1);

	//Compute other variables
	y19 = (input >> 19) & 0x1;
	y28 = (input >> 28) & 0x1;
	x35 = (input >> 35) & IR[r]; //AND with IR[r]
	x41 = (input >> 41) & 0x1;
	x47 = (input >> 47) & 0x1;

	word_t visited[8] = {0};
	int ctr = 0;

	for(auto f0:ddt[a0]) {
		for(auto f1:ddt[a1]) {
			for(auto f2:ddt[a2]) {
				//Construct difference
				fa = x47 ^ x41 ^ f2 ^ x35;
				fb = y28 ^ y19 ^ f1 ^ f0;
				//111111111111111111011111111111111111111111111110 (mask)
				output = ROTL48(input,1); //Rotate left by 1 
				//Mask bits to replace:
				output &= 0xFFFFDFFFFFFE;
				//Replace bit 0 with fa
				output |= fa;
				//Replace bit 29 with fb
				output |= fb<<29;


				int flag = 0;
				for (int check=0; check<ctr; check++) {
					if (output == visited[check]) 
					{
						flag = 1;
						continue;
					}
				}
				if (flag == 1) continue;

				if (!sr) {
					traverse_forward(output,r,sr+1,endr);
				}
				else {
					if ((r+1)==endr) {
						numtrails++;
						auto result = cluster_trails.insert({output, 1});
						if (!result.second) {
							++(result.first->second);
						}
					} else {
						traverse_forward(output,r+1,0,endr);
					}
				}

				
				visited[ctr++] = output;
			}
		}
	}
}

//Given an input difference, traverse all possible trails and update the trails map in forward direction
void traverse_forward_64(word_t input, int r, int sr, int endr) {
	//sr = 0: First sub-round (increment sr and recurse)
	//sr = 1: Second sub-round (increment r and recurse)
	word_t output;
	size_t a0, a1, a2; //Active ANDs
	word_t y25, y38; //Variables for fb
	word_t x48, x54, x63; //Variables for fa
	word_t fa = 0, fb = 0; //Variables to hold output of nonlinear functions

	//First Sub-round
	//Check if ANDs are active
	//9 and 14
	a0 = ((input>>9)&0x1)|((input>>14)&0x1);
	//21 and 33
	a1 = ((input>>21)&0x1)|((input>>33)&0x1);
	//50 and 59
	a2 = ((input>>50)&0x1)|((input>>59)&0x1);

	//Compute other variables
	y38 = (input >> 38) & 0x1;
	y25 = (input >> 25) & 0x1;
	x48 = (input >> 48) & IR[r]; //AND with IR[r]
	x54 = (input >> 54) & 0x1;
	x63 = (input >> 63) & 0x1;

	word_t visited[8] = {0};
	int ctr = 0;
	for(auto f0:ddt[a0]) {
		for(auto f1:ddt[a1]) {
			for(auto f2:ddt[a2]) {
				//Construct difference
				//printf("f0 %d, f1 %d, f2 %d\n", f0, f1, f2);
				fa = x63 ^ x54 ^ f2 ^ x48;
				fb = y38 ^ y25 ^ f1 ^ f0;
				//1111111111111111111111110111111111111111111111111111111111111110 (mask)
				output = ROTL64(input,1); //Rotate left by 1 
				//Mask bits to replace:
				output &= 0xFFFFFF7FFFFFFFFE;
				//Replace bit 0 with fa
				output |= fa;
				//Replace bit 39 with fb
				output |= fb<<39;
				int flag = 0;
				for (int check=0; check<ctr; check++) {
					if (output == visited[check]) 
					{
						flag = 1;
						continue;
					}
				}
				if (flag == 1) continue;
				if (sr!=2) {
					//printf("%.16lx - round %d subround %d\n", output, r, sr);
					traverse_forward_64(output,r,sr+1,endr);
				}
				else {
					if ((r+1)==endr) {
						//printf("%.16lx - round %d subround %d (FINAL)\n", output, r, sr);
						numtrails++;
						auto result = cluster_trails.insert({output, 1});
						if (!result.second) {
							++(result.first->second);
						}
					} else {
						//printf("%.16lx - round %d subround %d\n", output, r, sr);
						traverse_forward_64(output,r+1,0,endr);
					}
				}
				
				visited[ctr++] = output;
			}
		}
	}
}

int main(int argc, char * argv[]) 
{
	if (argc != 5) {
        printf("Usage: %s <input> <offset> <numrounds> <katan version>\n", argv[0]);
        return -1;
    }
	//Initialize vector
	word_t input = 0x400000002092; //87-round output
	//010000000000000000000000000000000010000010010010
    sscanf(argv[1], "%lx", &input);
	int offset = atoi(argv[2]);
	int rounds = atoi(argv[3]);
	int ver = atoi(argv[4]);
	if (ver == 48) traverse_forward(input, offset, 0,offset+rounds);
	if (ver == 64) traverse_forward_64(input, offset, 0,offset+rounds);
	if (0)
	for (auto i : cluster_trails) {
		if (ver == 48) printf("%.12lx: %ld\n", i.first, i.second);
		if (ver == 64) printf("%.16lx: %ld\n", i.first, i.second);
	}
	printf("Total number of trails in forward search = 2**%0.4lf (%lu)\n", log2(numtrails), numtrails);
    return 0;
}
