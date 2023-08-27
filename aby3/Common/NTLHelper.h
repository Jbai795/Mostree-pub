//
// Created by 张晓武 on 2022/9/2.
//

#ifndef ABY3_NTLHELPER_H
#define ABY3_NTLHELPER_H
#include <iostream>
#include <NTL/ZZ_p.h> // integers mod p
#include <NTL/ZZ_pX.h> // polynomials over ZZ_p
#include <NTL/ZZ_pE.h> // ring/field extension of ZZ_p
#include <NTL/ZZ_pEX.h> // polynomials over ZZ_pE
#include <NTL/ZZ_pXFactoring.h>
#include <NTL/ZZ_pEXFactoring.h>
#include <NTL/GF2X.h>
#include <NTL/GF2E.h>
#include <NTL/GF2XFactoring.h>
#include <NTL/GF2XFactoring.h>
#include <vector>


using namespace std;
using namespace NTL;

//class NTLHelper {
//
//};

// #define I642POLY(v) to_GF2E(i64_GF2X((v)))
#define I642POLY(v) to_GF2E(u64_GF2X((v)))
#define POLY2I64(v) GF2X_i64(((v)._GF2E__rep))

// Note: Only get xrep[0] value
int64_t GF2X_i64(GF2X &data, long i = 0);
GF2X i64_GF2X(int64_t data);
GF2X u64_GF2X(u_int64_t data);

#endif //ABY3_NTLHELPER_H
