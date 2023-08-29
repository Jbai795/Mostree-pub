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
        mEnc.init(partyIdx, c, prng.get<block>());
        mEval.init(partyIdx, c, prng.get<block>());
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

    const sb64 SS3Dtree::getShareMACKeyBinData(int partIdx, i64 MAC_KEY) {
        sb64 ret;
        if (partIdx == 0) {
            ret = localBinary(MAC_KEY);
        } else {
            ret = remoteZeroBinary();
        }
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

        for (int i = 0; i < depth; ++i) {
            // if (pIdx == 0)
            //     LOG(INFO) << "****** Travel Decision Tree Layer: [ " << i << " ] ******";
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
}

