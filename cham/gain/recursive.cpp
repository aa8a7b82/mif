/*
Memory management:
We allocate a vector of TrailState at the beginning of each round recursion
(pos = 0, 16, 32).
Then, we only keep vectors of pointers to that states.
These vectors are being "sieved"/filtered when descending in the recursion.
TrailStates themselves are not modified, except at the beginning of new round,
where their new copies are decrypted by 1 round.
*/
#include "common.h"
#include "cham.h"
#include "recursive.h"

using namespace std;

// Declarations

struct TrailState;
static void attack_bitwise_recursive(
    vector<TrailState*> &states,  // is ok to modify as it'll be cleared
    int round_no,
    int pos // 0=LSB .. 15=MSB, but could be incremented by 2,3,4,...
);

uint64_t candidates = 0;
uint64_t surv_pairs = 0;
extern unsigned int seed;
extern int fixed_trail;
// Globals

static size_t g_min_trails = 1;
static const vector<Trail> *g_trails;
static const vector<
    pair<  pair<pair<word_t, word_t>,pair<word_t, word_t>>, pair<pair<word_t, word_t>,pair<word_t, word_t>>  >
> *g_testvecs;
static vector<word_t> g_cur_keys;
static int g_n_bits;
static bool g_completed;

static const int INITIAL_JUMP = 1; //Not used
static word_t g_saved_B;

static vector<TrailState> g_states_values[8];

// TrailState

struct TrailState {
    const Trail &trail;

    word_t a1, a2, b1, b2, c1, c2, d1, d2; 
    word_t dz;

    TrailState(const Trail &t) : trail(t)
    {}

    TrailState clone() const {
        return TrailState(trail);
    }

    void init_ct() {
        //Initialization is only for the final round so check rotations once
        unsigned r1 = 1, r2 = 8;
        if (NROUNDS%2) { 
            r1 = 8;
            r2 = 1;
        }
        const size_t dsz = trail.diffs.size();
        //Set ciphertext values after unshuffling
        //Pre-rotate a and b
        a1 = ROTR16(trail.ct1.second.second, r1); //d1 
        a2 = ROTR16(trail.ct2.second.second, r1); //d2
        b1 = ROTL16(trail.ct1.first.first, r2); //a1
        b2 = ROTL16(trail.ct2.first.first, r2); //a2
        c1 = trail.ct1.first.second; //b1
        c2 = trail.ct2.first.second; //b2
        d1 = trail.ct1.second.first; //c1
        d2 = trail.ct2.second.first; //c2

        dz = trail.diffs[dsz-2].first.first; //da
    }

    void advance_to(TrailState &t, word_t key, int round_no, unsigned r1, unsigned r2) const {

        const size_t dsz = trail.diffs.size();

        word_t tmp;

        //Pre-rotate a and b
        t.a1 = ROTR16(d1,r1);
        t.a2 = ROTR16(d2,r1);
        tmp = (a1 - (b1 ^ key))^(NROUNDS-round_no);
        t.b1 = ROTL16(tmp,r2);
        tmp = (a2 - (b2 ^ key))^(NROUNDS-round_no);
        t.b2 = ROTL16(tmp,r2);
        t.c1 = ROTR16(b1,r1);
        t.c2 = ROTR16(b2,r1);
        t.d1 = c1;
        t.d2 = c2;
        
        //Get previous dz
        t.dz = trail.diffs[dsz-2-round_no].first.first;
    }

    bool check(word_t k, word_t mask) const {
        word_t z1 = (a1 - (b1 ^ k));
        word_t z2 = (a2 - (b2 ^ k));
        return !((dz ^ z1 ^ z2) & mask); // xors = saved xors
    }
};


void full_attack_bitwise_recursive(
    const vector<Trail> &trails,
    const vector<
        pair<  pair<pair<word_t, word_t>,pair<word_t, word_t>>, pair<pair<word_t, word_t>,pair<word_t, word_t>>  >
    > & testvecs,
    size_t min_trails,
    int nbits
) {
    g_states_values[0].reserve(trails.size());
    g_states_values[1].reserve(trails.size());
    g_states_values[2].reserve(trails.size());
    g_states_values[3].reserve(trails.size());
    g_states_values[4].reserve(trails.size());
    g_states_values[5].reserve(trails.size());
    g_states_values[6].reserve(trails.size());
    g_states_values[7].reserve(trails.size());

    g_states_values[0].clear();

    for (auto &trail : trails) {
		g_states_values[0].push_back(TrailState(trail));
        g_states_values[0].back().init_ct();
    }
    assert(g_states_values[0].size());
    
    g_min_trails = min_trails;
    g_trails = &trails;
    g_cur_keys.clear();
    g_n_bits = nbits;
    g_testvecs = &testvecs;
    g_completed = false;
    
    // Similar to Dinur, but smaller jump to save RAM (proof-of-concept)
    assert(1 <= INITIAL_JUMP && INITIAL_JUMP <= 14);
    word_t mask = (1ull << INITIAL_JUMP) - 1;
    vector<TrailState*> states;
    states.reserve(g_states_values[0].size());
    for (uint32_t k0 = 0; k0 <= mask; k0++) {
        states.clear();
        for (auto &state_value : g_states_values[0]) {
            if (state_value.check(k0, mask)) {
                states.push_back(&state_value);
            }
        }
        if (!(k0 & (mask >> 4))) {
           printf("progress: k0 %u/1 trails %lu\n", k0, states.size());
        }
        if (states.size()) {
            g_cur_keys = {(word_t)k0};
            attack_bitwise_recursive(states, 0, INITIAL_JUMP);
        }
        if (g_completed && (!fixed_trail)) {
            break;
        }
    }
    
    g_cur_keys.clear();
}

void states_init_round(
    const vector<TrailState*> & states,
    word_t key,
    int round_no
) {
    assert(states.size());
    vector<TrailState> & ret = g_states_values[round_no];
    ret.clear();
    unsigned r1 = 1, r2=8;
    if ((NROUNDS-round_no)%2) {
        r1 = 8;
        r2 = 1;
    }
    for (TrailState * state : states) {
        TrailState state2(state->trail);
        state->advance_to(state2, key, round_no,r1,r2);
        ret.push_back(state2);
    }
    return;
}

static void attack_bitwise_recursive(
    vector<TrailState*> &states,  // is ok to reuse
    int round_no,
    int pos // 0=LSB .. 15=MSB, but could be incremented by 2,3,4,...
) {

    if (g_completed && (!fixed_trail)) {
        return;
    }
    assert(pos != 15); // see optimization below

    if (round_no == 2) {
        // key candidate test
        if ((g_cur_keys[0]==0xa269) && (g_cur_keys[1]==0xa8ec) && (seed == 1)) { //18r (seed 1)
            if (g_completed == false) printf("Trail-key pairs contain the right key.\n");
           g_completed = true;
            return;
        }
		
		else if ((g_cur_keys[0]==0x35c1) && (g_cur_keys[1]==0x7cf1) && (seed == 5)) { //18r (seed 5)
            if (g_completed == false) printf("Trail-key pairs contain the right key.\n");
            //printf("%ld candidates tested to find right key\n", candidates);
            g_completed = true;
            return;
        }
    }
    
    if (round_no*16 + pos >= g_n_bits) {
        //Keep track of tra-key pairs
		candidates+=states.size();
		return;
    }
	
    // check # unique pairs if has chances to drop below the counter
    if (states.size() <= 30) {
        size_t n_unique_pairs = 0;
        size_t last_pair_id = -1ull;
        for(TrailState *state: states) {
            if (state->trail.pair_id != last_pair_id) {
                last_pair_id = state->trail.pair_id;
                n_unique_pairs++;
            }
        }
        if (n_unique_pairs < g_min_trails) {
		return;
        }
    }

    if (pos == 0) {
        assert(round_no != 0); // should be covered by INITIAL_JUMP

        // decrypt 1 round of (surviving) states
        states_init_round(
            states, g_cur_keys.back(), round_no
        );
        g_cur_keys.push_back(0);
        
        // refresh states ptrs
        states.clear();
        for (TrailState &state_value : g_states_values[round_no]) {
            states.push_back(&state_value);
        }
    }
    // check 1 more bit than guessed (ARX property)
    word_t mask = (1ull << (pos + 2)) - 1;

    word_t subkey0 = g_cur_keys.back();
    word_t subkey1 = subkey0 | (1 << pos);

    // if we filter given states list for one bit value at a time
    // we'll have original "large" list and a smaller one
    // but we can split it *in place* into two smaller ones
    // note: not partition, they can intersect or even be full duplicates
    vector<TrailState*> &states0 = states;
    vector<TrailState*> states1;
    states1.reserve(states.size());

    size_t orig = states.size();
    size_t top = 0;

    for (size_t i = 0; i < states0.size(); i++) {
        if (states0[i]->check(subkey1, mask)) {
            states1.push_back(states0[i]);
        }
        if (states0[i]->check(subkey0, mask)) {
            states0[top++] = states0[i];
        }
    }
    states0.erase(states0.begin() + top, states0.end());

    if (states0.size() >= g_min_trails) {
        if (pos <= 13) {
            g_cur_keys.back() = subkey0;
            attack_bitwise_recursive(states0, round_no, pos + 1);
        }
        else {
            // skip MSB guess and jump to next round
            assert(pos == 14);
            
            auto copy = states0;
            g_cur_keys.back() = subkey0;
            attack_bitwise_recursive(copy, round_no + 1, 0);
            copy.clear();

            g_cur_keys.back() = subkey0 ^ 0x8000;
            attack_bitwise_recursive(states0, round_no + 1, 0);
        }
    } else //{ printf("State 0 size = %d\n", states0.size());}
    states0.clear();

    if (states1.size() >= g_min_trails) {
        if (pos <= 13) {
            g_cur_keys.back() = subkey1;
            attack_bitwise_recursive(states1, round_no, pos + 1);
        }
        else {
            // skip MSB guess and jump to next round
            assert(pos == 14);
            
            auto copy = states1;
            g_cur_keys.back() = subkey1;
            attack_bitwise_recursive(copy, round_no + 1, 0);
            copy.clear();

            g_cur_keys.back() = subkey1 ^ 0x8000;
            attack_bitwise_recursive(states1, round_no + 1, 0);
        }
    } else //{ printf("State 1 size = %d\n", states1.size());}
    states1.clear();
    
    g_cur_keys.back() = subkey0;

    if (pos == 0) {
        g_cur_keys.pop_back();
    }
}
