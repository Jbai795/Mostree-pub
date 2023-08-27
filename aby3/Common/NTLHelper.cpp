//
// Created by 张晓武 on 2022/9/2.
//

#include "NTLHelper.h"
int64_t GF2X_i64(GF2X &data, long i) {
    if (IsZero(data))
        return 0;
    return data.xrep[i];
}
GF2X i64_GF2X(int64_t data) {
    GF2X ret;
    if (data > 0)
        ret.xrep.append(data);
    return ret;
}

GF2X u64_GF2X(u_int64_t data) {
    GF2X ret;
    if (data > 0)
        ret.xrep.append(data);
    return ret;
}
