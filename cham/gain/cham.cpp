#include "common.h"


vector<word_t> chamKS_Revert(const vector<word_t> &last_subkeys_rev, int nrounds) {
    assert(last_subkeys_rev.size() == 8); // currently supported
    vector<word_t> subkeys = last_subkeys_rev;
    word_t sortedKeys[8];

    //nrounds-1 is the last subkey, ctr = 0 represents last subkey
    int ctr = 0;
    word_t temp;
    for (int i = (nrounds - 1); i >= nrounds - 8; i--) {
        if ((i%16) < 8) { //Invert for 0,1,8
            sortedKeys[i%16] = ROTR16(subkeys[ctr], 2)^ROTR16(subkeys[ctr],10)^ROTR16(subkeys[ctr],1);
        }
        if ((i%16) >= 8) { //Invert for 0,1,11        
            sortedKeys[(i^1)%8] = subkeys[ctr]^ROTR16(subkeys[ctr], 2)^ROTR16(subkeys[ctr], 3)^ROTR16(subkeys[ctr], 4)^ROTR16(subkeys[ctr], 6)^ROTR16(subkeys[ctr], 7)^ROTR16(subkeys[ctr], 9)^ROTR16(subkeys[ctr], 12)^ROTR16(subkeys[ctr], 15);
        }
        ctr++;
    }

    for (int i = 0; i<8; i++) {
        subkeys[i] = sortedKeys[i];
    }

    return subkeys;
}
