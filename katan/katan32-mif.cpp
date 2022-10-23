/*
g++ katan32-mif.cpp -O3 -o kmf-mif.elf

Experiment for KATAN32 attack:
Input difference has been hardcoded.
Select between two output differences: 21000004 or 21000006.
Attack in the paper uses has 41 rounds MiF, 23 forward, 18 backward.
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

#define word_t uint32_t
#define ROTL32(x,r) (((x)<<(r)) | (x>>(32-(r))))
#define ROTR32(x,r) (((x)>>(r)) | ((x)<<(32-(r))))


//Initialize
word_t inputDiff =  0x40028200; //87-round input diff used to generate input pair
word_t outputDiff1 = 0x21000004; //87-round output diff used for cluster search
word_t outputDiff2 = 0x21000006; //87-round output diff used for cluster search
int samples = 1;
int offset = 107; //33 + 74
int rounds = 41; //MiF Rounds
int fRounds = 23; //Forward/Cluster rounds
int bRounds = 18; //rounds-fRounds - Backward/Filter rounds
int output_offset = 148; //offset+rounds

uint64_t fnumtrails = 0;
uint64_t bnumtrails = 0; //Used to store matches
uint64_t btrails = 0; //To stop search for particular plaintext pair
unordered_map <word_t, word_t> cluster_trails;
unordered_map <word_t, word_t> filter_trails;
vector <vector<size_t>> ddt{{0},{0,1}};
static bool IR[254] = {1,1,1,1,1,1,1,0,0,0,1,1,0,1,0,1,0,1,0,1,1,1,1,0,1,1,0,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,1,1,1,1,0,
        0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,1,1,
        0,1,1,1,1,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,0,1,0,0,1,1,1,0,0,1,1,0,1,1,0,0,0,1,0,1,1,1,0,1,1,0,1,1,1,1,0,0,1,0,1,1,
        0,1,1,0,1,0,1,1,1,0,0,1,0,0,1,0,0,1,1,0,1,0,0,0,1,1,1,0,0,0,1,0,0,1,1,1,1,0,1,0,0,0,0,1,1,1,0,1,0,1,1,0,0,0,0,0,
        1,0,1,1,0,0,1,0,0,0,0,0,0,1,1,0,1,1,1,0,0,0,0,0,0,0,1,0,0,1,0};
bool ka[254] = {0};
bool kb[254] = {0};

double roundProb[254]={0};
double totalProb[254]={0};

//Katan key schedule
void keySchedule(word_t seed) {
	//Generate random 80-bit key
	bool key[508] = {0};
	srand(seed);
	for (int i=0; i<80; i++) {
		key[i] = rand()%2;
	}

	//Generate the remaining keystream
    for (int i=80; i<508; i++){
        key[i] = key[i-80]^key[i-61]^key[i-50]^key[i-13];
    }

    //Split the keystream into ka and kb
    int counter=0;
    for (int i=0; i<508; i=i+2){
        ka[counter]=key[i];
        kb[counter]=key[i+1];
        counter++;
    }
}

//Given an input difference, traverse all possible trails and update the trails map in forward direction
void traverse_forward(word_t input, int r, int endr) {
	word_t output;
	size_t a0, a1, a2; //Active ANDs
	word_t y7, y18; //Variables for fb
	word_t x22, x26, x31; //Variables for fa
	word_t fa = 0, fb = 0; //Variables to hold output of nonlinear functions

	//First Sub-round
	//Check if ANDs are active
	//3 and 8
	a0 = ((input>>3)&0x1)|((input>>8)&0x1);
	//10 and 12
	a1 = ((input>>10)&0x1)|((input>>12)&0x1);
	//24 and 27
	a2 = ((input>>24)&0x1)|((input>>27)&0x1);

	//Compute other variables
	y18 = (input >> 18) & 0x1;
	y7 =  (input >> 7) & 0x1;
	x22 = (input >> 22) & IR[r]; //AND with IR[r]
	x26 = (input >> 26) & 0x1;
	x31 = (input >> 31) & 0x1;

	word_t visited[8] = {0};
	int ctr = 0;

	for(auto f0:ddt[a0]) {
		for(auto f1:ddt[a1]) {
			for(auto f2:ddt[a2]) {
				//Construct difference
				fa = x31 ^ x26 ^ f2 ^ x22;
				fb = y18 ^ y7 ^ f1 ^ f0;
				output = ROTL32(input,1); //Rotate left by 1 
				//Mask bits to replace:
				output &= 0xFFF7FFFE;
				//Replace bit 0 with fa
				output |= fa;
				//Replace bit 19 with fb
				output |= fb<<19;

				int flag = 0;
				for (int check=0; check<ctr; check++) {
					if (output == visited[check]) 
					{
						flag = 1;
						continue;
					}
				}
				if (flag == 1) continue;

				if ((r+1)==endr) {
					fnumtrails++;
					if (fnumtrails > (1ull<<40)) {
						printf("Max trails reached...");
						exit(0);	
					}
					auto result = cluster_trails.insert({output, 1});
					if (!result.second) {
						++(result.first->second);
					}
				} else {
					traverse_forward(output,r+1,endr);
				}
				visited[ctr++] = output;
			}
		}
	}
}

//Given an output difference, traverse all possible trails and update the trails map in reverse direction//Given an output difference, traverse all possible trails and update the trails map in reverse direction
int traverse_reverse(word_t output, int r, int endr) {
	word_t input;
	size_t a0, a1, a2; //Active ANDs
	word_t y7, y18; //Variables for fb
	word_t x22, x26, x31; //Variables for fa
	word_t fa = 0, fb = 0; //Variables to hold output of nonlinear functions
	
	//Initialize fa= bit 0 and fb = bit 19
	fa = output & 0x1;
	fb = (output >> 19) & 0x1;

	//Rotate register right by 1
	input = ROTR32(output,1); //Rotate right by 1
	input &= 0x7FFBFFFF;
	
	//Check if ANDs are active
	//3 and 8
	a0 = ((input>>3)&0x1)|((input>>8)&0x1);
	//10 and 12
	a1 = ((input>>10)&0x1)|((input>>12)&0x1);
	//24 and 27
	a2 = ((input>>24)&0x1)|((input>>27)&0x1);
	//No need to sum round prob, just store the current round prob
  roundProb[r]=pow(2,-1.0*((a0|a1)+a2));

	//Compute other variables
	y7 = (input >> 7) & 0x1;
	x22 = (input >> 22) & IR[r]; //AND with IR[r]
	x26 = (input >> 26) & 0x1;
	//fa and fb already calculated

    word_t tmp = input; //Backup input
	word_t visited[8] = {0};
	int ctr = 0;

	for(auto f0:ddt[a0]) {
		for(auto f1:ddt[a1]) {
			for(auto f2:ddt[a2]) {
                input = tmp;
				//Construct difference
				x31 = fa ^ x26 ^ f2 ^ x22;
				y18 = fb ^ y7 ^ f1 ^ f0;
				//Replace bit 31 with x31
				input |= x31<<31;
				//Replace bit 18 with y18
				input |= y18<<18;

				int flag = 0;
				for (int check=0; check<ctr; check++) {
					if (input == visited[check]) 
					{
						flag = 1;
						continue;
					}
				}
				if (flag == 1) continue;

				if ((r-1)==endr) {	
					btrails++;
					//Skip to the next plaintext pair if higher than 32
					if (btrails > (1ull<<33)) {
						// printf("Max trails reached, skipping..\n");
						return 0;
					}
					//No need to store trail, just check the cluster for a match
					//If there is a match, increment bnumtrails
					if (cluster_trails.find(input) == cluster_trails.end()){
						// printf("%.8x not found\n", key.first);
						continue;
					}
					bnumtrails++;
					//Sum up probabilities
					for (int cr=output_offset-bRounds; cr<output_offset; cr++ )
					{
						// printf("%lf\n", roundProb[cr]);
						totalProb[cr] += roundProb[cr];
					}
					// printf("----\n");
				} 
				else {
					int res = traverse_reverse(input,r-1,endr);
					if (res == 0) return 0;
				}
				visited[ctr++] = input;
			}
		}
	}
	return 1;
}

//Katan encryption test vectors
word_t katanEncryptOnly(word_t p1, word_t start, word_t rounds) {
	word_t x1, x2, x3, x4, x5, y1, y2, y3, y4, y5, y6;
	word_t fa, fb;
	for (int r = start; r < start+rounds; r++) { //Big rounds
		//Extract bits involved in fa and fb
		x1 = (p1 >> 31) & 0x1; //19+12
		x2 = (p1 >> 26) & 0x1; //19+7
		x3 = (p1 >> 27) & 0x1; //19+8
		x4 = (p1 >> 24) & 0x1; //19+5
		x5 = (p1 >> 22) & 0x1; //19+3

		y1 = (p1 >> 18) & 0x1; //19+7
		y2 = (p1 >> 7) & 0x1; //19+7
		y3 = (p1 >> 12) & 0x1; //19+7
		y4 = (p1 >> 10) & 0x1; //19+7
		y5 = (p1 >> 8) & 0x1; //19+7
		y6 = (p1 >> 3) & 0x1; //19+7

		fa = x1^x2^(x3&x4)^(x5&IR[r]^ka[r]);
		fb = y1^y2^(y3&y4)^(y5&y6)^kb[r];
			
		//Shift and mask the plaintext register
		p1 = ROTL32(p1,1); //Rotate left by 1 
		p1 &= 0xFFF7FFFE;

		//Replace bit 0 with fa
		p1 |= fa;
		//Replace bit 19 with fb
		p1 |= fb<<19;
	}
	return p1;
}

int main(int argc, char * argv[])  {
	if ((argc != 1) && (argc != 2) && (argc != 3)) {
        printf("Usage: %s <samples> <seed>\n", argv[0]);
        return -1;
    }

	int seed = time(0);
	if (argc == 2) {
		samples = atoi(argv[1]);
	}
	if (argc == 3) {
		samples = atoi(argv[1]);
		seed = atoi(argv[2]);
	}
    printf("Seed = %d\n", seed);
    printf("Number of Samples = %d\n", samples);
		
	
	//First build the cluster with both input differences:
	printf("Building cluster from two differences.\n");
	traverse_forward(outputDiff1, offset, offset+fRounds);
	printf("Cluster Size after first difference = 2**%0.4lf (%lu)\n", log2(fnumtrails), fnumtrails);
	traverse_forward(outputDiff2, offset, offset+fRounds);
	printf("Cluster Size after second difference = 2**%0.4lf (%lu)\n", log2(fnumtrails), fnumtrails);

	//Perform reverse search to find a match in the cluster. No need to store filter trails, only store ave prob.
	//Generate random 32-bit output difference by encrypting a pair of plaintexts
	int prev=0;
	if (1) {
		srand(seed);
		keySchedule(seed);
		word_t tmp = rand();
		word_t p1 = rand();
		word_t p2 = p1 ^ inputDiff;
		word_t c1, c2;
		printf("Round,-AveProbLog,Samples\n");
		for (word_t i=0; i<samples; i++)
		{
			btrails = 0;
			c1 = katanEncryptOnly(p1, 33, 74+rounds);
			c2 = katanEncryptOnly(p2, 33, 74+rounds);
			word_t c_diff = c1^c2;	  
			traverse_reverse(c_diff,output_offset-1,output_offset-bRounds-1);
			p1+=tmp;
			p2 = p1 ^ inputDiff;   
			if (bnumtrails>prev) {
				for (int r=output_offset-bRounds; r<output_offset; r++ )
				{
					//Round,AveProbLog,Samples
					printf("%d,%lf,%ld\n",r, -1*log2(totalProb[r]/bnumtrails), bnumtrails);
				}
				prev = bnumtrails;
				printf("\n");
			}
		}
	}

	return 0;
}
