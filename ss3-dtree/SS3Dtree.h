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
#include "dpf/dpf.h"

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

        const sb64 getShareMACKeyBinData(int partIdx, i64 val);
        //add by jianli
        const sb64 getShareBinData(int partIdx, u64 val);
        const sb8 getShareBinData(int partIdx, u8 val);
        const su64 getShareU64Data(int partIdx, u64 val);
        
        const sb64 getShareRandBinData();

        const sb64 getCovertP2NBinary(sb64 &sour);
        const sb64 getCovertN2PBinary(sb64 &sour);

        const sbMatrix shareFeatureSchema(int partIdx, DtreeModel &model);

        //add by jianli
        const su8Matrix shareValueSchema(int partIdx, u8Matrix plain, DtreeModel &model);
        const su8Matrix ssDGKFun(int partIdx, DtreeModel &model, u8Matrix &plain);


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

        su64 rss_u64_mul(const su64& left, const su64& right)
        {
            su64 dest;
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

        void comparison_DGK(DtreeModel &model, su8Matrix ssValueScheme, u8Matrix y, int value_depth, int pIdx,
                    su64 &compare_result, su64 &maced_compare_result, su64 ss_mac_key_for_2to3);
        sb8 bin_xor(sb8 &a, sb8 &b);
        su64 bit2A(sb8 b_r, su64 a_r, sb8 b_x, int pIdx);

        sb64 travelTree(int pIdx, int depth, SSNode &root, DtreeModel &model, sbMatrix &ss_model_data,sbMatrix &ssFeatureSchema);

        sb64 travelTree(int pIdx, int depth, SSNode &root, DtreeModel &model, sbMatrix &ss_model_data, sbMatrix &ss_mac_model_data,
                              sbMatrix &ssFeatureSchema, sb64 &ss_mac_key, sb64 gamma, sb64 * rho,sb64 *a,sb64 *b,sb64 *c, sb64 *as,sb64 *bs,sb64 *cs);
        
        
        
        //add by jianli
        // DPF key distribution (party0 -> party1,party2)
        void sendDPFKeysFromParty0(uint32_t len, uint32_t rdx);
        DPFKeyBlob recvDPFKeyFromParty0();  // party1/2 用
        std::pair<uint32_t, int> recvAndEvalDPFfromParty0(uint32_t len, aby3::Ref<aby3::su8> value_share);

        //batch DPF keys generation and evaluation
        //only used for small depth
        void sendDPFKeyBatchFromParty0(uint32_t depth, su64 ss_mac_key_for_2to3);
        void recvDPFKeyBatch();
        // int evalDPFBatch(uint32_t alpha, uint32_t x);

        void prepareMaskedRdxBatch(aby3::su8Matrix& ssDGKSchema, std::vector<u8>& my_to_peer);
        // p1 and p2 receive masked rdx batch from each other
        void exchangeMaskedRdxBatch(std::vector<u8>& my_to_peer, std::vector<u8>& peer_to_me);
        // int evalDPFBatch(size_t idx, const std::vector<u8>& peer_to_me, aby3::su8Matrix& ssDGKSchema);
        DpfEvalInt2 evalDPFBatchWithMAC(size_t idx, const std::vector<u8>& peer_to_me, aby3::su8Matrix& ssDGKSchema);

        //p0 sends masked rdx batch to p1 and p2
        void sendMaskedRdxBatch (aby3::su8Matrix&ssDGKSchema);
        //p1 and p2 receive masked rdx batch from p0
        void recvMaskedRdxBatch();
        void maskedRdxCheck(std::vector<u8>& peer_to_me);
        

        //临时
        // debug only: send local DPF share back to party0
        void sendDPFShareToParty0(int share);

        // debug only: party0 recv both shares and print sum
        void recvAndCheckDPFShares();
        // debug only
        void debug_print_dpfkey(DPFKEY& key, const std::string& tag);
    private:
        // party0 only (cache for debugging / later use)
        std::vector<u8> dpfRdxPreBatch;   // size N, sent to party2
        std::vector<u8> dpfRdxNextBatch;  // size N, sent to party1
        // === DPF batch cache ===
        std::vector<uint8_t>  dpfKeyBatchBuf;  // 拼接后的所有 DPF key 字节
        std::vector<uint32_t> dpfKeyOffsets;   // offsets[alpha] = 起始字节位置
        uint32_t              dpfDepth = 0;    // 当前 batch 的 depth
        std::vector<u8>       dpfRdxShares;     // rdxShares[alpha] = 本 party 的 rdx share (u8)
        uint32_t              dpfN = 0;         // = 1<<depth

    public:
        uint32_t getDPFN() const {
            return dpfN;
        }
        vector<u8> getDPFRdxPreBatch() const {
            return dpfRdxPreBatch;
        }
        vector<u8> getDPFRdxNextBatch() const {
            return dpfRdxNextBatch;
        }



    public:
        sb64 localBinary(i64 val) {
            sb64 dest;
            mEnc.localBinary(mRt, val, dest).get();
            return dest;
        }

        sb64 localBinary(u64 val) {
            sb64 dest;
            mEnc.localBinary(mRt, val, dest).get();
            return dest;
        }


        sb8 localBinary(u8 val) {
            sb8 dest;
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


        //add by jianli
        su64 localU64(u64 val) {
            su64 dest;
            mEnc.localU64(mRt, val, dest).get();
            return dest;
        }

        su64 remoteU64() {
            su64 dest;
            mEnc.remoteU64(mRt, dest).get();
            return dest;
        }

        // su64 remoteZeroU64() {
        //     su64 dest;
        //     mEnc.remoteZeroU64(mRt, dest).get();
        //     return dest;
        // }

        //end

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

        // add by jianli
        // generates an arithmatic sharing of the matrix m and places the result in dest
        void localU8Matrix(const u8Matrix &m, su8Matrix &dest) {
            mEnc.localU8Matrix(mRt, m, dest).get();
        }

        // add by jianli
        // generates an arithmatic sharing of the matrix input by the remote party and places the result in dest
        void remoteU8Matrix(su8Matrix &dest) {
            mEnc.remoteU8Matrix(mRt, dest).get();
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


        // add by jianli
        //reveal u8 binary value
        u8 reveal(const sb8 &val) {
            u8 dest;
            mEnc.revealAll(mRt.noDependencies(), val, dest).get();
            return static_cast<u8>(dest);
        }

        // add by jianli
        u64 reveal(const su64 &val) {
            u64 dest;
            mEnc.revealAll(mRt.noDependencies(), val, dest).get();
            return static_cast<u64>(dest);
        }
        //end

        i64Matrix reveal(const sbMatrix &val) {
            i64Matrix dest(val.rows(), val.i64Cols());
            mEnc.revealAll(mRt.noDependencies(), val, dest).get();
            return dest;
        }

        u8Matrix reveal(const su8Matrix &val) {
            u8Matrix dest(val.rows(), val.cols());
            mEnc.revealAllU8Value(mRt.noDependencies(), val, dest).get();
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
