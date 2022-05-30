#include <cstdio>
#include <vector>
#include <math.h>
#include <cassert>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <cstdint> // include this header for uint64_t

using namespace std;

#define word_t unsigned short
#define ROTL16(x,r) (((x)<<(r)) | (x>>(16-(r))))
#define ROTR16(x,r) (((x)>>(r)) | ((x)<<(16-(r))))
#define ROTL16_1(x) (((x)<<(1)) | (x>>(16-(1))))
#define ROTR16_1(x) (((x)>>(1)) | ((x)<<(16-(1))))
#define ROTL16_8(x) (((x)<<(8)) | (x>>(16-(8))))
#define ROTR16_8(x) (((x)>>(8)) | ((x)<<(16-(8))))
#define ROTL16_11(x) (((x)<<(11)) | (x>>(16-(11))))
#define ROTR16_11(x) (((x)>>(11)) | ((x)<<(16-(11)))) 
#define ROL1 ROTL16_1
#define ROL8 ROTL16_8
#define ROL11 ROTL16_11


#define ER16_0(a,b,k,i) (a^=i, b=ROTL16_1(b), b^=k, a+=b, a=ROTL16_8(a)) 
#define DR16_0(a,b,k,i) (a=ROTR16_8(a), b = ROTL16_1(b), b^=k, a-=b, a^=i)

#define ER16_1(a,b,k,i) (a^=i, b=ROTL16_8(b), b^=k, a+=b, a=ROTL16_1(a))  
#define DR16_1(a,b,k,i) (a=ROTR16_1(a), b = ROTL16_8(b), b^=k, a-=b, a^=i)

#define offset 8

#define ER0(a,b,k,i) ER16_0(a,b,k,i)
#define ER1(a,b,k,i) ER16_1(a,b,k,i)
#define DR0(a,b,k,i) DR16_0(a,b,k,i)
#define DR1(a,b,k,i) DR16_1(a,b,k,i)

word_t ks[16] = {};

void ChamKeySchedule(word_t K[8]) {
    //We generate all 16 RKs based on the master key
    for (int i=0; i<8; i++) {

        ks[i] = ROL1(K[i])^ROL8(K[i])^K[i];
        ks[(i+offset)^1] = ROL1(K[i])^ROL11(K[i])^K[i];
    }
}

vector<word_t> chamKS_Revert(const vector<word_t> &last_subkeys_rev, int nrounds) {
    assert(last_subkeys_rev.size() == 8); // currently supported
    vector<word_t> subkeys = last_subkeys_rev;
    word_t sortedKeys[8];

    //nrounds-1 is the last subkey, ctr = 0 represents last subkey
    int ctr = 0;
    word_t temp;
    for (int i = (nrounds - 1); i >= nrounds - 8; i--) {
        if ((i%16) < 8) { //Invert using 0,1,8
            sortedKeys[i%16] = ROTR16(subkeys[ctr], 2)^ROTR16(subkeys[ctr],10)^ROTR16(subkeys[ctr],1);
            printf("R%d %d %x->%x\n",i, i%16,subkeys[ctr], sortedKeys[i%16]);
        }
        if ((i%16) >= 8) { //Invert using 0,1,11        
            sortedKeys[(i^1)%8] = subkeys[ctr]^ROTR16(subkeys[ctr], 2)^ROTR16(subkeys[ctr], 3)^ROTR16(subkeys[ctr], 4)^ROTR16(subkeys[ctr], 6)^ROTR16(subkeys[ctr], 7)^ROTR16(subkeys[ctr], 9)^ROTR16(subkeys[ctr], 12)^ROTR16(subkeys[ctr], 15);
            printf("R%d %d %x->%x\n",i, (i^1)%8,subkeys[ctr], sortedKeys[(i^1)%16]);
        }
        ctr++;
    }

    for (int i = 0; i<8; i++) {
        subkeys[i] = sortedKeys[i];
    }

    return subkeys;
}

void ChamKeyInvert(word_t K[8]) {
    //Given 8 consective keywords, first 
    for (int i=0; i<8; i++) {
        printf("%x ", K[i]);
        ks[i] = ROL1(K[i])^ROL8(K[i])^K[i];
        ks[(i+offset)^1] = ROL1(K[i])^ROL11(K[i])^K[i];
    }
    printf("\n");
    word_t KR[8] = {};
    
    for (int i=0; i<8; i++) {
        KR[i] = ROTR16(ks[i], 2)^ROTR16(ks[i],10)^ROTR16(ks[i],1);
        printf("%x ", KR[i]);

    }
    printf("\n");
    int ctr = 8;
    for (int i=0; i<8; i++) {
        KR[i] = ks[ctr^1]^ROTR16(ks[ctr^1], 2)^ROTR16(ks[ctr^1], 3)^ROTR16(ks[ctr^1], 4)^ROTR16(ks[ctr^1], 6)^ROTR16(ks[ctr^1], 7)^ROTR16(ks[ctr^1], 9)^ROTR16(ks[ctr^1], 12)^ROTR16(ks[ctr^1], 15);
        printf("%x ", KR[i]);
        ctr++;
    }
}

void ChamEncryptOnly(word_t *pt, word_t *inputDiff, int nrounds, word_t *ctdiff) {
	
	word_t ct[nrounds*4+4]={0};
	word_t ctp[nrounds*4+4]={0};
    ct[0] = pt[0]; 
    ct[1] = pt[1]; 
    ct[2] = pt[2]; 
    ct[3] = pt[3]; 

    ctp[0] = pt[0]^inputDiff[0];
    ctp[1] = pt[1]^inputDiff[1];
    ctp[2] = pt[2]^inputDiff[2];
    ctp[3] = pt[3]^inputDiff[3];

    //generate encryption trace
    word_t tmp;
    for(int i=0; i<nrounds; i++){
        //Encrypt first plaintext

        ct[4*i+4] = ct[4*i+0]; //a
        ct[4*i+5] = ct[4*i+1]; //b
        ct[4*i+6] = ct[4*i+2]; //c
        ct[4*i+7] = ct[4*i+3]; //d

        //Cham encryption
        tmp = ct[4*i+5]; //tmp = b
        if (i%2==0) {
            ER0(ct[4*i+4],ct[4*i+5],ks[i%(2*offset)],i); //ER(a,b,k)
        }
        else {
            ER1(ct[4*i+4],ct[4*i+5],ks[i%(2*offset)],i);
        }
        //Rotation
        ct[4*i+5] = tmp; //b = tmp
        tmp = ct[4*i+4]; //tmp = a
        ct[4*i+4] = ct[4*i+5]; // a = b
        ct[4*i+5] = ct[4*i+6]; //b = c;
        ct[4*i+6] = ct[4*i+7]; //c = d;
        ct[4*i+7] = tmp; //d = tmp;

        //Encrypt second plaintext
        ctp[4*i+4] = ctp[4*i+0]; //a
        ctp[4*i+5] = ctp[4*i+1]; //b
        ctp[4*i+6] = ctp[4*i+2]; //c
        ctp[4*i+7] = ctp[4*i+3]; //d

        //Cham encryption
        tmp = ctp[4*i+5]; //tmp = b
        if (i%2==0) {
            ER0(ctp[4*i+4],ctp[4*i+5],ks[i%(2*offset)],i); //ER(a,b,k)
        }
        else {
            ER1(ctp[4*i+4],ctp[4*i+5],ks[i%(2*offset)],i);
        }
        //Rotation
        ctp[4*i+5] = tmp; //b = tmp
        tmp = ctp[4*i+4]; //tmp = a
        ctp[4*i+4] = ctp[4*i+5]; // a = b
        ctp[4*i+5] = ctp[4*i+6]; //b = c;
        ctp[4*i+6] = ctp[4*i+7]; //c = d;
        ctp[4*i+7] = tmp; //d = tmp;
    }

    ctdiff[0]=ct[4*nrounds]^ctp[4*nrounds]; 
    ctdiff[1]=ct[4*nrounds +1 ]^ctp[4*nrounds +1 ];
    ctdiff[2]=ct[4*nrounds +2 ]^ctp[4*nrounds +2 ];
    ctdiff[3]=ct[4*nrounds +3 ]^ctp[4*nrounds +3 ];

    //Return the ciphertext pair
    pt[0] = ct[4*nrounds];
    pt[1] = ct[4*nrounds +1];
    pt[2] = ct[4*nrounds +2];
    pt[3] = ct[4*nrounds +3];
}

//Calculate success rate of finding right pairs
void successRate(word_t inputdiff[], word_t target_diff[], uint64_t pairs, int ROUNDS) {
    word_t pt[4], ctdiff[4];
    uint64_t good_count = 0, right_pairs = 0, iterations = 0;
	double averageCount = 0;
	double totalCount = 0;
	uint64_t val = 0;
	iterations = 0;
	printf("Data = %ld\n", pairs);
	
	while (1) {
        //Random key experiments
        word_t MASTER[8];
        srand (time(NULL)+iterations);
        right_pairs = 0;
        for (int i=0; i<8; i++){
            MASTER[i]=rand();
        }
        ChamKeySchedule(MASTER);
            
        for (uint64_t i = 0; i < pairs; i++)
        {
            pt[0] = rand();
            pt[1] = rand();
            pt[2] = rand();
            pt[3] = rand();
                
            ChamEncryptOnly(pt, inputdiff, ROUNDS, ctdiff);
                
            if ((ctdiff[0] == target_diff[0]) && (ctdiff[1] == target_diff[1]) && (ctdiff[2] == target_diff[2]) && (ctdiff[3] == target_diff[3])){
                right_pairs++;
                break;
            }
            
        }
            
        if (right_pairs!=0) good_count++;
        iterations++;
        printf("Success rate = %lf%% (%ld/%ld)\n",(100.0*good_count/iterations),good_count,iterations);
        printf("----\n");
    }
}

//Experimental estimate of 20R differential probability
void expDiff_20r(word_t MASTER[])
{
    word_t inputdiff[4] = {0x8004,  0x4082,  0x8200,  0x0100};
    word_t target_diff[4] = {0x0004, 0x0502, 0x0088, 0x0000};
    uint64_t good_count = 0, right_pairs = 0, iterations = 0;
	double averageCount = 0;
	double totalCount = 0;
	uint64_t val = 0;
    word_t pt[4], ctdiff[4];
	iterations = 0;

    while (1) {
		srand (time(NULL)+iterations);
		good_count = 0;
		iterations++;
		pt[0] = rand();
		pt[1] = rand();
		pt[2] = rand();
		pt[3] = rand();
		right_pairs = 0;
		ChamKeySchedule(MASTER);
		
		while(1)
		{	
			good_count++;
			val += 0x36a9f1d9b3a2ee68;
			pt[0] += val;
			pt[1] += val>>16;
			pt[2] += val>>32;
			pt[3] += val>>48;
			
			ChamEncryptOnly(pt, inputdiff, 20, ctdiff);
			
			if ((ctdiff[0] == target_diff[0]) && (ctdiff[1] == target_diff[1]) && (ctdiff[2] == target_diff[2]) && (ctdiff[3] == target_diff[3])){
				break;
			}

            if (good_count >= (1ull<<30) ){
				printf("No good pair found after %ld pairs\n", good_count);
				exit(0);
			}
			
		}
		if (good_count != 0) {
			totalCount+=good_count;
			averageCount = totalCount / iterations;
			printf("Average Count = %lf (%lf/%ld)\n",averageCount,totalCount,iterations);
			printf("Average Weight = %lf\n",log2(averageCount));
			printf("----\n");
		} 
	}
}

int main(int argc, char * argv[]) {

    if (argc != 2 && argc !=3) {
        printf("For success rate experiment: %s <rounds (12-24)>\n", argv[0]);
        printf("For key-dependent probability: %s 1 <key selection>\n", argv[0]);
        printf("Keys for key-dependent differential experiment: \n");
        printf("1 - 9f90 e672 c314 d139 4cba c8fe a4b4 22a9\n");
        printf("2 - e794 7cf6 a4b9 97aa 4101 ca49 c292 9e96\n");
        printf("3 - 835e 0c87 dba8 d003 9cd6 16a9 2fd7 214b\n");
        printf("4 - d509 3044 6169 0f46 5fda 53e9 480e 4661\n");
        printf("5 - 0c34 6ff1 7198 11a4 65ef 7698 d74f 23a8\n");
        return -1;
    }

    int ROUNDS = atoi(argv[1]);
    int exp = 0;
    int key = 0;

    if (argc == 3) {
        exp = 1;
        key = atoi(argv[2]);
    }



    if (!exp) {
        switch(ROUNDS) {
            case 12:
            {
                // // 12R input/output difference (best) w=6.81
                word_t inputdiff[4] = {0x1,  0x8000,  0x80,  0x0};
                word_t target_diff[4] = {0x0002, 0x0201, 0x8000, 0x0400};
                uint64_t pairs = pow(2,6.81); 
                successRate(inputdiff, target_diff, pairs, ROUNDS);
                break;
            }
            case 13:
            {
                //13R input/output difference (best) w = 8
                word_t inputdiff[4] = {0x0080,  0x0040,  0x4000,  0x2000};
                word_t target_diff[4] = {0x0001, 0x8100, 0x0001, 0x0200};
                uint64_t pairs = pow(2,8); 
                successRate(inputdiff, target_diff, pairs, ROUNDS);
                break;
            }
            case 14:
            {
                //14R input/output difference (best) w = 8.81
                word_t inputdiff[4] = {0x8200,  0x0100,  0x0001,  0x8000};
                word_t target_diff[4] = {0x0002, 0x0201, 0x8000, 0x0400};
                uint64_t pairs = pow(2,8.81); 
                successRate(inputdiff, target_diff, pairs, ROUNDS);
                break;
            }
            case 15:
            {
                //15R input/output difference (best) w = 10.59
                word_t inputdiff[4] = {0x8200,  0x0100,  0x0001,  0x8000};
                word_t target_diff[4] = {0x0201, 0x8000, 0x0400, 0x0004};
                uint64_t pairs = pow(2,10.59); 
                successRate(inputdiff, target_diff, pairs, ROUNDS);
                break;
            }
            case 16:
            {
                //16R input/output difference (best) w = 13.13
                word_t inputdiff[4] = {0x0001,  0x8000,  0x0080,  0x0000};
                word_t target_diff[4] = {0x0004, 0x0502, 0x0088, 0x0000};
                uint64_t pairs = pow(2,13.13); 
                successRate(inputdiff, target_diff, pairs, ROUNDS);
                break;
            }
            case 17:
            {
                //17R input/output difference (best) w = 14.36
                word_t inputdiff[4] = {0x0080,  0x0040,  0x4000,  0x2000};
                word_t target_diff[4] = {0x0100, 0x0281, 0x0002, 0x0000};
                uint64_t pairs = pow(2,14.36); 
                successRate(inputdiff, target_diff, pairs, ROUNDS);
                break;
            }
            case 18:
            {
                //18R input/output difference (best) w = 15.13
                word_t inputdiff[4] = {0x8200,  0x0100,  0x0001,  0x8000};
                word_t target_diff[4] = {0x0004, 0x0502, 0x0088, 0x0000};
                uint64_t pairs = pow(2,15.13); 
                successRate(inputdiff, target_diff, pairs, ROUNDS);
                break;
            }
            case 19:
            {
                //19R input/output difference (best) w = 17.88
                word_t inputdiff[4] = {0x8200,  0x0100,  0x0001,  0x8000};
                word_t target_diff[4] = {0x0502, 0x0088, 0x0000, 0x000A};
                uint64_t pairs = pow(2,17.88); 
                successRate(inputdiff, target_diff, pairs, ROUNDS);
                break;
            }
            case 20:
            {
                // 20R input/output difference (best) w = 19.94
                word_t inputdiff[4] = {0x8004,  0x4082,  0x8200,  0x0100};
                word_t target_diff[4] = {0x0004, 0x0502, 0x0088, 0x0000};
                uint64_t pairs = pow(2,19.94); 
                successRate(inputdiff, target_diff, pairs, ROUNDS);
                break;
            }
            case 21:
            {
                // //21R input/output difference (best) w = 20.24
                word_t inputdiff[4] = {0x0020,  0x0010,  0x1000,  0x0800};
                word_t target_diff[4] = {0x4001, 0xA140, 0x0001, 0x0280};
                uint64_t pairs = pow(2,20.24); 
                successRate(inputdiff, target_diff, pairs, ROUNDS);
                break;
            }
            case 22:
            {
                //22R input/output difference (best) w = 23.55
                word_t inputdiff[4] = {0x8208,  0x0004,  0x0480,  0x8240};
                word_t target_diff[4] = {0x0400, 0x0205, 0x8800, 0x0000};
                uint64_t pairs = pow(2,23.55);   
                successRate(inputdiff, target_diff, pairs, ROUNDS);      
                break;
            }
            case 23:
            {
                //23R input/output difference (best) w = 24.8
                word_t inputdiff[4] = {0xC810,  0x2400,  0x0020,  0x0010};
                word_t target_diff[4] = {0x4001, 0xA140, 0x0001, 0x0280};
                uint64_t pairs = pow(2,24.8);     
                successRate(inputdiff, target_diff, pairs, ROUNDS);    
                break;
            }
            case 24:
            {
                //24R input/output difference (best) w = 27.44
                word_t inputdiff[4] = {0x0200,  0x4100,  0x2041,  0x9000};
                word_t target_diff[4] = {0x0000, 0x0005, 0x8502, 0x0004};
                uint64_t pairs = pow(2,27.44);   
                successRate(inputdiff, target_diff, pairs, ROUNDS);      
                break;
            }
            default:
                printf("Invalid number of rounds. Only 12 to 24.\n");
                exit(0);
        }
    }
    else if (exp) {
        switch(key) {
            case 1: 
            { 
                word_t MASTER[8] = {0x9f90,0xe672,0xc314,0xd139,0x4cba,0xc8fe,0xa4b4,0x22a9};
                expDiff_20r(MASTER);
                break;
            }
            case 2: 
            {
                word_t MASTER[8] = {0xe794,0x7cf6,0xa4b9,0x97aa,0x4101,0xca49,0xc292,0x9e96};
                expDiff_20r(MASTER);
                break;
            }
            case 3: 
            { 
                word_t MASTER[8] = {0x835e,0x0c87,0xdba8,0xd003,0x9cd6,0x16a9,0x2fd7,0x214b};  
                expDiff_20r(MASTER); 
                break;
            }   
            case 4: 
            { 
                word_t MASTER[8] = {0xd509,0x3044,0x6169,0x0f46,0x5fda,0x53e9,0x480e,0x4661}; 
                expDiff_20r(MASTER); 
                break;
            }  
            case 5: 
            { 
                word_t MASTER[8] = {0x0c34,0x6ff1,0x7198,0x11a4,0x65ef,0x7698,0xd74f,0x23a8};
                expDiff_20r(MASTER);
                break;
            }
            default:
                printf("Invalid key selection.\n");
                exit(0);
        }
    }
	
    return 0;
}
