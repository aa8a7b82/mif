// g++ katan32-expand.cpp -O3 -o kmf32.elf

#include <bits/stdc++.h> 

using namespace std;

typedef int32_t i32;
typedef int64_t i64;
typedef uint32_t u32;
typedef uint32_t word;
typedef __uint128_t u128;

constexpr u128 ONE = 1;


bool IR[] = {
    1,1,1,1,1,1,1,0,0,0,1,1,0,1,0,1,0,1,0,1,1,1,1,0,1,1,0,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,1,1,1,1,0,
    0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,1,1,
    0,1,1,1,1,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,0,1,0,0,1,1,1,0,0,1,1,0,1,1,0,0,0,1,0,1,1,1,0,1,1,0,1,1,1,1,0,0,1,0,1,1,
    0,1,1,0,1,0,1,1,1,0,0,1,0,0,1,0,0,1,1,0,1,0,0,0,1,1,1,0,0,0,1,0,0,1,1,1,1,0,1,0,0,0,0,1,1,1,0,1,0,1,1,0,0,0,0,0,
    1,0,1,1,0,0,1,0,0,0,0,0,0,1,1,0,1,1,1,0,0,0,0,0,0,0,1,0,0,1,0
};

typedef u128 t_PROB;
constexpr int PROB1e = 127;

constexpr i64 ND = 1ll << 32;
// constexpr word AMASK = ((1 << 13) - 1) << 19;
constexpr word BMASK = (1 << 19) - 1;

constexpr word AMASK_AND = (1 << 5) | (1 << 8);
constexpr word BMASK_AND = (1 << 3) | (1 << 8) | (1 << 10) | (1 << 12);


static inline void extend(i64 rno, word diff, auto func) {
    word diff2 = (diff << 1) | (diff >> 31);

    word da = diff >> 19;
    word db = diff & BMASK;
    
    word a = (da >> 7);
    if (IR[rno]) {
        a ^= da >> 3;
    }
    a &= 1;

    word b = (db >> 7);
    b &= 1;

    diff2 ^= (b << 19) ^ a; // swapped!
    
    bool afork = (da & AMASK_AND);
    bool bfork = (db & BMASK_AND);
    // printf("ext diff %08x a %d b %d afork %d bfork %d -> %08x\n",
    //         diff, a, b, afork, bfork, diff2);
    
    int shift = afork + bfork;

    func(diff2, shift);
    if (afork)
        func(diff2 ^ 1, shift);
    if (bfork)
        func(diff2 ^ (1 << 19), shift);
    if (afork && bfork)
        func(diff2 ^ 1 ^ (1 << 19), shift);
}

struct Entry {
    u128 num;
    t_PROB prob_diff;
    i32 wt_trail;
};

void work(i64 rno_start, i64 n_rounds_max, word diff) {
    vector<Entry> cur(ND);
    vector<Entry> nxt(ND);

    cur[diff].num = 1;
    cur[diff].prob_diff = ONE << PROB1e;
    cur[diff].wt_trail = 0;

    printf("start diff: %08x before round #%ld (1-based)\n", diff, rno_start);
    double prev = 0.0;
    for(i64 rno = rno_start-1; rno < rno_start-1+n_rounds_max; rno++) {
        i64 n_rounds = rno - rno_start + 2;
        
        // for(i64 diff = 0; diff < cur.size(); diff++) {
        //     nxt[diff] = {0};
        // }
        memset(&nxt[0], 0, nxt.size() * sizeof nxt[0]);

        u128 count_by_shift[3] = {};        

        for(i64 diff = 0; diff < cur.size(); diff++) {
            auto num = cur[diff].num;
            auto wt_trail = cur[diff].wt_trail;
            auto prob_diff = cur[diff].prob_diff;

            assert(!(wt_trail & 3));
            assert(!(prob_diff & 3));

            if (num) {
                extend(rno, diff, [&](word diff2, int shift) {
                    count_by_shift[shift] += 1;
                    i32 wt_new = wt_trail + shift;
                    if (nxt[diff2].num) {
                        nxt[diff2].wt_trail = min(
                            nxt[diff2].wt_trail,
                            wt_new
                        );
                    }
                    else {
                        nxt[diff2].wt_trail = wt_new;
                    }
                    nxt[diff2].num += num;
                    nxt[diff2].prob_diff += prob_diff >> shift;

                    // assert(nxt[diff2] < (1ull << 61));
                    assert(nxt[diff2].num < (ONE << 125));
                });
            }
        }
        swap(cur, nxt);

        u128 n_trails = 0;
        u128 n_diff = 0;
        u128 n_trails_max = 0;
        i32 wt_trail_min = 9999999;
        t_PROB prob_diff_max = 0;

        
        for(i64 diff = 0; diff < cur.size(); diff++) {
            auto num = cur[diff].num;
            auto wt_trail = cur[diff].wt_trail;
            auto prob_diff = cur[diff].prob_diff;
            n_trails += num;
            if (num) {
                // printf("diff %08lx cnt %lu\n", diff, num);
                n_diff++;
                n_trails_max = max(n_trails_max, num);
                wt_trail_min = min(wt_trail_min, wt_trail);
                prob_diff_max = max(prob_diff_max, prob_diff);
            }
        }
        double avg_trail = log2(1.0 / n_trails);
        double avg_diff = log2(1.0 / n_diff);
        printf("rno %3ld (+%3ld) (IR=%d): avg trail %6.2f avg diff %6.2f (avg trail wt / round %6.2f) (round wt %6.2f) ",
            rno+1, n_rounds, IR[rno], avg_trail, avg_diff, avg_trail / n_rounds, avg_trail - prev);
        printf("max trails %6.2f\n", log2(double(n_trails_max)));
        fflush(stdout);

        printf("    >> best trail %d:", wt_trail_min);
        
        int n_print = 0;
        for(i64 diff = 0; diff < cur.size(); diff++) {
            if (cur[diff].num && cur[diff].wt_trail == wt_trail_min) {
                printf(" %08x", (u32)diff);
                n_print++;
                if (n_print >= 100) {
                    printf(" ...");
                    break;
                }
            }
        }
        printf("\n");
        fflush(stdout);
        
        printf("    >> best  diff %6.2f:", log2((double)prob_diff_max)-PROB1e);
        
        n_print = 0;
        for(i64 diff = 0; diff < cur.size(); diff++) {
            if (cur[diff].prob_diff == prob_diff_max) {
                printf(" %08x", (u32)diff);
                n_print++;
                if (n_print >= 100) {
                    printf(" ...");
                    break;
                }
            }
        }
        printf("\n");
        fflush(stdout);

        prev = avg_trail;
    }
}

int main(int argc, char * argv[]) {
    i64 rno_start;
    i64 n_rounds_max = 80;
    word diff;
    sscanf(argv[1], "%x", &diff);
    sscanf(argv[2], "%ld", &rno_start);
    sscanf(argv[3], "%ld", &n_rounds_max);
    work(rno_start, n_rounds_max, diff);
    return 0;
}