#include "SS3Dtree.h"

using namespace std;
using namespace oc;
using namespace aby3;

namespace mostree {

    void SS3Dtree::init(u64 partyIdx, oc::Session &prev, oc::Session &next, oc::block seed) {
        mPreproPrev = prev.addChannel();
        mPreproNext = next.addChannel();
        mPrev = prev.addChannel();
        mNext = next.addChannel();

        mPrev.waitForConnection();
        mNext.waitForConnection();

        std::chrono::milliseconds timeout(5000);
        while (!mNext.waitForConnection(timeout)) {

        }
        while (!mPrev.waitForConnection(timeout)) {

        }

        LOG(INFO) << "SS3Dtree idx: " << partyIdx << "  " << mNext.getSession().getName() << " isConnected: "
                  << mNext.isConnected();
        LOG(INFO) << "SS3Dtree idx: " << partyIdx << "  " << mPrev.getSession().getName() << " isConnected: "
                  << mPrev.isConnected();

        CommPkg c{mPrev, mNext};
        mRt.init(partyIdx, c);

        oc::PRNG prng(seed);
        mEnc.init(partyIdx, c, prng.get<aby3::block>());
        mEval.init(partyIdx, c, prng.get<aby3::block>());
        LOG(INFO) << "SS3Dtree idx: " << partyIdx << "  init ok!";
    }

    const sbMatrix SS3Dtree::getShareModelBinData(int partIdx, DtreeModel &model) {
        if (partIdx == 0) {
            localBinMatrix(model.getPlainModel(), model.ssModel);
        } else {
            remoteBinMatrix(model.ssModel);
        }
        return model.ssModel;
    }


    const sbMatrix SS3Dtree::getShareMACModelBinData(int partIdx, DtreeModel &model) {
        if (partIdx == 0) {
            localBinMatrix(model.getPlainMACModel(), model.ssMACModel);
        } else {
            remoteBinMatrix(model.ssMACModel);
        }
        return model.ssMACModel;
    }

    const sbMatrix SS3Dtree::getShareMACModelBinData(int partIdx, sbMatrix model_data, sbMatrix & dest_data, sb64 key) {
        sb64 temp;
        sb64 mul;
        for (u64 i = 0; i < model_data.mShares[0].size(); ++i){
            //mShareGen.getBinaryShare(), value is zero
            temp[0] = model_data.mShares[0](i);
            temp[1] = model_data.mShares[1](i);
            mul = poly_mul(temp, key);
            dest_data.mShares[0](i) = mul[0];
            dest_data.mShares[1](i) = mul[1];
            
            // std::cout << "ret.mShares[0](i): " << ret.mShares[0](i) << "\n";
            // std::cout << "mShareGen.getBinaryShare(): " << mShareGen.getBinaryShare() << "\n";
        }
        return dest_data;
    }
    //RSS binary share of MAC key
    const sb64 SS3Dtree::getShareMACKeyBinData(int partIdx, i64 val) {
        sb64 ret;
        if (partIdx == 0) {
            ret = localBinary(val);
        } else {
            ret = remoteZeroBinary();
        }
        return ret;
    }

    //64bit RSS binary share value
    const sb64 SS3Dtree::getShareBinData(int partIdx, u64 val) {
        sb64 ret = localBinary(val);
        return ret;
    }


    //8bit RSS binary share value
    const sb8 SS3Dtree::getShareBinData(int partIdx, u8 val) {
        sb8 ret = localBinary(val);
        return ret;
    }



    //RSS 64 bit arithmetic share value
    const su64 SS3Dtree::getShareU64Data(int partIdx, u64 val) {
        su64 ret = localU64(val);
        return ret;
    }

    const sb64 SS3Dtree::getShareRandBinData() {
        sb64 ret;
            ret = remoteBinary();
        return ret;
    }


    const sb64 SS3Dtree::getCovertP2NBinary(sb64 & sour) {
        sb64 dest;
        CovertP2NBinary(sour, dest);
        return dest;
    }

    const sb64 SS3Dtree::getCovertN2PBinary(sb64 & sour) {
        sb64 dest;
        CovertN2PBinary(sour, dest);
        return dest;
    }
    
    

    const sbMatrix SS3Dtree::shareFeatureSchema(int partIdx, DtreeModel &model) {
        if (partIdx == 0) {
            localBinMatrix(model.getFeatureSchema(), model.ssFeatureSchema);
        } else {
            remoteBinMatrix(model.ssFeatureSchema);
        }
        return model.ssFeatureSchema;
    }
    
    const su8Matrix SS3Dtree::shareValueSchema(int partIdx, u8Matrix plain, DtreeModel &model) {
        if (partIdx == 0) {
            localU8Matrix(plain, model.ssValueSchema);
        } else {
            remoteU8Matrix(model.ssValueSchema);
        }
        return model.ssValueSchema;
    }

    const sb64 SS3Dtree::shareSelectedIndex(int partIdx, DtreeModel &model, i64 &idx) {
        sb64 ret({0, 0});
        if (partIdx == 0) {
            ret = localBinary(idx);
        } else {
            ret = remoteBinary();
        }
        model.setSelectedIdx(idx);
        model.setSSSelectedIdx(ret);
        return ret;
    }

    const su8Matrix SS3Dtree::ssDGKFun(int partIdx, DtreeModel &model, u8Matrix& plain) {
        u64 bitLength = MODEL_BITS;
        su8Matrix ssValueSchema = model.getSSValueSchema();
        su8Matrix f(bitLength,1);
        su8Matrix t0(bitLength,1);
        su8Matrix t1(bitLength,1);
        su8Matrix t2(bitLength,1);

        // t0_i = [x_i] - y_i + 1
        // t1_i = [x_i] + y_i - [x_i] * y_i
        // t2_i = sum_(j<i){t1_j} 
        // f_i = [x_i] -y_i +1 + sum_(j<i){[x_i] + y_i - [x_i] * y_i} = t0_i + t2_i
        // p0和p1做加法和减法相关的操作，如在x0上 只做 x0 - y或者 x0 + y
        // 乘法每个元素都乘以 y
        if (partIdx == 0) {
            t0.mShares[0] = (ssValueSchema.mShares[0] - plain).array() + 1;
            t0.mShares[1] = ssValueSchema.mShares[1];

            t1.mShares[0] = ssValueSchema.mShares[0].array() + plain.array() - ssValueSchema.mShares[0].array() * plain.array();
            t1.mShares[1] = ssValueSchema.mShares[1].array() - ssValueSchema.mShares[1].array() * plain.array();

        }
        if (partIdx == 1) {
            t0.mShares[0] = ssValueSchema.mShares[0];
            t0.mShares[1] = (ssValueSchema.mShares[1] - plain).array() + 1;
            t1.mShares[0] = ssValueSchema.mShares[0].array() - ssValueSchema.mShares[0].array() * plain.array();
            t1.mShares[1] = ssValueSchema.mShares[1].array() + plain.array() - ssValueSchema.mShares[1].array() * plain.array();
        }

        if (partIdx == 2) {
            t0.mShares[0] = ssValueSchema.mShares[0];
            t0.mShares[1] = ssValueSchema.mShares[1];

            t1.mShares[0] = ssValueSchema.mShares[0].array() - ssValueSchema.mShares[0].array() * plain.array();
            t1.mShares[1] = ssValueSchema.mShares[1].array() - ssValueSchema.mShares[1].array() * plain.array();
        }
        
        //compute t2
        t2.mShares[0](0,0) = 0;
        t2.mShares[1](0,0) = 0;

        for (u64 i = 1; i < bitLength; i++) {
            t2.mShares[0](i,0) = t2.mShares[0](i-1,0) + t1.mShares[0](i-1,0);
            t2.mShares[1](i,0) = t2.mShares[1](i-1,0) + t1.mShares[1](i-1,0);
        
        }

        //compute f
        for (u64 i = 0; i < bitLength; i++)
        {
            f.mShares[0](i,0) = t0.mShares[0](i,0) + t2.mShares[0](i,0);
            f.mShares[1](i,0) = t0.mShares[1](i,0) + t2.mShares[1](i,0);
        }
        
        return f;
    }


    // Only test!!!
    // Note: b = (X[v] < t) ? 1 : 0
    // Return: >= return 0
    sb64 SS3Dtree::compare_fake(int pIdx, sb64 &x, sb64 &t) {
        sb64 ret({0, 0});
        i64 xx = reveal(x);
        i64 tt = reveal(t);
        if (xx > tt)
            return ret;
        else {
            if (pIdx == 0) {
                ret.mData[0] = 1;
            } else if (pIdx == 1) {
                ret.mData[1] = 1;
            }
        }
        return ret;
    }

    sb64 SS3Dtree::and_ss(sb64 &a, sb64 &b) {
        sb64 dest;
        mEval.asyncAnd(mRt.noDependencies(), a, b, dest).get();
        return dest;
    }

    // Note: ~X =  P0(~x0, x1)  P1(x1, x2)  P3(x2, ~x0)
    sb64 SS3Dtree::negate_ss(int pIdx, sb64 &x) {
        sb64 ret = x;
        if (DtreeModel::realIdx(pIdx) == 1)
            return ret;
        if (DtreeModel::realIdx(pIdx) == 0) {
            ret[1] = ~x[1];
        } else { //P2
            ret[0] = ~x[0];
        }
        return ret;
    }

    // std::array<sb64, 2> SS3Dtree::compare_process(sb64 &gt, sb64 &eq, int size) {
    //     std::array<sb64, 2> ret;
    //     ret[0].zero();
    //     ret[1].zero();
    //     for (int i = 0; i < size / 2; ++i) {
    //         int offset = i * 2;
    //         sb64 qt_tmp({0, 0});
    //         sb64 eq_tmp({0, 0});
    //         qt_tmp = (gt(offset + 1) ^ (eq(offset + 1) & gt(offset))) << i;
    //         eq_tmp = (eq(offset) & eq(offset + 1)) << i;
    //         ret[0] = ret[0] ^ qt_tmp;
    //         ret[1] = ret[1] ^ eq_tmp;
    //     }
    //     if (size == 2)
    //         return ret;
    //     return compare_process(ret[0], ret[1], size / 2);
    // }

    //semi-honest
    std::array<sb64, 2> SS3Dtree::compare_process(int partIdx, sb64 &gt, sb64 &eq, int size) {
        std::array<sb64, 2> ret;
        ret[0].zero();
        ret[1].zero();
        sb64 temp;
        if (partIdx == 0){
            temp.mData[0] = 1;
            temp.mData[1] = 0;
        }else if (partIdx == 1){
            temp.mData[0] = 0;
            temp.mData[1] = 1;
        }else{
            temp.mData[0] = 0;
            temp.mData[1] = 0;
        }        


        sb64 gt_temp({0, 0});
        sb64 eq_temp({0, 0});
        sb64 g({0, 0});
        sb64 e({0, 0});
        
        // compute eq(2*i) & eq(2*i + 1)
        sb64 eq_temp1 = eq >> 1;
        sb64 eq_temp2 = and_ss(eq, eq_temp1);

        // compute eq(2*i + 1) & gt(2*i)
        sb64 gt_temp2 = and_ss(gt, eq_temp1);

        //compute gt(2*i+1) ^ gt_temp2(2*i)
        sb64 gt_temp1 = gt >> 1;
        sb64 gt_temp3 = gt_temp1 ^ gt_temp2;


        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing eq_temp1 --------------- "<< i << " location is " << reveal(eq_temp1(i));
        // }
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing eq_temp2 --------------- "<< i << " location is " << reveal(eq_temp2(i));
        // }

        // LOG(INFO) <<"*************gt(24) ************ " << reveal(gt(24));
        // LOG(INFO) <<"*************eq(24) ************ " << eq(24);
        // LOG(INFO) <<"*************gt(25) ************ " << reveal(gt(25));
        // LOG(INFO) <<"*************eq(25) ************ " << eq(25);
        // LOG(INFO) <<"----------------------------------------------------------" ;
        // gt_temp(24) = gt(25)^ (eq(25) & gt(24));

        // should not use & directly as and requires communication
        // eq_temp(24) = eq(24) & eq(25);
        // LOG(INFO) <<"*************test gt_temp(24) ************ " << gt_temp(24);
        // LOG(INFO) <<"*************test eq_temp(24) ************ " << eq_temp(24);

        // int c;
        // for (int i = 0; i < size / 2; ++i) {
        //     c = 2 * i;
        //     gt_temp(i) = gt_temp3(c);
        //     eq_temp(i) = eq_temp2(c);
        //     // LOG(INFO) <<"***************************** ************ ";
        //     if (i ==0)
        //         LOG(INFO) << i << " gt_temp(0) ************ " << reveal(gt_temp(0));
        // }
        sb64 c;
        sb64 gc;
        sb64 ec;
        for (int i = 0; i < size / 2; ++i) {
            c = temp << (2*i);
            // for (int j = 0; j < 10; j++){
            //     LOG(INFO) <<"testing temp --------------- "<< j << " location is " << reveal(c(j));
            // }
            g = and_ss(c, gt_temp3);
            gc = g >> (2*i -i);
            gt_temp = gt_temp ^ gc;


            e = and_ss(c, eq_temp2);
            ec = e >> (2*i -i);
            eq_temp = eq_temp ^ ec;  
        }
        
        ret[0] = ret[0] ^ gt_temp(0);
        ret[1] = ret[1] ^ eq_temp(0);
        // LOG(INFO) <<"gt_temp(0) ************ " << reveal(gt_temp(0));
        // ret[0] = gt_temp(0);
        // LOG(INFO) <<"************ " << reveal(ret[0]);
        // LOG(INFO) <<"************ " << revint pIdx, eal(ret[1]);
        // return ret;
        if (size == 2)
            return ret;
        return compare_process(partIdx, gt_temp, eq_temp, size / 2);
    }

    //semi-honest

    // This is a greater than comparison process
    // If b = 2 in decimal and 00000010 in 8 bits binary, b(0)=0, b(1)=1, b(2)=0,...,b(7)=0
    // It naturally matches with greater than comparison process.
    sb64 SS3Dtree::compare_ss(int pIdx, sb64 &a, sb64 &b) {
        sb64 neg_b = negate_ss(pIdx, b);
        // LOG(INFO) <<"negate_ss(pIdx, b) ************ " << neg_b;
        // LOG(INFO) <<"negate_ss(pIdx, b) ************ " << reveal(neg_b);
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing bit order ************ "<< i << " location is " << reveal(b(i));
        // }
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing negate bit order --------------- "<< i << " location is " << reveal(neg_b(i));
        // }
        sb64 gt = and_ss(a, neg_b);
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing value a ************ "<< i << " location is " << reveal(a(i));
        // }
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing and_ss --------------- "<< i << " location is " << reveal(gt(i));
        // }
        // LOG(INFO) <<"and_ss(a, neg_b) ************ " << gt;
        // LOG(INFO) <<"and_ss(a, neg_b) ************ " << reveal(gt);

        sb64 gt_test = and_ss(a, b);
        // LOG(INFO) <<"and_ss(a, neg_b) ************ " << gt_test;
        // LOG(INFO) <<"and_ss(a, neg_b) ************ " << reveal(gt_test);
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing a --------------- "<< i << " location is " << reveal(a(i));
        // }
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing b --------------- "<< i << " location is " << reveal(b(i));
        // }
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing gt_test --------------- "<< i << " location is " << reveal(gt_test(i));
        // }

        sb64 a_xor_b = a ^ b;
        sb64 eq = negate_ss(pIdx, a_xor_b);
        // LOG(INFO) <<"negate_ss(pIdx, a_xor_b) ************ " << reveal(eq);
        

        auto ret = compare_process(pIdx, gt, eq, MODEL_BITS);
        // auto ret = compare_process(gt, eq, MODEL_BITS);
        
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing gt ************ "<< i << " location is " << reveal(gt(i));
        // }
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing eq ************ "<< i << " location is " << reveal(eq(i));
        // }
        return ret[0];
    }

    // check if and_ss is correctly performed
    void SS3Dtree::check_and_ss(int pIdx, sb64 x, sb64 y, sb64 z, sb64 as, sb64 bs, sb64 cs){
        // using triples (c = a and b) to check multiplication
        sb64 e,f,fa,eb,rec;
        e = x ^ as; // x xor a
        f = y ^ bs; // y xor b
        uint64_t e_open = reveal(e);
        uint64_t f_open = reveal(f); //*check open values are same
        uint64_t ef = e_open & f_open;

        fa[0] = f_open & as[0];
        fa[1] = f_open & as[1];

        eb[0] = e_open & bs[0];
        eb[1] = e_open & bs[1];

        if(pIdx == 0){
            rec[0] = ef ^ fa[0] ^ eb[0] ^ cs[0] ^ z[0];
            rec[1] = fa[1] ^ eb[1] ^ cs[1] ^ z[1];
        }else if(pIdx == 1){
            rec[0] = fa[0] ^ eb[0] ^ cs[0] ^ z[0];
            rec[1] = ef ^ fa[1] ^ eb[1] ^ cs[1] ^ z[1];
        }else if(pIdx == 2){
            rec[0] = fa[0] ^ eb[0] ^ cs[0] ^ z[0];
            rec[1] = fa[1] ^ eb[1] ^ cs[1] ^ z[1];
        }

        if (reveal(rec) == 0){
            // LOG(INFO) << "****** Multiplication check in AND_SS passed! ******";
        }else{
            // abort
            // LOG(INFO) << "****** Multiplication check in AND_SS not passed! ******";
        }
    }


    // malicious
    std::array<sb64, 2> SS3Dtree::compare_process(int partIdx, sb64 &gt, sb64 &eq, int size, sb64 *as, sb64 *bs, sb64 *cs, int val, int incr) {
        std::array<sb64, 2> ret;
        ret[0].zero();
        ret[1].zero();
        sb64 temp;
        if (partIdx == 0){
            temp.mData[0] = 1;
            temp.mData[1] = 0;
        }else if (partIdx == 1){
            temp.mData[0] = 0;
            temp.mData[1] = 1;
        }else{
            temp.mData[0] = 0;
            temp.mData[1] = 0;
        }       

        sb64 gt_temp({0, 0});
        sb64 eq_temp({0, 0});
        sb64 g({0, 0});
        sb64 e({0, 0});
        
        // compute eq(2*i) & eq(2*i + 1)
        sb64 eq_temp1 = eq >> 1;
        sb64 eq_temp2 = and_ss(eq, eq_temp1);

        int se = 140*val + incr;
        check_and_ss(partIdx, eq, eq_temp1, eq_temp2, as[se + 2], bs[se + 2], cs[se + 2]);
        

        // compute eq(2*i + 1) & gt(2*i)
        sb64 gt_temp2 = and_ss(gt, eq_temp1);
        check_and_ss(partIdx, gt, eq_temp1, gt_temp2, as[se + 3], bs[se + 3], cs[se + 3]);

        //compute gt(2*i+1) ^ gt_temp2(2*i)
        sb64 gt_temp1 = gt >> 1;
        sb64 gt_temp3 = gt_temp1 ^ gt_temp2;

        sb64 c;
        sb64 gc;
        sb64 ec;
        for (int i = 0; i < size / 2; ++i) {
            c = temp << (2*i);
            // for (int j = 0; j < 10; j++){
            //     LOG(INFO) <<"testing temp --------------- "<< j << " location is " << reveal(c(j));
            // }
            g = and_ss(c, gt_temp3);
            check_and_ss(partIdx, c, gt_temp3, g, as[se + 4 + 2*i], bs[se + 4 + 2*i], cs[se + 4 + 2*i]);
            gc = g >> (2*i -i);
            gt_temp = gt_temp ^ gc;


            e = and_ss(c, eq_temp2);
            check_and_ss(partIdx, c, eq_temp2, e, as[se + 5 + 2*i], bs[se + 5 + 2*i], cs[se + 5 + 2*i]);
            ec = e >> (2*i -i);
            eq_temp = eq_temp ^ ec;  
        }
        
        ret[0] = ret[0] ^ gt_temp(0);
        ret[1] = ret[1] ^ eq_temp(0);
        if (size == 2)
            return ret;
        int acumu = size + 2 + incr; 
        return compare_process(partIdx, gt_temp, eq_temp, size / 2, as, bs, cs, val, acumu);
    }

    // This is a greater than comparison process
    // If b = 2 in decimal and 00000010 in 8 bits binary, b(0)=0, b(1)=1, b(2)=0,...,b(7)=0
    // It naturally matches with greater than comparison process.
    sb64 SS3Dtree::compare_ss(int pIdx, sb64 &a, sb64 &b, sb64 *as, sb64 *bs, sb64 *cs, int val) {
        sb64 neg_b = negate_ss(pIdx, b);
        // LOG(INFO) <<"negate_ss(pIdx, b) ************ " << neg_b;
        // LOG(INFO) <<"negate_ss(pIdx, b) ************ " << reveal(neg_b);
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing bit order ************ "<< i << " location is " << reveal(b(i));
        // }
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing negate bit order --------------- "<< i << " location is " << reveal(neg_b(i));
        // }
        sb64 gt = and_ss(a, neg_b);
        check_and_ss(pIdx, a, neg_b, gt, as[140*val + 0], bs[140*val + 0], cs[140*val + 0]);
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing value a ************ "<< i << " location is " << reveal(a(i));
        // }
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing and_ss --------------- "<< i << " location is " << reveal(gt(i));
        // }
        // LOG(INFO) <<"and_ss(a, neg_b) ************ " << gt;
        // LOG(INFO) <<"and_ss(a, neg_b) ************ " << reveal(gt);

        sb64 gt_test = and_ss(a, b);
        check_and_ss(pIdx, a, b, gt_test, as[140*val + 1], bs[140*val + 1], cs[140*val + 1]);
        // LOG(INFO) <<"and_ss(a, neg_b) ************ " << gt_test;
        // LOG(INFO) <<"and_ss(a, neg_b) ************ " << reveal(gt_test);
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing a --------------- "<< i << " location is " << reveal(a(i));
        // }
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing b --------------- "<< i << " location is " << reveal(b(i));
        // }
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing gt_test --------------- "<< i << " location is " << reveal(gt_test(i));
        // }

        sb64 a_xor_b = a ^ b;
        sb64 eq = negate_ss(pIdx, a_xor_b);
        // LOG(INFO) <<"negate_ss(pIdx, a_xor_b) ************ " << reveal(eq);
        
        int incr = 0;
        auto ret = compare_process(pIdx, gt, eq, MODEL_BITS, as, bs, cs, val, incr);
        // auto ret = compare_process(gt, eq, MODEL_BITS);
        
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing gt ************ "<< i << " location is " << reveal(gt(i));
        // }
        // for (int i = 0; i < MODEL_BITS; i++){
        //     LOG(INFO) <<"testing eq ************ "<< i << " location is " << reveal(eq(i));
        // }
        return ret[0];
    }

    /*
     * Note: b = (X[v] < t) ? 1 : 0
     * Params: x: feature value   t: threshold
     * Return: t>x return 1
     */
    //semi-honest
    sb64 SS3Dtree::compare_threshold(int pIdx, sb64 &x, sb64 &t) {
        // return compare_ss(pIdx, t, x);
        return compare_ss(pIdx, t, x);
    }


    //malicious
    sb64 SS3Dtree::compare_threshold(int pIdx, sb64 &x, sb64 &t, sb64 *as, sb64 *bs, sb64 *cs, int val) {
        // return compare_ss(pIdx, t, x);
        return compare_ss(pIdx, t, x, as, bs, cs, val);
    }

    void SS3Dtree::fssPreprocess(int pIdx, DtreeModel &model) {
        model.fssPreprocess(pIdx);
    }

    void SS3Dtree::realFeatureFssPreprocess(int pIdx, DtreeModel &model) {
        model.realFeatureFssPreprocess(pIdx);
    }

    void SS3Dtree::realTreeFssPreprocess(int pIdx, DtreeModel &model) {
        model.realTreeFssPreprocess(pIdx);
    }

    // X: shared model table
    sbMatrix SS3Dtree::semi_ob_selection_matrix(int partIdx, sb64 &ssSelectedIdx, sbMatrix &X, FssGen &fssgen) {
        // int real_idx = DtreeModel::realIdx(partIdx); 
        i64 d02 = ssSelectedIdx[0] ^ ssSelectedIdx[1] ^ fssgen.r[0];
        i64 d01 = ssSelectedIdx[0] ^ fssgen.r[1];
        
        sb64 data1({d02, d01});
        sb64 data2({d02, d01});

        sb64 r1 = getCovertP2NBinary(data1);
        sb64 r2 = getCovertN2PBinary(data2);

        // LOG(INFO) << "P" << partIdx << " data1 is " << data1;
        // LOG(INFO) << "P" << partIdx << " r1 is " << r1;
        // LOG(INFO) << "P" << partIdx << " r2 is " << r2;

        sb64 ssDelta;
        ssDelta[0] = r1[0] ^ r2[0] ^ d02;
        ssDelta[1] = r1[1] ^ r2[1] ^ d01;
        // LOG(INFO) << "P" << partIdx << " d02 is " << d02;

        // LOG(INFO) << "P" << partIdx << " ssDelta is " << ssDelta;
        // LOG(INFO) << "P" << partIdx << " ssDelta is " << reveal(ssDelta);

        u64 n = X.rows();
        i64Matrix y01(1, X.i64Cols());
        y01.setZero();
        for (i64 i = 0; i < n; ++i) {
            i64Matrix tmp;
            tmp = X(i ^ ssDelta[0], 0) * fssgen.k.mShares[0](i);
            for (int j = 0; j < tmp.size(); ++j) {
                y01(j) = y01(j) ^ tmp(j);
            }
        }

        i64Matrix y00(1, X.i64Cols());
        y00.setZero();
        for (i64 i = 0; i < n; ++i) {
            i64Matrix tmp;
            tmp = X(i ^ ssDelta[1], 1) * fssgen.k.mShares[1](i);
            for (int j = 0; j < tmp.size(); ++j) {
                y00(j) = y00(j) ^ tmp(j);
            }
        }

        i64Matrix selectedY(1, y00.size());
        for (int j = 0; j < y00.size(); ++j) {
            selectedY(j) = y00(j) ^ y01(j);
        }

        // Reshare
        sbMatrix ssY(1, X.bitCount());
        localBinMatrix(selectedY, ssY);
        return ssY;
    }

    // sb64 SS3Dtree::semi_ob_selection_y(int partIdx, sb64 &ssSelectedIdx, sbMatrix &X, FssGen &fssgen) {
    //     // int real_idx = DtreeModel::realIdx(partIdx);
    //     i64 d02 = ssSelectedIdx[0] ^ ssSelectedIdx[1] ^ fssgen.r[0];
    //     mNext.asyncSendCopy(d02);

    //     i64 d01 = ssSelectedIdx[0] ^ fssgen.r[1];
    //     mPrev.asyncSendCopy(d01);

    //     i64 d12 = 0;
    //     mNext.recv(d12);

    //     i64 d21 = 0;
    //     mPrev.recv(d21);

    //     sb64 ssDelta;
    //     ssDelta[0] = d02 ^ d12;
    //     ssDelta[1] = d21 ^ d01;

    //     u64 n = X.rows();
    //     i64 y01 = 0;
    //     for (i64 i = 0; i < n; ++i) {
    //         y01 ^= (X.mShares[0](i ^ ssDelta[0]) * fssgen.k.mShares[0](i));
    //     }

    //     i64 y00 = 0;
    //     for (i64 i = 0; i < n; ++i) {
    //         y00 ^= X.mShares[1](i ^ ssDelta[1]) * fssgen.k.mShares[1](i);
    //     }

    //     i64 selectedY = y01 ^ y00;

    //     // Reshare
    //     sb64 ssY = localBinary(selectedY);
    //     return ssY;
    // }

    sb64 SS3Dtree::semi_ob_selection_y(int partIdx, sb64 &ssSelectedIdx, sbMatrix &X, FssGen &fssgen) {
        // int real_idx = DtreeModel::realIdx(partIdx);
        i64 d02 = ssSelectedIdx[0] ^ ssSelectedIdx[1] ^ fssgen.r[0];
        i64 d01 = ssSelectedIdx[0] ^ fssgen.r[1];
        
        sb64 data1({d02, d01});
        sb64 data2({d02, d01});

        sb64 r1 = getCovertP2NBinary(data1);
        sb64 r2 = getCovertN2PBinary(data2);

        // LOG(INFO) << "P" << partIdx << " data1 is " << data1;
        // LOG(INFO) << "P" << partIdx << " r1 is " << r1;
        // LOG(INFO) << "P" << partIdx << " r2 is " << r2;

        sb64 ssDelta;
        ssDelta[0] = r1[0] ^ r2[0] ^ d02;
        ssDelta[1] = r1[1] ^ r2[1] ^ d01;
        // LOG(INFO) << "P" << partIdx << " d02 is " << d02;

        // LOG(INFO) << "P" << partIdx << " ssDelta is " << ssDelta;
        // LOG(INFO) << "P" << partIdx << " ssDelta is " << reveal(ssDelta);

        u64 n = X.rows();
        i64 y01 = 0;
        for (i64 i = 0; i < n; ++i) {
            y01 ^= (X.mShares[0](i ^ ssDelta[0]) * fssgen.k.mShares[0](i));
        }

        i64 y00 = 0;
        for (i64 i = 0; i < n; ++i) {
            y00 ^= X.mShares[1](i ^ ssDelta[1]) * fssgen.k.mShares[1](i);
        }

        i64 selectedY = y01 ^ y00;

        // Reshare
        sb64 ssY = localBinary(selectedY);
        return ssY;
    }

    /*** Semi-honest oblivious selection feature protocol ***/
    sb64 SS3Dtree::semi_ob_selection_feature(int partIdx, sbMatrix &feature, sb64 selected_idx, DtreeModel &model,
                                             bool isFss) {
        /*
         * Semi-honest oblivious selection protocol
         * Step 3 Preprocess
        */
        if (isFss) {
            realFeatureFssPreprocess(partIdx, model);
        }
        /*
         * Semi-honest oblivious selection protocol
         * Step 4 Online
         */
        return semi_ob_selection_y(partIdx, selected_idx, feature, model.mFssGenFeature);
    }

    /*** Semi-honest oblivious selection dtree node protocol ***/
    sbMatrix SS3Dtree::semi_ob_selection_node(int partIdx, sbMatrix &model_table, sb64 selected_node_idx, DtreeModel &model,
                                     bool isFss) {
        model.setSSModel(model_table);
        /*
         * Semi-honest oblivious selection protocol
         * Step 3 Preprocess
        */
        if (isFss) {
            realTreeFssPreprocess(partIdx, model);
            // if (partIdx == 0) {
            //     LOG(INFO) << "Model share fss r partyIdx: " << partIdx << " " << model.mFssGenNodes.r;
            //     LOG(INFO) << "Model share fss k partyIdx: " << partIdx;
            //     model.mFssGenNodes.k.printAllShareBits();
            // }
        }
        /*
         * Semi-honest oblivious selection protocol
         * Step 4 Online
         */
        return semi_ob_selection_matrix(partIdx, selected_node_idx, model_table, model.mFssGenNodes);
    }


    /*** Semi-honest oblivious selection dtree node protocol ***/
    sbMatrix SS3Dtree::semi_ob_selection_mac_node(int partIdx, sbMatrix &model_table, sb64 selected_node_idx, DtreeModel &model,
                                     bool isFss) {
        model.setSSMACModel(model_table);
        /*
         * Semi-honest oblivious selection protocol
         * Step 3 Preprocess
        */
        if (isFss) {
            realTreeFssPreprocess(partIdx, model);
            if (partIdx == 0) {
                LOG(INFO) << "Model share fss r partyIdx: " << partIdx << " " << model.mFssGenNodes.r;
                LOG(INFO) << "Model share fss k partyIdx: " << partIdx;
                model.mFssGenNodes.k.printAllShareBits();
            }
        }
        /*
         * Semi-honest oblivious selection protocol
         * Step 4 Online
         */
        return semi_ob_selection_matrix(partIdx, selected_node_idx, model_table, model.mFssGenNodes);
    }

    /*
     *  Function: next index = r xor b * (l xor r)
     *  Note: b = (X[v] < t) ? 1 : 0
     */
    sb64 SS3Dtree::getChildNodeIndex(sb64 &ssb, sb64 &left, sb64 &right) {
        sb64 rl = left ^ right;
        sb64 tmp = poly_mul(ssb, rl);
        return sb64(right ^ tmp);
    }

    sb64 SS3Dtree::getChildNodeIndex(int pIdx, sb64 &ssb, sb64 &left, sb64 &right,sb64 a,sb64 b,sb64 c) {
        sb64 rl = left ^ right;
        sb64 tmp = poly_mul(ssb, rl);

        // using triples (c = a*b) to check multiplication
        sb64 e,f,fa,eb,z;
        e = ssb ^ a; // x-a
        f = rl ^ b; // y-b
        uint64_t e_open = reveal(e);
        uint64_t f_open = reveal(f); //*check open values are same
        GF2E polu_ef = I642POLY(f_open) * I642POLY(e_open);
        uint64_t ef = POLY2I64(polu_ef);

        GF2E fa0 = I642POLY(f_open) * I642POLY(a[0]);
        GF2E fa1 = I642POLY(f_open) * I642POLY(a[1]);
        fa[0] = POLY2I64(fa0);
        fa[1] = POLY2I64(fa1);

        GF2E eb0 = I642POLY(e_open) * I642POLY(b[0]);
        GF2E eb1 = I642POLY(e_open) * I642POLY(b[1]);
        eb[0] = POLY2I64(eb0);
        eb[1] = POLY2I64(eb1);

        if(pIdx == 0){
            z[0] = ef ^ fa[0] ^ eb[0] ^ c[0] ^ tmp[0];
            z[1] = fa[1] ^ eb[1] ^ c[1] ^ tmp[1];
        }else if(pIdx == 1){
            z[0] = fa[0] ^ eb[0] ^ c[0] ^ tmp[0];
            z[1] = ef ^ fa[1] ^ eb[1] ^ c[1] ^ tmp[1];
        }else if(pIdx == 2){
            z[0] = fa[0] ^ eb[0] ^ c[0] ^ tmp[0];
            z[1] = fa[1] ^ eb[1] ^ c[1] ^ tmp[1];
        }

        if (reveal(z) == 0){
            // LOG(INFO) << "****** Multiplication check in getChildNodeIndex passed! ******";
            return sb64(right ^ tmp);
        }else{
            // LOG(INFO) << "****** Multiplication check in getChildNodeIndex not passed! ******";
            return sb64(right ^ tmp); // should abort
        }
        
    }


    int SS3Dtree::mac_check(int pIdx, sb64 &x, sb64 &y, sb64 &key) {
        // return compare_ss(pIdx, t, x);
        sb64 tmp1 = poly_mul(x, key);
        sb64 tmp2 = tmp1 ^ y;
        return reveal(tmp2);
    }


    void SS3Dtree::comparison_DGK(DtreeModel &model, su8Matrix ssValueScheme, u8Matrix y, int value_depth, int pIdx,
                    su64 &compare_result, su64 &maced_compare_result, su64 ss_mac_key_for_2to3){
        
        /* compute DGK vector dgkScheme */
        su8Matrix ssDGKSchema = ssDGKFun(pIdx, model, y);

        //TODO make p0 sends DPF key pairs and p1.p2 receives DPF key pairs offline
        /* BATCH DPF test offline phase
        party0 sends 2^depth DPF keys to party1 and party2 respectively */
        const uint32_t N = 1u << value_depth;  
        // ================= party 0: keygen + send =================
        if (pIdx == 0) {
            sendDPFKeyBatchFromParty0(value_depth, ss_mac_key_for_2to3);//party0生成f(x) = (1,beta2) if x == alpha, else f(x) = (0,0) DPF keys，并发送给party1和party2
        }

        // ========= 接收 batch DPF keys + rdx shares =========
        if (pIdx == 1 || pIdx == 2) recvDPFKeyBatch();
        std::cout << "[BATCH-TEST] party" << pIdx << " batch received.\n";

        #ifdef SET_MALICIOUS 
        //BATCH DPF test online phase
        // ================= party 0: send batch =================
        if(pIdx == 0){
            sendMaskedRdxBatch(ssDGKResult); // u8 vector
        }
        #endif


        // ================= party 1/2: recv batch =================
        if (pIdx == 1 || pIdx == 2) {
            #ifdef SET_MALICIOUS 
            // ========= step 接收来自party0的 masked rdx batch =========
            recvMaskedRdxBatch(); // u8 vector
            std::cout << "[BATCH-TEST] party" << pIdx << " masked rdx batch received.\n";
            #endif
            // ========= step 1: 本地准备 masked rdx batch =========
            std::vector<u8> my_to_peer;
            prepareMaskedRdxBatch(ssDGKSchema, my_to_peer);    // su8Matrix

            // ========= step 2: 一次性交换 masked rdx =========
            std::vector<u8> peer_to_me;
            exchangeMaskedRdxBatch(
                my_to_peer,
                peer_to_me
            );

            #ifdef SET_MALICIOUS
            maskedRdxCheck(peer_to_me);
            #endif

            // ========= step 3: 本地 batch eval =========
            vector <uint64_t> dpfShares;
            dpfShares.resize(getDPFN());
            vector <uint64_t> dpfMACShares;
            dpfMACShares.resize(getDPFN());
            DpfEvalInt2 dpfOutput;
            uint64_t dpfSharesSum = 0;
            uint64_t dpfMACSharesSum = 0;
            for (size_t i = 0; i < getDPFN(); ++i) {
                dpfOutput = evalDPFBatchWithMAC(i, peer_to_me, ssDGKSchema); // 对应 peer_to_me 的 index
                dpfShares[i] = dpfOutput.v1;
                dpfMACShares[i] = dpfOutput.v2;
                dpfSharesSum += dpfShares[i];
                dpfMACSharesSum += dpfMACShares[i];
            }

            if(pIdx==1)
                dpfMACSharesSum = dpfSharesSum * ss_mac_key_for_2to3[0] + dpfMACSharesSum;
            else//pIdx==2
                dpfMACSharesSum = dpfSharesSum * ss_mac_key_for_2to3[1] + dpfMACSharesSum;
            
            //p1 and p2 reshare comparison result and maced comparison result
            compare_result = getShareU64Data(pIdx, dpfSharesSum);
            maced_compare_result = getShareU64Data(pIdx, dpfMACSharesSum);    
        }

        if(pIdx == 0){
            //p0 reshare
            compare_result = getShareU64Data(pIdx, u64(0));
            maced_compare_result = getShareU64Data(pIdx, u64(0));
        }
    }


    sb8 SS3Dtree::bin_xor(sb8 &a, sb8 &b){
        sb8 res;
        res[0] = a[0] ^ b[0];
        res[1] = a[1] ^ b[1];
        return res;
    }

    // convert an RSS bit (0/1) to RSS u64
    // x \xor y = x + y -2xy iff x y \in {0,1}
    su64 SS3Dtree::bit2A(sb8 b_r, su64 a_r, sb8 b_x, int pIdx){
        sb8 tem;
        su64 res;
        tem = bin_xor(b_r, b_x);
        u8 reveal_c = reveal(tem);
        if(pIdx == 0){
            res[0] = a_r[0] + reveal_c - 2 * a_r[0] * reveal_c;
            res[1] = a_r[1] - 2 * a_r[1] * reveal_c;
        }else if(pIdx == 1){
            res[0] = a_r[0] - 2 * a_r[0] * reveal_c;
            res[1] = a_r[1] + reveal_c - 2 * a_r[1] * reveal_c;
        }else if(pIdx == 2){
            res[0] = a_r[0] - 2 * a_r[0] * reveal_c;
            res[1] = a_r[1] - 2 * a_r[1] * reveal_c;
        }
        return res;
        
    }


    // semi-honest
    sb64 SS3Dtree::travelTree(int pIdx, int depth, SSNode &root, DtreeModel &model, sbMatrix &ss_model_data, sbMatrix &ssFeatureSchema) {
        SSNode cur_node = root;

        for (int i = 0; i < depth; ++i) {
            if (pIdx == 0)
                LOG(INFO) << "****** Travel Decision Tree Layer: [ " << i << " ] ******";
            sb64 ssSelectedIdx = cur_node.getVIdx();
            // LOG(INFO) << "P" << pIdx << " selected feature V index: " << reveal(ssSelectedIdx);
            sb64 ssY = semi_ob_selection_feature(pIdx, ssFeatureSchema, ssSelectedIdx, model, false);
            
            // LOG(INFO) << "P" << pIdx << " selected feature V index: " << reveal(ssSelectedIdx) << " -> X: " << reveal(ssY);

            sb64 threshold = cur_node.getThreshold();
            sb64 left = cur_node.getLeft();
            sb64 right = cur_node.getRight();

            // LOG(INFO) << "P" << pIdx << " right: " << right << " reveal " << reveal(right);
            // LOG(INFO) << "P" << pIdx << " left: " << left << " reveal " << reveal(left);
            // 0: right child;  1: left child
            // LOG(INFO) << "P" << pIdx << " ssY: " << ssY;
            // LOG(INFO) << "P" << pIdx << " threshold: " << threshold;
            // LOG(INFO) << "P" << pIdx << " compare: (" << reveal(ssY) << ", " << reveal(threshold) << ")";
            sb64 ssb = compare_threshold(pIdx, ssY, threshold);
            // LOG(INFO) << "P" << pIdx << " compare_result: " << ssb << " -> X: " << reveal(ssb);
            sb64 ssNextNodeIdx = getChildNodeIndex(ssb, left, right);
            // LOG(INFO) << "P" << pIdx << " ssb: " << ssb << " -> expect NextNodeIdx: " << ssNextNodeIdx << " reveal " << reveal(ssNextNodeIdx);

            //selection of model
            sbMatrix next_node = semi_ob_selection_node(pIdx, ss_model_data, ssNextNodeIdx, model, false);
            i64Matrix plain_ssnode = reveal(next_node);
            
            // LOG(INFO) << "P" << pIdx << " real selected node [" << reveal(ssNextNodeIdx) << "] : " << plain_ssnode;


            // i64Matrix plain_ssnode = reveal(next_node);
            cur_node.setSSData(next_node);
        }

            LOG(INFO) << "Semi-honest tree travel success! ";
            return cur_node.getLabel();    
    }


    // malicious
    sb64 SS3Dtree::travelTree(int pIdx, int depth, SSNode &root, DtreeModel &model, sbMatrix &ss_model_data, sbMatrix &ss_mac_model_data,
                              sbMatrix &ssFeatureSchema, sb64 &ss_mac_key, sb64 gamma, sb64 *rho, sb64 *a,sb64 *b,sb64 *c, sb64 *as,sb64 *bs,sb64 *cs) {
        SSNode cur_node = root;
        SSMACNode cur_mac_node;
        //save for mac checking
        sb64 sum_model_data({0, 0});
        sb64 sum_mac_model_data({0, 0});
        u64 selected_part_mac_key = 3;
        su64 ss_mac_key_for_2to3 = getShareU64Data(pIdx, selected_part_mac_key);
        //depth is tree depth, value_depth is bit length of model value
        uint32_t value_depth = (uint32_t)std::ceil(std::log2((double)MODEL_BITS));


        // suppose a pre-shared r^A and r^B between parties
        //TODO A2B conversion
        u64 r1;
        su64 a_r;
        if (pIdx == 0) {// 1 = -4+2+3
            r1 = -4;
            a_r = getShareU64Data(pIdx, r1);
        }else if(pIdx == 1){
            r1 = 2;
            a_r = getShareU64Data(pIdx, r1);
        }else{
            r1 = 3;
            a_r = getShareU64Data(pIdx, r1);
        }
        
        u8 r2;
        sb8 b_r;

        if (pIdx == 0) {// 1 = 6 xor 11 xor 12
            r2 = 6;
            b_r = getShareBinData(pIdx, r2);
        }else if(pIdx == 1){
            r2 = 11;
            b_r = getShareBinData(pIdx, r2);
        }else{
            r2 = 12;
            b_r = getShareBinData(pIdx, r2);
        }

        sb8 b_s;
        u64 thre = 214;
        sb64 ss_thre = getShareBinData(pIdx, thre);// u64 RSS shared
        su64 tem = su64::zero();
        for(int i = 0; i < MODEL_BITS; i++){
            b_s = ss_thre.getBitShare(i);
            tem = tem + (1ULL << i) * bit2A(b_r, a_r, b_s, pIdx);
        }

        u64 attri = 150;
        sb64 ss_attri = getShareBinData(pIdx, attri);// u64 RSS shared
        su64 tem2 = su64::zero();
        for(int i = 0; i < MODEL_BITS; i++){
            b_s = ss_attri.getBitShare(i);
            tem2 = tem2 + (1ULL << i) * bit2A(b_r, a_r, b_s, pIdx);
        }
        auto result = reveal(tem2);
        if (pIdx == 0) {
            LOG(INFO) << "converted value in arithematic is : " << result;
        }

        su64 delta = tem - tem2;
        auto delta_reveal = reveal(delta);
        if (pIdx == 0) {
            LOG(INFO) << "converted delta value in arithematic is : " << delta_reveal;
        }
        
        
        // given su64 value s, compute msb(s)
        // prepare su64 r
        // compute m = x + r, open m
        // compute msb(s) = msb(m) \xor msb(2^{\ell} - r) \xor {m' + r' > 2^{\ell -1} -1} 
        // where m' = m mod 2^{\ell -1}, r' = (2^{\ell} - r) mod 2^{\ell -1}

        u64 s;
        su64 ss_s;
        if (pIdx == 0) {// 1 = -4+2+3
            s = -4;
            ss_s = getShareU64Data(pIdx, s);
        }else if(pIdx == 1){
            s = 2;
            ss_s = getShareU64Data(pIdx, s);
        }else{
            s = 3;
            ss_s = getShareU64Data(pIdx, s);
        }

        // offline
        // share r in su64
        u64 r;//define rand1 = 2
        su64 ss_r;
        if (pIdx == 0) {// 2 = -4+1+5
            r = -4;
            ss_r = getShareU64Data(pIdx, r);
        }else if(pIdx == 1){
            r = 1;
            ss_r = getShareU64Data(pIdx, r);
        }else{
            r = 5;
            ss_r = getShareU64Data(pIdx, r);
        }

        // offline
        // share msb(2^ell - r) in sb64
        u64 msb_neg_rand = ((1ULL << 64) - 2 > (1ULL << 63)) ? 1 : 0;
        sb64 ss_msb_neg_rand;
        if (pIdx == 0) {// 2 = -4+1+5
            ss_msb_neg_rand = getShareBinData(pIdx, msb_neg_rand);
        }else if(pIdx == 1){
            ss_msb_neg_rand = getShareBinData(pIdx, u64(1));
        }else{
            ss_msb_neg_rand = getShareBinData(pIdx, u64(1));
        }

        // open the difference m in sb64
        su64 m = ss_s + ss_r;
        auto m_reveal = reveal(m);
        if (pIdx == 0) {
            LOG(INFO) << "opened m is : " << m_reveal;
        }

        // decompose m' = m mod 2^(\ell -1) and share each bit in su8
        u64 m_prime = m_reveal % (1ULL << 63);
        u8Matrix m_prime_bits(MODEL_BITS, 1);
        for (u64 i = 0; i < MODEL_BITS; ++i) {
            m_prime_bits(i, 0) = static_cast<u8>((m_prime >> i) & 1ULL);
        }

        // offline
        // decompose and share 2^(\ell -1) - r' -1 where r' = (2^{\ell} - r) mod 2^{\ell -1}, each bit share in su8
        u64 r_prime = ((1ULL << 64) - r) % (1ULL << 63);
        u64 r_prime_complement = (1ULL << 63) - r_prime - 1;
        u8Matrix r_prime_complement_bits(MODEL_BITS, 1);
        for (u64 i = 0; i < MODEL_BITS; ++i) {
            r_prime_complement_bits(i, 0) = static_cast<u8>((r_prime_complement >> i) & 1ULL);
        }
        // 现在是假设p0分发2^(\ell -1) - r' -1 
        // TODO 改为三方共享计算
        su8Matrix ss_r_prime_complement_bits = shareValueSchema(pIdx, r_prime_complement_bits, model);

        // local compute msb(m) in sb64
        u64 msb_m = (m_reveal > (1ULL << 63)) ? 1 : 0;
        sb64 ss_msb_m;
        if (pIdx == 0) {
            ss_msb_m[0] = msb_m;
            ss_msb_m[1] = 0;
        }else if(pIdx == 1){
            ss_msb_m[0] = 0;
            ss_msb_m[1] = msb_m;
        }else{
            ss_msb_m[0] = 0;
            ss_msb_m[1] = 0;
        }

        // compute the thrid term: {m' + r' > 2^{\ell -1} -1}
        su64 compare_result;
        su64 maced_compare_result;
        comparison_DGK(model, ss_r_prime_complement_bits, m_prime_bits, value_depth, pIdx,
                        compare_result, maced_compare_result, ss_mac_key_for_2to3);

        //check reshare
        su64 mac_multi_compare_result;
        mac_multi_compare_result = rss_u64_mul(ss_mac_key_for_2to3, compare_result);


        su64 ret = mac_multi_compare_result - maced_compare_result;
        auto revealed = reveal(compare_result);
        if (pIdx == 0) {
            LOG(INFO) << "Comparison result revealed: \n" << revealed;
        }

        





        // //newly added
        // /*
        // * test steps, to see how to share a 64*1 matrix, each element is 8 bits. 
        // * x < y
        // */

        // //  假设party0 准备 value x
        // u8Matrix x;
        // if (pIdx == 0) {
        //     x.resize(MODEL_BITS, 1);
        //     for (u64 i = 0; i < x.size(); ++i) x(i) = 0;
        // }
        // su8Matrix ssValueScheme = shareValueSchema(pIdx, x, model);

        // // prepare a plaintext value y
        // u8Matrix y;
        // y.resize(MODEL_BITS, 1);
        // for (u64 i = 0; i < y.size(); ++i) y(i) = 1;

        // su64 compare_result;
        // su64 maced_compare_result;
        // comparison_DGK(model, ssValueScheme, y, value_depth, pIdx,
        //                 compare_result, maced_compare_result, ss_mac_key_for_2to3);

        // //check reshare
        // su64 mac_multi_compare_result;
        // mac_multi_compare_result = rss_u64_mul(ss_mac_key_for_2to3, compare_result);


        // su64 ret = mac_multi_compare_result - maced_compare_result;
        // auto revealed = reveal(ret);
        // if (pIdx == 0) {
        //     LOG(INFO) << "Comparison result revealed: \n" << revealed;
        // }
        // //end

        for (int i = 0; i < depth; ++i) {
            if (pIdx == 0)
                LOG(INFO) << "****** Travel Decision Tree Layer: [ " << i << " ] ******";
            sb64 ssSelectedIdx = cur_node.getVIdx();
            // LOG(INFO) << "P" << pIdx << " selected feature V index: " << reveal(ssSelectedIdx);
            sb64 ssY = semi_ob_selection_feature(pIdx, ssFeatureSchema, ssSelectedIdx, model, false);
            
            // LOG(INFO) << "P" << pIdx << " selected feature V index: " << reveal(ssSelectedIdx) << " -> X: " << reveal(ssY);

            sb64 threshold = cur_node.getThreshold();
            sb64 left = cur_node.getLeft();
            sb64 right = cur_node.getRight();

            // LOG(INFO) << "P" << pIdx << " right: " << right << " reveal " << reveal(right);
            // LOG(INFO) << "P" << pIdx << " left: " << left << " reveal " << reveal(left);
             
            /*
            0: right child;  1: left child
            */
            // LOG(INFO) << "P" << pIdx << " ssY: " << ssY;
            // LOG(INFO) << "P" << pIdx << " threshold: " << threshold;
            // LOG(INFO) << "P" << pIdx << " compare: (" << reveal(ssY) << ", " << reveal(threshold) << ")";
            sb64 ssb = compare_threshold(pIdx, ssY, threshold, as, bs, cs, i);
            // LOG(INFO) << "P" << pIdx << " compare_result: " << ssb << " -> X: " << reveal(ssb);
            sb64 ssNextNodeIdx = getChildNodeIndex(pIdx, ssb, left, right, a[i], b[i], c[i]);
            // LOG(INFO) << "P" << pIdx << " ssb: " << ssb << " -> expect NextNodeIdx: " << ssNextNodeIdx << " reveal " << reveal(ssNextNodeIdx);

            //selection of model
            sbMatrix next_node = semi_ob_selection_node(pIdx, ss_model_data, ssNextNodeIdx, model, false);
            i64Matrix plain_ssnode = reveal(next_node);
            
            // LOG(INFO) << "P" << pIdx << " real selected node [" << reveal(ssNextNodeIdx) << "] : " << plain_ssnode;
            
            //selection of maced model
            sbMatrix next_mac_node = semi_ob_selection_mac_node(pIdx, ss_mac_model_data, ssNextNodeIdx, model, false);
            i64Matrix plain_mac_ssnode = reveal(next_mac_node);
            
            // LOG(INFO) << "P" << pIdx << " real selected mac node [" << reveal(ssNextNodeIdx) << "] : " << plain_mac_ssnode;
            

            // i64Matrix plain_ssnode = reveal(next_node);
            cur_node.setSSData(next_node);
            cur_mac_node.setSSMACData(next_mac_node);
            // LOG(INFO) << "P" << pIdx << " real selected node [" << reveal(ssNextNodeIdx) << "] : " << plain_ssnode;

            //sum selected model node
            // sum_model_data = sum_model_data ^ cur_node.getThreshold() ^ cur_node.getLeft() ^ cur_node.getRight() ^ cur_node.getVIdx() ^ cur_node.getLabel();
            sum_model_data = sum_model_data ^ poly_mul(rho[i], cur_node.getThreshold() ^ cur_node.getLeft() ^ cur_node.getRight() ^ cur_node.getVIdx() ^ cur_node.getLabel());

            //sum selected mac model node
            sum_mac_model_data = sum_mac_model_data ^ poly_mul(rho[i], cur_mac_node.getMACThreshold() ^ cur_mac_node.getMACLeft() ^ cur_mac_node.getMACRight() ^ cur_mac_node.getMACVIdx() ^ cur_mac_node.getMACLabel());
        
        }
        //mac checking
        sum_model_data = sum_model_data ^ gamma;
        sum_mac_model_data = poly_mul(ss_mac_key, gamma) ^ sum_mac_model_data;
        int rec = mac_check(pIdx, sum_model_data, sum_mac_model_data, ss_mac_key);
        // LOG(INFO) << "mac checking result is " << rec;

        if (rec == 0){
            LOG(INFO) << "Batch mac checking passed! ";
            return cur_node.getLabel();
        }else{
            //should terminate the protocol in real application
            LOG(INFO) << "Batch mac checking not passed! ";
            return cur_node.getLabel();   
        }
        
    }

    /***
     *   Tests
     */
    void SS3Dtree::test_negate_ss(int pIdx, i64 &a) {
        i64 ret;
        sb64 ssa, ssret;
        if (pIdx == 0) {
            LOG(INFO) << "a : " << std::hex << a;
            ssa = localBinary(a);
            ssret = negate_ss(pIdx, ssa);
            ret = reveal(ssret);
            LOG(INFO) << "ssa : " << ssa;
            LOG(INFO) << "~ssa : " << ssret;
            LOG(INFO) << "~a : " << std::hex << ret << std::dec;
        } else {
            ssa = remoteBinary();
            ssret = negate_ss(pIdx, ssa);
            ret = reveal(ssret);
        }
        LOG(INFO) << "ret : " << ret;
    }

    void SS3Dtree::test_compare_ss(int pIdx) {
        i64 a = 6001, b = 6000;
        sb64 ssa, ssb, ssc;
        i64 ret;
        if (pIdx == 0) {
            ssa = localBinary(a);
            ssb = localBinary(b);
            ssc = compare_ss(pIdx, ssa, ssb);
            ret = reveal(ssc);

            LOG(INFO) << "ssa : " << ssa;
            LOG(INFO) << "ssb : " << ssb;
            LOG(INFO) << "ssc: " << ssc;
        } else {
            ssa = remoteBinary();
            ssb = remoteBinary();
            ssc = compare_ss(pIdx, ssa, ssb);
            ret = reveal(ssc);
        }
        LOG(INFO) << "ret : " << ret;
    }

    void SS3Dtree::test_and_ss(int pIdx) {
        i64 a = 7, b = 6;
        sb64 ssa, ssb, ssc;
        i64 ret;
        if (pIdx == 0) {
            ssa = localBinary(a);
            ssb = localBinary(b);
            ssc = and_ss(ssa, ssb);
            ret = reveal(ssc);

            LOG(INFO) << "ssa : " << ssa;
            LOG(INFO) << "ssb : " << ssb;
            LOG(INFO) << "ssc: " << ssc;
        } else {
            ssa = remoteBinary();
            ssb = remoteBinary();
            ssc = and_ss(ssa, ssb);
            ret = reveal(ssc);
        }

        LOG(INFO) << "ret : " << ret;
    }

    void SS3Dtree::test_mul_poly_ss(int pIdx) {
        // test mul
        i64 a = 4, b = 6;
        sb64 ssa, ssb, ssc;
        i64 ret;
        if (pIdx == 0) {
            ssa = localBinary(a);
            ssb = localBinary(b);
            ssc = poly_mul(ssa, ssb);
            ret = reveal(ssc);

            LOG(INFO) << "ssa : " << ssa;
            LOG(INFO) << "ssb : " << ssb;
            LOG(INFO) << "ssa[0] poly : " << ssa.to_poly_string(0);
            LOG(INFO) << "ssb[0] poly : " << ssb.to_poly_string(0);
            LOG(INFO) << "poly a: " << I642POLY(a);
            LOG(INFO) << "poly b: " << I642POLY(b);
            LOG(INFO) << "poly a*b: " << I642POLY(ret);
        } else {
            ssa = remoteBinary();
            ssb = remoteBinary();
            ssc = poly_mul(ssa, ssb);
            ret = reveal(ssc);
        }

        LOG(INFO) << "ret : " << ret;
    }

    std::array<BitVector, 2> SS3Dtree::compare_plain(BitVector &gt, BitVector &eq, int size) {
        LOG(INFO) << "compare_bit compare_plain: " << size;
        std::array<BitVector, 2> ret;
        ret[0].resize(size / 2, 0);
        ret[1].resize(size / 2, 0);
        if ((eq.size() != size) && (eq.size() == gt.size()))
            throw std::logic_error("Bitvector size error");

        for (int i = 0; i < size / 2; ++i) {
            int offset = i * 2;
            ret[0][i] = gt[offset + 1] ^ (eq[offset + 1] & gt[offset]);
            ret[1][i] = eq[offset] & eq[offset + 1];
        }
        LOG(INFO) << "in ret 0: " << ret[0];
        LOG(INFO) << "in ret 1: " << ret[1];
        if (size == 2)
            return ret;
        return compare_plain(ret[0], ret[1], ret[0].size());
    }

    void SS3Dtree::test_plain_compare() {
        int BIT_SZ = 4;
        u8 x = 0x07;
        u8 y = 0x06;
        BitVector a((u8 *) &x, BIT_SZ);
        BitVector b((u8 *) &y, BIT_SZ);
        LOG(INFO) << "a: " << a;
        LOG(INFO) << "b: " << b;
        LOG(INFO) << "~b: " << ~b;
        BitVector gt = a & ~b;
        BitVector eq = ~(a ^ b);
        LOG(INFO) << "a&~b gt: " << gt;
        LOG(INFO) << "a^b eq: " << eq;
        std::array<BitVector, 2> ret = compare_plain(gt, eq, gt.size());
        LOG(INFO) << "qt ret: " << ret[0];
        LOG(INFO) << "eq ret: " << ret[1];
    }

    //add by jianli
    // void SS3Dtree::sendDPFKeysFromParty0(uint32_t len, uint32_t rdx)
    // {
    //     if (partyIdx() != 0) return;

    //     auto blobs = DPFGen(len, rdx, makeBlock(0, 0x1));

    //     std::cout<< "DPF Key Size to Party1: " << blobs.toParty1.bytes.size() << std::endl;
    //     std::cout<< "DPF Key Size to Party2: " << blobs.toParty2.bytes.size() << std::endl;

    //     // to party1 (next)
    //     uint64_t n1 = (uint64_t)blobs.toParty1.bytes.size();
    //     mNext.send(n1);
    //     if (n1) mNext.send(blobs.toParty1.bytes);

    //     // to party2 (prev)
    //     uint64_t n2 = (uint64_t)blobs.toParty2.bytes.size();
    //     mPrev.send(n2);
    //     if (n2) mPrev.send(blobs.toParty2.bytes);
    // }

    DPFKeyBlob SS3Dtree::recvDPFKeyFromParty0()
    {
        DPFKeyBlob blob;
        uint64_t n = 0;

        if (partyIdx() == 1) {
            mPrev.recv(n);
            blob.bytes.resize((size_t)n);
            if (n) mPrev.recv(blob.bytes.data(), n);   // POD recv
        }
        else if (partyIdx() == 2) {
            mNext.recv(n);
            blob.bytes.resize((size_t)n);
            if (n) mNext.recv(blob.bytes.data(), n);   // POD recv
        }
        else {
            throw std::runtime_error("party0 should not call recvDPFKeyFromParty0()");
        }

        return blob;
    }


    void SS3Dtree::sendDPFKeysFromParty0(uint32_t len, uint32_t rdx)
    {
        if (partyIdx() != 0) return;

        u8 rdx_pre = rand() % MODEL_BITS;
        u8 rdx_next = rdx - rdx_pre % MODEL_BITS;

        std::cout << "[DPF] rdx: " << rdx << ", rdx_pre: " << (uint32_t)rdx_pre << ", rdx_next: " << (uint32_t)rdx_next << std::endl;

        // 先广播 alpha/rdx
        mNext.send(rdx_next);
        mPrev.send(rdx_pre);

        auto blobs = DPFGen(len, rdx, makeBlock(0, 0x1));

        #ifdef DEBUG_PRINT_DPFKEY
        //for debugging purpose
        auto out = DPFGenWithKeys(len, rdx, makeBlock(0, 0x1));
        debug_print_dpfkey(*out.keyToParty1, "party1 before send");
        debug_print_dpfkey(*out.keyToParty2, "party2 before send");
        auto &blobs = out.blobs;
        #endif


        // to party1 (next)
        uint64_t n1 = (uint64_t)blobs.toParty1.bytes.size();
        mNext.send(n1);
        if (n1) mNext.send(blobs.toParty1.bytes.data(), n1);   // ✅ POD send

        // to party2 (prev)
        uint64_t n2 = (uint64_t)blobs.toParty2.bytes.size();
        mPrev.send(n2);
        if (n2) mPrev.send(blobs.toParty2.bytes.data(), n2);   // ✅ POD send
    }


    std::pair<uint32_t, int> SS3Dtree::recvAndEvalDPFfromParty0(uint32_t len, aby3::Ref<aby3::su8> value_share)
    {
        if (partyIdx() != 1 && partyIdx() != 2)
            throw std::runtime_error("only party1/2 should call recvAndEvalDPFfromParty0()");

        u8 rdx = 0;
        if (partyIdx() == 1) mPrev.recv(rdx);
        else mNext.recv(rdx);
        std::cout << "[DPF] party " << partyIdx() << " rdx received: " << (uint32_t)rdx << std::endl;


        u8 my_to_peer, peer_to_me;
        u8 masked_value;


        DPFKeyBlob blob;
        uint64_t n = 0;
        if (partyIdx() == 1) {
            mPrev.recv(n);
            blob.bytes.resize((size_t)n);
            if (n) mPrev.recv(blob.bytes.data(), n);
            my_to_peer = (u8)(value_share[1] + rdx);
            mNext.send(my_to_peer);
            mNext.recv(peer_to_me);
        } else {
            mNext.recv(n);
            blob.bytes.resize((size_t)n);
            if (n) mNext.recv(blob.bytes.data(), n);
            my_to_peer = (u8)(value_share[0] + rdx);
            mPrev.send(my_to_peer);
            mPrev.recv(peer_to_me);
        }
        std::cout << "[DPF] party " << partyIdx() << " my_to_peer: " << (uint32_t)my_to_peer << ", peer_to_me: " << (uint32_t)peer_to_me << std::endl;
        masked_value = (u8)(value_share[0] + value_share[1] + peer_to_me + rdx);

        std::cout<< "[DPF] party " << partyIdx() << " masked_value: " << (uint32_t)masked_value << std::endl;

        uint32_t evalId01 = (partyIdx() == 1) ? 0 : 1;

        DPFKEY key(len);
        deserialization((char*)blob.bytes.data(), &key);

        #ifdef DEBUG_PRINT_DPFKEY
        //for debugging purpose
        debug_print_dpfkey(key, "party1 after recv");
        #endif

        emp::block hash_out[2];
        int share = dpf_eval_int(evalId01, &key, (uint32_t)masked_value, hash_out);

        return { rdx, share };
    }

    // only for small depth, in our work, it is 6
    // party0生成64组DPF keys并且一次性发送，每组key额外将mac的一部分编入
    void SS3Dtree::sendDPFKeyBatchFromParty0(uint32_t depth, su64 ss_mac_key_for_2to3)
    {
        if (partyIdx() != 0) return;

        const uint32_t N = 1u << depth;
        const uint32_t MOD = 1u << depth;   // split in Z_{2^depth}

        std::vector<uint8_t> buf1, buf2;
        std::vector<uint32_t> offsets;

        std::vector<u8> rdx_to_p1;
        std::vector<u8> rdx_to_p2;

        // ===== party0 cache =====
        dpfDepth = depth;
        dpfN = N;
        dpfRdxPreBatch.assign(N, 0);
        dpfRdxNextBatch.assign(N, 0);

        for (uint32_t i = 0; i < N; ++i) {
            uint32_t rdx = rand() % MOD;              // rdx in [0, 2^depth)
            u8 rdx_pre   = (u8)(rand() % MOD);        // pre in same ring
            u8 rdx_next  = (u8)((rdx + MOD - rdx_pre) % MOD);

            // cache on party0
            dpfRdxPreBatch[i]  = rdx_pre;
            dpfRdxNextBatch[i] = rdx_next;

            // send shares
            rdx_to_p1.push_back(rdx_next); // to party1
            rdx_to_p2.push_back(rdx_pre);  // to party2

            //DPF generation without MAC key
            // auto blobs = DPFGen(depth, rdx, makeBlock(0, 1));

            //DPF generation with MAC key
            u64 tem = u64(ss_mac_key_for_2to3[0] + (u64)ss_mac_key_for_2to3[1] );
            auto blobs = DPFGenWithMAC(depth, rdx, makeBlock(0, 1), makeBlock(0, tem));

            offsets.push_back((uint32_t)buf1.size());

            buf1.insert(buf1.end(),
                        blobs.toParty1.bytes.begin(),
                        blobs.toParty1.bytes.end());

            buf2.insert(buf2.end(),
                        blobs.toParty2.bytes.begin(),
                        blobs.toParty2.bytes.end());
        }

        // ===== meta =====
        mNext.send(depth); mNext.send(N);
        mPrev.send(depth); mPrev.send(N);

        // ===== offsets =====
        uint64_t offsz = (uint64_t)offsets.size() * sizeof(uint32_t);
        mNext.send(offsz); mNext.send(offsets.data(), offsz);
        mPrev.send(offsz); mPrev.send(offsets.data(), offsz);

        // ===== rdx shares ===== (still using byte-size field like your current code)
        uint64_t rdxsz = (uint64_t)rdx_to_p1.size() * sizeof(u8);
        mNext.send(rdxsz); mNext.send(rdx_to_p1.data(), rdxsz);
        mPrev.send(rdxsz); mPrev.send(rdx_to_p2.data(), rdxsz);

        // ===== DPF blobs =====
        uint64_t n1 = (uint64_t)buf1.size();
        mNext.send(n1); mNext.send(buf1.data(), n1);

        uint64_t n2 = (uint64_t)buf2.size();
        mPrev.send(n2); mPrev.send(buf2.data(), n2);
    }




    void SS3Dtree::recvDPFKeyBatch()
    {
        if (partyIdx() != 1 && partyIdx() != 2)
            throw std::runtime_error("only party1/2 should call recvDPFKeyBatch()");

        uint32_t depth = 0, N = 0;

        // ===== meta =====
        if (partyIdx() == 1) { mPrev.recv(depth); mPrev.recv(N); }
        else                 { mNext.recv(depth); mNext.recv(N); }

        // 基本 sanity（避免读错包导致后面 resize 爆炸）
        if (depth == 0 || depth > 20) throw std::runtime_error("bad depth in recvDPFKeyBatch()");
        if (N != (1u << depth))       throw std::runtime_error("bad N in recvDPFKeyBatch()");

        // ===== offsets =====
        uint64_t offsz = 0;
        std::vector<uint32_t> offsets;

        if (partyIdx() == 1) mPrev.recv(offsz);
        else                 mNext.recv(offsz);

        if (offsz != (uint64_t)N * sizeof(uint32_t))
            throw std::runtime_error("bad offsz in recvDPFKeyBatch()");

        offsets.resize(N);

        if (partyIdx() == 1) mPrev.recv(offsets.data(), offsz);
        else                 mNext.recv(offsets.data(), offsz);

        // ===== rdx shares =====
        uint64_t rdxsz = 0;
        std::vector<u8> rdxShares;

        if (partyIdx() == 1) mPrev.recv(rdxsz);
        else                 mNext.recv(rdxsz);

        if (rdxsz != (uint64_t)N * sizeof(u8))
            throw std::runtime_error("bad rdxsz in recvDPFKeyBatch()");

        rdxShares.resize(N);

        if (partyIdx() == 1) mPrev.recv(rdxShares.data(), rdxsz);
        else                 mNext.recv(rdxShares.data(), rdxsz);

        // ===== blob =====
        uint64_t n = 0;
        std::vector<uint8_t> buf;

        if (partyIdx() == 1) mPrev.recv(n);
        else                 mNext.recv(n);

        buf.resize((size_t)n);

        if (n) {
            if (partyIdx() == 1) mPrev.recv(buf.data(), n);
            else                 mNext.recv(buf.data(), n);
        }

        // ===== extra sanity：offset 必须落在 buf 范围内 =====
        for (uint32_t i = 0; i < N; ++i) {
            if (offsets[i] >= buf.size())
                throw std::runtime_error("offset out of range in recvDPFKeyBatch()");
        }

        // ===== cache =====
        dpfDepth       = depth;
        dpfN           = N;
        dpfKeyOffsets  = std::move(offsets);
        dpfRdxShares   = std::move(rdxShares);
        dpfKeyBatchBuf = std::move(buf);

        std::cout << "[BATCH-DBG] recv ok: depth=" << dpfDepth
                << " N=" << dpfN
                << " offsets=" << dpfKeyOffsets.size()
                << " rdxShares=" << dpfRdxShares.size()
                << " buf=" << dpfKeyBatchBuf.size()
                << std::endl;
    }


    // 输入：alphas[] + ssDGKSchema
    // 输出：my_to_peer[]
    void SS3Dtree::prepareMaskedRdxBatch(
    aby3::su8Matrix& ssDGKSchema,
    std::vector<u8>& my_to_peer)
    {
        if (dpfRdxShares.size() != dpfN)
            throw std::runtime_error("prepareMaskedRdxBatch: dpfRdxShares not ready");

        // ★ 关键 1：resize
        my_to_peer.resize(dpfN);

        // ★ 关键 2：用 dpfN，不是 MODEL_BITS
        for (size_t i = 0; i < dpfN; ++i) {
            u8 rdx = dpfRdxShares[i];

            if (partyIdx() == 1)
                //cause ssDGKSchema is 64 * 1
                my_to_peer[i] = (u8)(ssDGKSchema(i,0)[1] + rdx);
            else
                my_to_peer[i] = (u8)(ssDGKSchema(i,0)[0] + rdx);
        }
    }


    // p0 reveals masked rdx to p1 and p2
    void SS3Dtree::sendMaskedRdxBatch (aby3::su8Matrix&ssDGKSchema){
        if (partyIdx() != 0) return;

        uint64_t sz = dpfRdxPreBatch.size();
        if (dpfRdxPreBatch.size() != dpfRdxNextBatch.size())
        throw std::runtime_error("sendMaskedRdxBatch: size mismatch");
        std::vector<u8> masked_rdx_to_next; // send to p1
        std::vector<u8> masked_rdx_to_pre; // send to p2

        masked_rdx_to_next.resize(dpfN);
        masked_rdx_to_pre.resize(dpfN);
        
        for (size_t i = 0; i < dpfN; ++i) {
            masked_rdx_to_next[i] = (u8)(ssDGKSchema(i,0)[0] + dpfRdxNextBatch[i]);
            masked_rdx_to_pre[i] = (u8)(ssDGKSchema(i,0)[1] + dpfRdxPreBatch[i]);
        }
        
        // to party1 
        mNext.send(sz);
        mNext.send(masked_rdx_to_pre.data(), sz);

        // to party2
        mPrev.send(sz);
        mPrev.send(masked_rdx_to_next.data(), sz);
    }

    // p1 and p2 receive masked values from p0
    void SS3Dtree::recvMaskedRdxBatch()
    {
        if (partyIdx() == 0)
            throw std::runtime_error("party0 should not call recvMaskedRevealBatch()");

        uint64_t sz = 0;

        if (partyIdx() == 1) {
            mPrev.recv(sz);
        } else { // partyIdx == 2
            mNext.recv(sz);
        }

        // ===== 安全检查 =====
        if (sz > (1ull << 20))
            throw std::runtime_error("recvMaskedRdxBatch: sz too large");

        if (sz) {
            if (partyIdx() == 1){
                dpfRdxNextBatch.resize((size_t)sz);
                mPrev.recv(dpfRdxNextBatch.data(), sz);
            }
                
            else{
                dpfRdxPreBatch.resize((size_t)sz);
                mNext.recv(dpfRdxPreBatch.data(), sz);
            }
                
        }
    }


    void SS3Dtree::maskedRdxCheck(std::vector<u8>& peer_to_me){
        if (partyIdx() == 1) {
            for (size_t i = 0; i < dpfN; ++i) {
                if ( dpfRdxNextBatch[i] != peer_to_me[i]) {
                    throw std::runtime_error("party1 maskedRdxCheck failed at index " + std::to_string(i));
                }
            }
        } else if (partyIdx() == 2) {
            for (size_t i = 0; i < dpfN; ++i) {
                if ( dpfRdxPreBatch[i] != peer_to_me[i]) {
                    throw std::runtime_error("party2 maskedRdxCheck failed at index " + std::to_string(i));
                }
            }
        }
        LOG(INFO) << "Party " << partyIdx() << " masked rdx check passed ";
    };




    void SS3Dtree::exchangeMaskedRdxBatch(
        std::vector<u8>& my_to_peer,
        std::vector<u8>& peer_to_me)
    {
        peer_to_me.resize(my_to_peer.size());

        uint64_t sz = my_to_peer.size() * sizeof(u8);

        if (partyIdx() == 1) {
            // P1: send -> recv
            mNext.send(sz);
            mNext.send(my_to_peer.data(), sz);

            mNext.recv(sz);
            mNext.recv(peer_to_me.data(), sz);
        } else {
            // P2: recv -> send（避免死锁）
            mPrev.recv(sz);
            mPrev.recv(peer_to_me.data(), sz);

            mPrev.send(sz);
            mPrev.send(my_to_peer.data(), sz);
        }
    }

    //single output DPF
    // int SS3Dtree::evalDPFBatch(
    //     size_t idx,
    //     const std::vector<u8>& peer_to_me,
    //     aby3::su8Matrix& ssDGKSchema)
    // {
    //     u8 masked_value =
    //         (u8)(ssDGKSchema(idx,0)[0]
    //         + ssDGKSchema(idx,0)[1]
    //         + peer_to_me[idx]
    //         + dpfRdxShares[idx]);

    //     // extract key
    //     uint32_t off = dpfKeyOffsets[idx];
    //     DPFKEY key(dpfDepth);
    //     deserialization((char*)(dpfKeyBatchBuf.data() + off), &key);

    //     uint32_t evalId01 = (partyIdx() == 1) ? 0 : 1;
    //     emp::block hash_out[2];

    //     return dpf_eval_int_two(evalId01, &key, (uint32_t)masked_value, hash_out);
    // }

    //double output DPF
    DpfEvalInt2 SS3Dtree::evalDPFBatchWithMAC(
        size_t idx,
        const std::vector<u8>& peer_to_me,
        aby3::su8Matrix& ssDGKSchema)
    {
        u8 masked_value =
            (u8)(ssDGKSchema(idx,0)[0]
            + ssDGKSchema(idx,0)[1]
            + peer_to_me[idx]
            + dpfRdxShares[idx]);

        // extract key
        uint32_t off = dpfKeyOffsets[idx];
        DPFKEY key(dpfDepth);
        deserialization((char*)(dpfKeyBatchBuf.data() + off), &key);

        uint32_t evalId01 = (partyIdx() == 1) ? 0 : 1;
        emp::block hash_out[2];

        return dpf_eval_int_two(evalId01, &key, (uint32_t)masked_value, hash_out);
    }









    //debug only
    void SS3Dtree::sendDPFShareToParty0(int share)
    {
        if (partyIdx() == 1) {
            // party1 -> party0 via prev
            mPrev.send(share);
        }
        else if (partyIdx() == 2) {
            // party2 -> party0 via next
            mNext.send(share);
        }
        else {
            throw std::runtime_error("party0 should not call sendDPFShareToParty0()");
        }
    }

    //debug only
    void SS3Dtree::recvAndCheckDPFShares()
    {
        if (partyIdx() != 0)
            throw std::runtime_error("only party0 should call recvAndCheckDPFShares()");

        int share1 = 0, share2 = 0;

        // from party1
        mNext.recv(share1);

        // from party2
        mPrev.recv(share2);

        uint32_t s1 = (uint32_t)share1;
        uint32_t s2 = (uint32_t)share2;
        uint32_t sum = s1 + s2;

        std::cout << "[DPF DEBUG]\n";
        std::cout << "  share1 (int)  = " << share1 << "\n";
        std::cout << "  share2 (int)  = " << share2 << "\n";
        std::cout << "  share1 (u32)  = " << s1 << "\n";
        std::cout << "  share2 (u32)  = " << s2 << "\n";
        std::cout << "  sum mod 2^32  = " << sum << "\n";
    }


    void debug_print_dpfkey(DPFKEY& key, const std::string& tag)
    {
        std::cout << "=== DPFKEY DEBUG (" << tag << ") ===\n";
        std::cout << "depth = " << key.get_depth() << "\n";
        std::cout << "rdx_share = " << key.get_share() << "\n";

        auto s = key.get_s();
        std::cout << "s.low64  = " << _mm_extract_epi64(s, 0) << "\n";
        std::cout << "s.high64 = " << _mm_extract_epi64(s, 1) << "\n";
        std::cout << "t        = " << key.get_t() << "\n";

        for (uint32_t i = 1; i <= std::min(2u, key.get_depth()); i++) {
        auto scw = key.get_s_CW(i);
        std::cout << "level " << i << " s_CW.low64  = "
                    << _mm_extract_epi64(scw, 0) << "\n";
        std::cout << "level " << i << " s_CW.high64 = "
                    << _mm_extract_epi64(scw, 1) << "\n";
        std::cout << "level " << i << " t_L=" << key.get_t_L_CW(i)
                    << " t_R=" << key.get_t_R_CW(i) << "\n";
        }
    }



}

