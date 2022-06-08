#pragma once

typedef unsigned short word_t;


#define ROTL16(x,r) (((x)<<(r)) | (x>>(16-(r))))
#define ROTR16(x,r) (((x)>>(r)) | ((x)<<(16-(r))))
#define ROL1(x) (((x)<<(1)) | (x>>(16-(1))))
#define ROR1(x) (((x)>>(1)) | ((x)<<(16-(1))))
#define ROL8(x) (((x)<<(8)) | (x>>(16-(8))))
#define ROR8(x) (((x)>>(8)) | ((x)<<(16-(8))))
#define ROL11(x) (((x)<<(11)) | (x>>(16-(11))))
#define ROR11(x) (((x)>>(11)) | ((x)<<(16-(11))))


const size_t NROUNDS = CHAM64_ATTACK;

#define ER0(a,b,k,i) (a^=i, b=ROL1(b), b^=k, a+=b, a=ROL8(a)) 
#define DR0(a,b,k,i) (a=ROR8(a), b = ROL1(b), b^=k, a-=b, a^=i)

#define ER1(a,b,k,i) (a^=i, b=ROL8(b), b^=k, a+=b, a=ROL1(a))  
#define DR1(a,b,k,i) (a=ROR1(a), b = ROL8(b), b^=k, a-=b, a^=i)

vector<word_t> chamKS_Revert(const vector<word_t> &last_subkeys_rev, int nrounds);
