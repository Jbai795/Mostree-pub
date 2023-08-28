#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Common/BitVector.h>
#include <cryptoTools/Crypto/PRNG.h>
#include <cryptoTools/Common/CLP.h>
#include <cryptoTools/Common/Log.h>
#include <aby3/Common/Defines.h>
#include <aby3/sh3/Sh3FixedPoint.h>
#include <aby3/sh3/Sh3Encryptor.h>
#include <aby3/sh3/Sh3Evaluator.h>
#include <aby3/sh3/Sh3Piecewise.h>
#include "aby3/sh3/Sh3FixedPoint.h"
#include "aby3/Common/NTLHelper.h"

#include "DtreeModel.h"

using namespace std;
using namespace aby3;
using namespace oc;

namespace mostree {
    class SS3Dtree {
    public:
        SS3Dtree(int polyDegree = 64) : _poly_degree(polyDegree) {
            GF2X p;
            // cout << p << "\n";
            BuildIrred(p, _poly_degree);
            GF2E::init(p);
            // cout << p << "\n";
            // LOG(INFO) << "Poly degree: " << GF2E::degree() << '\n';
        }

    public:
        oc::Channel mPreproNext, mPreproPrev, mNext, mPrev;
        Sh3Encryptor mEnc;
        Sh3Evaluator mEval;
        Sh3Runtime mRt;
        bool mPrint = true;

        u64 partyIdx() {
            return mRt.mPartyIdx;
        }

        void init(u64 partyIdx, oc::Session &prev, oc::Session &next, oc::block seed);

        const sbMatrix getShareModelBinData(int partIdx, DtreeModel &model);

        const sbMatrix getShareMACModelBinData(int partIdx, DtreeModel &model);

        const sbMatrix getShareMACModelBinData(int partIdx, sbMatrix model_data, sbMatrix & dest_data, sb64 key);

        const sb64 getShareMACKeyBinData(int partIdx, i64 MAC_KEY);
        const sb64 getShareRandBinData();

        const sb64 getCovertP2NBinary(sb64 &sour);
        const sb64 getCovertN2PBinary(sb64 &sour);

        const sbMatrix shareFeatureSchema(int partIdx, DtreeModel &model);

        const sb64 shareSelectedIndex(int partIdx, DtreeModel &model, i64 &idx);

        void fssPreprocess(int pIdx, DtreeModel &model);
        void realTreeFssPreprocess(int pIdx, DtreeModel &model);
        void realFeatureFssPreprocess(int pIdx, DtreeModel &model);

        sb64 compare_fake(int pIdx, sb64 &x, sb64 &t);
        sb64 compare_ss(int pIdx, sb64 &a, sb64 &b);
        sb64 compare_ss(int pIdx, sb64 &a, sb64 &b,sb64 *as, sb64 *bs, sb64 *cs, int val);


        sb64 and_ss(sb64 &a, sb64 &b);

        void check_and_ss(int pIdx, sb64 x, sb64 y, sb64 z, sb64 as, sb64 bs, sb64 cs);

        sb64 negate_ss(int pIdx, sb64 &x);

        sb64 poly_mul(const sb64 &left, const sb64 &right) {
            sb64 dest;
            mEval.asyncMul(mRt.noDependencies(), left, right, dest).get();
            return dest;
        }

        sb64 compare_threshold(int pIdx, sb64 &x, sb64 &t);

        sb64 compare_threshold(int pIdx, sb64 &x, sb64 &t, sb64 *as, sb64 *bs, sb64 *cs, int val);

        sb64 getChildNodeIndex(sb64 &ssb, sb64 &left, sb64 &right);

        sb64 getChildNodeIndex(int pIdx, sb64 &ssb, sb64 &left, sb64 &right,sb64 a,sb64 b,sb64 c);

        // Semi-honest oblivious selection feature protocol
        sb64 semi_ob_selection_feature(int partIdx, sbMatrix &feature, sb64 selected_idx, DtreeModel &model,
                                       bool isFss = false);

        // Semi-honest oblivious selection node protocol
        sbMatrix semi_ob_selection_node(int partIdx, sbMatrix &model_table, sb64 selected_node_idx, DtreeModel &model,
                                        bool isFss = false);

        sbMatrix semi_ob_selection_mac_node(int partIdx, sbMatrix &model_table, sb64 selected_node_idx, DtreeModel &model,
                                        bool isFss = false);

        sb64 travelTree(int pIdx, int depth, SSNode &root, DtreeModel &model, sbMatrix &ss_model_data,sbMatrix &ssFeatureSchema);

        sb64 travelTree(int pIdx, int depth, SSNode &root, DtreeModel &model, sbMatrix &ss_model_data, sbMatrix &ss_mac_model_data,
                              sbMatrix &ssFeatureSchema, sb64 &ss_mac_key, sb64 gamma, sb64 * rho,sb64 *a,sb64 *b,sb64 *c, sb64 *as,sb64 *bs,sb64 *cs);

    public:
        sb64 localBinary(i64 val) {
            sb64 dest;
            mEnc.localBinary(mRt, val, dest).get();
            return dest;
        }

        sb64 remoteBinary() {
            sb64 dest;
            mEnc.remoteBinary(mRt, dest).get();
            return dest;
        }

        sb64 remoteZeroBinary() {
            sb64 dest;
            mEnc.remoteZeroBinary(mRt, dest).get();
            return dest;
        }

        void CovertP2NBinary(sb64 &sour, sb64 &dest) {
            mEnc.CovertP2NBinary(mRt, sour, dest).get();
        }

        void CovertN2PBinary(sb64 &sour, sb64 &dest) {
            mEnc.CovertN2PBinary(mRt, sour, dest).get();
        }

        // generates a binary sharing of the matrix m and places the result in dest
        void localBinMatrix(const i64Matrix &m, sbMatrix &dest) {
            mEnc.localBinMatrix(mRt, m, dest).get();
        }

        // generates a binary sharing of the matrix input by the remote party and places the result in dest
        void remoteBinMatrix(sbMatrix &dest) {
            mEnc.remoteBinMatrix(mRt, dest).get();
        }

        template<Decimal D>
        sf64Matrix<D> localInput(const f64Matrix<D> &val) {
            std::array<u64, 2> size{val.rows(), val.cols()};
            mNext.asyncSendCopy(size);
            mPrev.asyncSendCopy(size);
            sf64Matrix<D> dest(size[0], size[1]);
            mEnc.localFixedMatrix(mRt, val, dest).get();

            return dest;
        }

        template<Decimal D>
        sf64Matrix<D> localInput(const eMatrix<double> &vals) {
            f64Matrix<D> v2(vals.rows(), vals.cols());
            for (u64 i = 0; i < vals.size(); ++i)
                v2(i) = vals(i);

            return localInput(v2);
        }

        template<Decimal D>
        sf64Matrix<D> remoteInput(u64 partyIdx) {
            std::array<u64, 2> size;
            if (partyIdx == (mRt.mPartyIdx + 1) % 3)
                mNext.recv(size);
            else if (partyIdx == (mRt.mPartyIdx + 2) % 3)
                mPrev.recv(size);
            else
                throw RTE_LOC;

            sf64Matrix<D> dest(size[0], size[1]);
            mEnc.remoteFixedMatrix(mRt, dest).get();
            return dest;
        }

        template<Decimal D>
        const sf64Matrix<D> getShareModelData(int partIdx, DtreeModel &model) {
            sf64Matrix<D> ss_model_data;
            if (partIdx == 0) {
                ss_model_data = localInput<D>(model.getPlainModel());
            } else {
                ss_model_data = remoteInput<D>(0);
            }

            model.setSSData(ss_model_data);

            return ss_model_data;
        }


        void preprocess(u64 n, Decimal d) {
            // TODO("implement this");
        }

        // add by muou
        i64 reveal(const sb64 &val) {
            i64 dest;
            mEnc.revealAll(mRt.noDependencies(), val, dest).get();
            return static_cast<i64>(dest);
        }

        i64Matrix reveal(const sbMatrix &val) {
            i64Matrix dest(val.rows(), val.i64Cols());
            mEnc.revealAll(mRt.noDependencies(), val, dest).get();
            return dest;
        }

        template<Decimal D>
        eMatrix<double> reveal(const sf64Matrix<D> &vals) {
            f64Matrix<D> temp(vals.rows(), vals.cols());
            mEnc.revealAll(mRt.noDependencies(), vals, temp).get();

            eMatrix<double> ret(vals.rows(), vals.cols());
            for (u64 i = 0; i < ret.size(); ++i)
                ret(i) = static_cast<double>(temp(i));
            return ret;
        }

        template<Decimal D>
        double reveal(const sf64<D> &val) {
            f64<D> dest;
            mEnc.revealAll(mRt.noDependencies(), val, dest).get();
            return static_cast<double>(dest);
        }

        template<Decimal D>
        double reveal(const Ref<sf64<D>> &val) {
            sf64<D> v2(val);
            return reveal(v2);
        }

        template<Decimal D>
        sf64Matrix<D> mul(const sf64Matrix<D> &left, const sf64Matrix<D> &right) {
            sf64Matrix<D> dest;
            mEval.asyncMul(mRt.noDependencies(), left, right, dest).get();
            return dest;
        }

        template<Decimal D>
        sf64Matrix<D> mulTruncate(const sf64Matrix<D> &left, const sf64Matrix<D> &right, u64 shift) {
            sf64Matrix<D> dest;
            mEval.asyncMul(mRt.noDependencies(), left, right, dest, shift).get();
            return dest;
        }

        template<typename T>
        SS3Dtree &operator<<(const T &v) {
            if (partyIdx() == 0 && mPrint) std::cout << v;
            return *this;
        }

        template<typename T>
        SS3Dtree &operator<<(T &v) {
            if (partyIdx() == 0 && mPrint) std::cout << v;
            return *this;
        }

        SS3Dtree &operator<<(std::ostream &(*v)(std::ostream &)) {
            if (partyIdx() == 0 && mPrint) std::cout << v;
            return *this;
        }

        SS3Dtree &operator<<(std::ios &(*v)(std::ios &)) {
            if (partyIdx() == 0 && mPrint) std::cout << v;
            return *this;
        }

        SS3Dtree &operator<<(std::ios_base &(*v)(std::ios_base &)) {
            if (partyIdx() == 0 && mPrint) std::cout << v;
            return *this;
        }

    private:
        std::array<sb64, 2> compare_process(sb64 &gt, sb64 &eq, int size);

        std::array<sb64, 2> compare_process(int partIdx, sb64 &gt, sb64 &eq, int size);

        std::array<sb64, 2> compare_process(int partIdx, sb64 &gt, sb64 &eq, int size, sb64 *as, sb64 *bs, sb64 *cs, int val, int incr);


        sb64 semi_ob_selection_y(int partIdx, sb64 &ssSelectedIdx, sbMatrix &X, FssGen &fssgen);

        sbMatrix semi_ob_selection_matrix(int partIdx, sb64 &ssSelectedIdx, sbMatrix &X, FssGen &fssgen);

        int mac_check(int pIdx, sb64 &x, sb64 &y, sb64 &key);

    private:
        int _poly_degree;

    // Tests
    public:
        void test_negate_ss(int pIdx, i64 &a);

        void test_compare_ss(int pIdx);

        void test_and_ss(int pIdx);

        void test_mul_poly_ss(int pIdx);

        std::array<BitVector, 2> compare_plain(BitVector &gt, BitVector &eq, int size);

        void test_plain_compare();
    };


}
