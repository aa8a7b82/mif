/* Mif Attack for 2 filter rounds
Goal: Test for gain over 2 rounds, recover 32-bit key*/

#include <unistd.h>
#include "common.h"
#include "recursive.h"
#include "cham.h"

using namespace std;
unsigned int seed;
int fixed_trail = 0;
extern uint64_t candidates;
struct ClusterTrail {
    Trail trail;

    word_t alpha1, alpha2, alpha3, alpha4;
    word_t beta1, beta2, beta3;
    
    ClusterTrail(const vector<pair<pair<word_t,word_t>,pair<word_t,word_t>>> &diffs) {
        trail = Trail{diffs, {{0,0},{0,0}}, {{0,0},{0,0}}, 0};

        
        auto [dab, dcd] = trail.diffs.back(); //Obtain the two pairs that contain (da,db), (dc,dd)
        
        //Precomputation
        alpha1 = dab.first;
        alpha2 = dab.second;
        alpha3 = dcd.first;
        alpha4 = dcd.second;

        if (NROUNDS%2) { //Odd - start from rot8
            beta1 = ROTL16(dab.second,8);
            beta2 = ROTL16(dcd.first,1);
            //beta3 = ROTL16(dcd.second,8);
        }
        else { //Even - start from rot1
            beta1 = ROTL16(dab.second,1);
            beta2 = ROTL16(dcd.first,8);
            //beta3 = ROTL16(dcd.second,1);
        }
    }
};

vector<Trail> trails;
int attackC = 1;
int offset = 8;

// precomputed cluster here:
// bottom part of the trails
// needs to be extended by 4 rounds to reach CT

//Same input diff for both attacks
const word_t DELTA_Pa = 0x0001; 
const word_t DELTA_Pb = 0x8000;
const word_t DELTA_Pc = 0x0080; 
const word_t DELTA_Pd = 0x0000;

const vector<ClusterTrail> cluster = {
ClusterTrail({
	{{0x0002, 0x0201}, {0x8000, 0x0400}},
	{{0x0201, 0x8000}, {0x0400, 0x0004}},
	{{0x8000, 0x0400}, {0x0004, 0x0502}},
	{{0x0400, 0x0004}, {0x0502, 0x0088}},
	{{0x0004, 0x0502}, {0x0088, 0x0000}}}),
};

static inline void MiF_simple(size_t pair_id, word_t a1, word_t b1, word_t c1, word_t d1, word_t a2, word_t b2, word_t c2, word_t d2) {
    unsigned r1 = 1, r2 = 8; //Rotation numbers, starting from last round backwards
    if(NROUNDS%2) { //Odd
        r1 = 8;
        r2 = 1;
    }
    
    //Output differences
    word_t oa = a1 ^ a2;
    word_t ob = b1 ^ b2;
    word_t oc = c1 ^ c2;
    word_t od = d1 ^ d2;

    //Calculate gamma values
    word_t gamma2 = ROTR16(od,r1);
    word_t gamma1 = ROTR16(oc,r2);

    
    for(auto &prec: cluster) {
		
		if (prec.alpha3 != oa) continue;
		if (prec.alpha4 != ob) continue;
        
        if (g(prec.alpha2, prec.beta2, gamma2)) {
            continue;
        }
        
        if (g(prec.alpha1, prec.beta1, gamma1)) {
            continue;
        }
        Trail trail = prec.trail;
        pair<word_t, word_t> ab = {prec.alpha2,oa};
        pair<word_t, word_t> cd = {ob,oc};
        trail.diffs.push_back({ab,cd});

        ab = {oa,ob};
        cd = {oc,od};
        trail.diffs.push_back({ab,cd});

        ab = {a1, b1};
        cd = {c1, d1};
        trail.ct1 = {ab, cd};

        ab = {a2, b2};
        cd = {c2, d2};
        trail.ct2 = {ab, cd};

        trail.pair_id = pair_id;
        
        trails.push_back(trail);

    }
    
}

int main(int argc, char * argv[])
{
    if (argc != 3 && argc != 4 && argc != 5 && argc != 6) {
        printf("Usage: %s <n_pairs_log> <c> [seed] [n_bits] [trail_num]\n", argv[0]);
        return -1;
    }
    double n_pairs_log;
    sscanf(argv[1], "%lf", &n_pairs_log);
    assert(0 <= n_pairs_log && n_pairs_log <= 50);
    size_t n_pairs = pow(2.0, n_pairs_log);
    size_t n_bits, trail_num;
    attackC = atoi(argv[2]);
    seed = time(NULL);
    srand(seed);
    srand(rand() ^ getpid());
    seed = rand();
	trail_num = 0;
    if (argc == 4) {
        seed = atoi(argv[3]);
        n_bits = 128;
    }
    if (argc == 5) {
        seed = atoi(argv[3]);
        n_bits = atoi(argv[4]);
    }
    if (argc == 6) {
        seed = atoi(argv[3]);
        n_bits = atoi(argv[4]);
        trail_num = atoi(argv[5]);
        fixed_trail=1;
    }

    assert(attackC >= 1);
    assert(1 <= attackC && attackC <= n_pairs);

    printf("n_rounds: %lu\n", NROUNDS);
    printf("seed: %u = 0x%08x\n", seed, seed);
    printf("c: %d\n", attackC);
    printf("n_pairs: %lu = 2^%.2f\n", n_pairs, n_pairs_log);
    printf("n_bits: %ld\n", n_bits);
    printf("trail_num: %ld\n", trail_num);

    srand(seed);

    word_t master_key[8];
    printf("Master key: ");
    for (int i = 0; i < 8; i++) {
        master_key[i] = rand();
        printf("%04x ", master_key[i]);
    }

    // key schedule
    word_t subkeys[16];
    for (int i=0; i<8; i++) {
        subkeys[i] = ROL1(master_key[i])^ROL8(master_key[i])^master_key[i];
        subkeys[(i+offset)^1] = ROL1(master_key[i])^ROL11(master_key[i])^master_key[i];
    }
    printf(" | Round keys: ");
    for (int i = 0; i < 16; i++) {
        printf("%04x ", subkeys[i]);
    }


    printf("\n");
    printf("\n");

    // Stage 0 + Stage 1:
    // encryptions + MiF tool (collect trails
    
    vector<
        pair<  pair<pair<word_t, word_t>,pair<word_t, word_t>>, pair<pair<word_t, word_t>,pair<word_t, word_t>>  >
    > testvecs;

    {
    printf("Stage 0+1 (data encryption + simplified MiF)\n");
    printf("============================================\n");
    auto t0 = chrono::high_resolution_clock::now();
    uint64_t abcd = 0;

    for (size_t i = 0; i < n_pairs; i++) {
        // permute plaintext space to make it random-like
        abcd += 0x36a9f1d9b3a2ee68;
        
        word_t a1 = abcd >> 16;
        word_t b1 = abcd >> 32;
        word_t c1 = abcd >> 48;
        word_t d1 = abcd;

        word_t a2 = a1 ^ DELTA_Pa;
        word_t b2 = b1 ^ DELTA_Pb;
        word_t c2 = c1 ^ DELTA_Pc;
        word_t d2 = d1 ^ DELTA_Pd;

        // ignore repeated pairs
        if (a1 > a2 || (a1 == a2 && b1 > b2) || (a1 == a2 && b1 == b2 && c1 > c2) || (a1 == a2 && b1 == b2 && c1 == c2 && d1 > d2)) {
            i--;
            continue;
        }

        // query 2 encryptions         
        word_t a, b; //temp 
        for(int j = 0; j < NROUNDS; j++) {

            //For a1-d1
            a = a1;
            b = b1;
            if (j%2==0) {
                ER0(a,b,subkeys[j%(2*offset)],j); //ER(a,b,k)
            }
            else {
                ER1(a,b,subkeys[j%(2*offset)],j);
            }
            //Rotation
            a1 = b1; //a=b
            b1 = c1; //b=c
            c1 = d1; //c=d
            d1 = a; //Encrypted a

            //For a2-d2
            a = a2;
            b = b2;
            if (j%2==0) {
                ER0(a,b,subkeys[j%(2*offset)],j); //ER(a,b,k)
            }
            else {
                ER1(a,b,subkeys[j%(2*offset)],j);
            }
            //Rotation
            a2 = b2; //a=b
            b2 = c2; //b=c
            c2 = d2; //c=d
            d2 = a; //Encrypted a            

        }		
		
		//Fix output differences for gain analysis
        //Randomly selected four good/bad trails for two different keys (seed = 1 and seed = 5)
		if (fixed_trail) {	
			word_t diff_out_a;
			word_t diff_out_b;
			word_t diff_out_c;
			word_t diff_out_d;
			
			//Seed 1
			// 0x0088,0x0000,0x001e,0x3a05 (517) (good)
			if (trail_num == 517 && seed == 1) {
				diff_out_a = 0x0088;
				diff_out_b = 0x0000;
				diff_out_c = 0x001e;
				diff_out_d = 0x3a05;
			} 
			// 0x0088,0x0000,0x0006,0x7a05 (941) (bad)
			else if (trail_num == 941 && seed == 1) {
				diff_out_a = 0x0088;
				diff_out_b = 0x0000;
				diff_out_c = 0x0006;
				diff_out_d = 0x7a05;
			} 
			//Seed 5
			// 0x0088,0x0000,0x001a,0xe604 (695) (good)
			else if (trail_num == 695) {
				diff_out_a = 0x0088;
				diff_out_b = 0x0000;
				diff_out_c = 0x001a;
				diff_out_d = 0xe604;
			} 
			// 0x0088,0x0000,0x0036,0x1a1d (5702) good
			else if (trail_num == 5702 && seed == 5) {
				diff_out_a = 0x0088;
				diff_out_b = 0x0000;
				diff_out_c = 0x0036;
				diff_out_d = 0x1a1d;
            }
			
			if ((a1^a2) != diff_out_a) {continue;}
			if ((b1^b2) != diff_out_b) {continue;}
			if ((c1^c2) != diff_out_c) {continue;}
			if ((d1^d2) != diff_out_d) {continue;}
		} 
		
        // run simplified mif
        MiF_simple(i, a1,b1,c1,d1, a2,b2,c2,d2);

        // save some test vectors for decryption testing
        if (i < 8) {
            
            word_t pa1 = abcd >> 16;
            word_t pb1 = abcd >> 32;
            word_t pc1 = abcd >> 48;
            word_t pd1 = abcd;

            pair<word_t, word_t> pab1 = {pa1, pb1};
            pair<word_t, word_t> pcd1 = {pc1, pd1};
            pair<word_t, word_t> ab1 = {a1, b1};
            pair<word_t, word_t> cd1 = {c1, d1};

            testvecs.push_back({
                {pab1, pcd1}, {ab1, cd1}
            });
        }
    }
    
    auto t1 = chrono::high_resolution_clock::now();
    double time_enc_mif = \
        chrono::duration_cast<chrono::microseconds>(t1 - t0).count() / (double)1e6;
    printf("Encryptions + MiF time (enc + T_mif): %.6lfs\n", time_enc_mif);
    printf("Collected %lu trails\n", trails.size());
    printf("\n");
    }
    
    //To debug the number of good/bad trails
    if (1) {
        size_t n_right_trails = 0;
        for (auto & trail: trails) {
            auto [ab1, cd1] = trail.ct1;
            auto [ab2, cd2] = trail.ct2;
            // auto [x1, y1] = trail.ct1;
            // auto [x2, y2] = trail.ct2;

            word_t a1 = ab1.first;
            word_t b1 = ab1.second;
            word_t c1 = cd1.first;
            word_t d1 = cd1.second;
            word_t a2 = ab2.first;
            word_t b2 = ab2.second;
            word_t c2 = cd2.first;
            word_t d2 = cd2.second;
			
			
			word_t tmp_a, tmp_b, tmp_c, tmp_d;
			word_t out_a, out_b, out_c, out_d;


            assert(trail.diffs.back().first.first == (a1 ^ a2));
            assert(trail.diffs.back().first.second == (b1 ^ b2));
            assert(trail.diffs.back().second.first == (c1 ^ c2));
            assert(trail.diffs.back().second.second == (d1 ^ d2));
            
            int good = 1;
            for (int i = 0; i < trail.diffs.size() - 1; i++) {
                auto &diff = trail.diffs[trail.diffs.size()-1-i];
                if (diff.first.first != (a1 ^ a2)) {
                    good = 0;
                }
                if (diff.first.second != (b1 ^ b2)) {
                    good = 0;
                }
                if (diff.second.first != (c1 ^ c2)) {
                    good = 0;
                }
                if (diff.second.second != (d1 ^ d2)) {
                    good = 0;
                }
				
				if (i==0)
				{
					out_a = a1 ^ a2;
					out_b = b1 ^ b2;
					out_c = c1 ^ c2;
					out_d = d1 ^ d2;
				}
				
				if (i==2)
				{
					tmp_a = a1 ^ a2;
					tmp_b = b1 ^ b2;
					tmp_c = c1 ^ c2;
					tmp_d = d1 ^ d2;
				}

                //for a1-d1
                //Unshuffle
                word_t a = d1;
                word_t b = a1;
                int j = NROUNDS-1-i;
                //Cham Decryption
                if (j%2==0) {
                    DR0(a,b,subkeys[j%(2*offset)],j); //ER(a,b,k)
                }
                else {
                    DR1(a,b,subkeys[j%(2*offset)],j);
                }
                //Rotation
                d1 = c1;
                c1 = b1;
                b1 = a1;
                a1 = a;

                //for a2-d2
                //Unshuffle
                a = d2;
                b = a2;
                //Cham encryption
                if (j%2==0) {
                    DR0(a,b,subkeys[j%(2*offset)],j); //ER(a,b,k)
                }
                else {
                    DR1(a,b,subkeys[j%(2*offset)],j);
                }
                //Rotation
                d2 = c2;
                c2 = b2;
                b2 = a2;
                a2 = a;
            }
            if (good) {
                if (0) {
                    for (auto [dab, dcd]: trail.diffs) {
                        printf("%x, %x, %x, %x\n", dab.first, dab.second, dcd.first, dcd.second);
                    }
                    auto[ab,cd] = trail.ct1;
                    printf("ct1 %x, %x, %x, %x\n", ab.first, ab.second, cd.first, cd.second);
                    auto[ab2,cd2] = trail.ct2;
                    printf("ct2 %x, %x, %x, %x\n", ab2.first, ab2.second, cd2.first, cd2.second);
                    
                }                    
                if (1) {
					printf("[DEBUG] caught GOOD trail (pair #%lu)\n", trail.pair_id);
						printf("=====\n");
					n_right_trails++;
				}
            } 
        }
        printf("\n");
        if (n_right_trails > attackC) {
            printf("More right pairs than expected, direct attack may take a bit longer (current version does not detect it)\n");
            printf("\n");
        }
        if (n_right_trails < attackC) {
            printf("Less right pairs than expected, the attack will fail.\n");
        }
    }
    
    // Stage 2:
    // recursive (multi-trail attack)
    {
    printf("Stage 2 (multi-trail recursive procedure)\n");
    printf("=========================================\n");
    auto t0 = chrono::high_resolution_clock::now();
    full_attack_bitwise_recursive(
        trails,
        testvecs,
        attackC,
        n_bits
    ); 
    auto t1 = chrono::high_resolution_clock::now();
    double time_cnt = \
        chrono::duration_cast<chrono::microseconds>(t1 - t0).count() / (double)1e6;
	
    printf("Trail-key candidates = 2**%f (%ld)\n", log2(candidates), candidates);
    if(fixed_trail) {
        printf("Trails = %ld, ", trails.size());
        double ave_keys = candidates/trails.size();
        printf("Gain = %f\n",n_bits-log2(ave_keys));
        printf("\n");
    }
    
    printf("Multi-trail recurse time (T_cnt): %.6lfs\n", time_cnt);
    }
    return 0;    
}
