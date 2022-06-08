#pragma once

#include <algorithm>
#include <random>
#include <chrono>

#include <cmath>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <ctime>

using namespace std;

#include "cham.h"


static inline word_t eq(word_t a, word_t b, word_t c){
    return (~a ^ b) & (~a ^ c) ;
}

static inline word_t g(word_t a, word_t b, word_t c){
    return eq(a<<1, b<<1, c<<1) & (a^b^c^(b<<1));
}

// static inline word_t mask(int s){
//     return (word_t)(1<<s)-1;
// }

static inline int wt(word_t val){
    return __builtin_popcountl(val);
}

// static inline int diffw_mask(word_t a, word_t b, word_t c, word_t msk){
//     return wt((~eq(a, b, c)) & (msk >> 1));
// }
// static inline int diffw(word_t a, word_t b, word_t c){
//     return !g(a, b, c) ? wt(~eq(a, b, c)) : 0 ;
// }


struct Trail {
	vector<pair<pair<word_t,word_t>,pair<word_t,word_t>>> diffs;

	pair<pair<word_t, word_t>,pair<word_t, word_t>> ct1, ct2;

	size_t pair_id;
};
