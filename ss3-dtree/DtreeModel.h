#pragma once

#include <vector>

#include <Eigen/Dense>
#include <aby3/sh3/Sh3Types.h>
#include <aby3/sh3/Sh3FixedPoint.h>
#include "aby3/Common/logging.h"
#include "common/tree_read_from_file.h"
#include "common/fss_read.h"

using namespace CW::LOGINTER;
using namespace aby3;
using namespace std;


namespace mostree {
    //template<typename T>
    //using eMatrix = eMatrix<T>;
    static const u64 MODEL_BITS = 64; // support max 64 bits

    enum class NODE_POS : unsigned long {
        NIDX = 0,   // node idx
        THR = 1,    // threshold
        LNODE = 2,  // left node idx
        RNODE = 3,  // right node idx
        VIDX = 4,   // feature idx
        LABEL = 5   // classification
    };

    struct Node {
        const uint8_t data_len = 6;
    private:
        uint64_t idx;
        uint64_t threshold;
        uint64_t left;
        uint64_t right;
        uint64_t v_idx;
        uint64_t label;
        std::vector<uint64_t> _data;

    public:
        Node() {
            idx = 0;
            threshold = 0;
            left = 0;
            right = 0;
            v_idx = 0;
            label = 0;
            _data.resize(data_len);
        };

        Node(uint64_t idx, uint64_t threshold, uint64_t left, uint64_t right, uint64_t vIdx, uint64_t label) :
                idx(idx),
                threshold(threshold),
                left(left),
                right(right),
                v_idx(vIdx),
                label(label) {
            _data.push_back(idx);
            _data.push_back(threshold);
            _data.push_back(left);
            _data.push_back(right);
            _data.push_back(vIdx);
            _data.push_back(label);
        }

        Node(std::vector<uint64_t> &tmp) {
            if (tmp.size() != data_len)
                throw std::logic_error("Node data length error");
            _data = tmp;
            idx = _data[uint8_t(NODE_POS::NIDX)];
            threshold = _data[uint8_t(NODE_POS::THR)];
            left  = _data[uint8_t(NODE_POS::LNODE)];
            right = _data[uint8_t(NODE_POS::RNODE)];
            v_idx = _data[uint8_t(NODE_POS::VIDX)];
            label = _data[uint8_t(NODE_POS::LABEL)];
        };

        Node &operator=(const Node &copy) {
            _data = copy._data;
            return *this;
        }

        Node(const Node &) = default;

        virtual ~Node() = default;


        uint64_t getThreshold() const {
            return threshold;
        }

        void setThreshold(uint64_t thr) {
            threshold = thr;
        }

        uint64_t getLeft() const {
            return left;
        }

        void setLeft(uint64_t l) {
            left = l;
        }

        uint64_t getRight() const {
            return right;
        }

        void setRight(uint64_t r) {
            right = r;
        }

        uint64_t getVIdx() const {
            return v_idx;
        }

        void setVIdx(uint64_t vIdx) {
            v_idx = vIdx;
        }

        uint64_t getLabel() const {
            return label;
        }

        void setLabel(uint64_t lab) {
            label = lab;
        }
    };

    template<Decimal D>
    struct SSFNode {
        const uint8_t data_len = 6;
    private:
        sf64<D> idx;
        sf64<D> threshold;
        sf64<D> left;
        sf64<D> right;
        sf64<D> v_idx;
        sf64<D> label;
    public:
//        std::vector<sf64<D>> ssdata;
        sf64Matrix<D> ssNodeData;

        const sf64<D> &getIdx() {
            return ssNodeData((u64) (NODE_POS::NIDX));
        }

        const sf64<D> &getThreshold() {
            return ssNodeData((u64) (NODE_POS::THR));
        }

        const sf64<D> &getLeft() {
            return ssNodeData((u64) (NODE_POS::LNODE));
        }

        const sf64<D> &getRight() {
            return ssNodeData((u64) (NODE_POS::RNODE));
        }

        const sf64<D> &getVIdx() {
            return ssNodeData((u64) (NODE_POS::VIDX));
        }

        const sf64<D> &getLabel() {
            return ssNodeData((u64) (NODE_POS::LABEL));
        }

        const sf64Matrix<D> &getSSData() const {
            return ssNodeData;
        }

        void setSSData(const sf64Matrix<D> &data) {
            ssNodeData = data;
        }

        SSFNode() {
            idx = {};
            threshold = {};
            left = {};
            right = {};
            v_idx = {};
            label = {};
            ssNodeData.resize(1, data_len);
        }
    };

    struct SSNode {
    private:
        sb64 idx;
        sb64 threshold;
        sb64 left;
        sb64 right;
        sb64 v_idx;
        sb64 label;
    public:
//        std::vector<sf64<D>> ssdata;
        sbMatrix ssNodeData;

        const i64Matrix to_matrix(int shared_idx) {
            i64Matrix ret(1, cols());
            std::copy_n(ssNodeData.mShares[shared_idx].data(), cols(), ret.data());
            return ret;
        }

        static const i64Matrix to_matrix(int shared_idx, sbMatrix &in) {
            i64Matrix ret(1, in.i64Cols());
            std::copy_n(in.mShares[shared_idx].data(0), in.i64Cols(), ret.data());
            return ret;
        }

        sb64 getIdx() {
            sb64 ret = ssNodeData(0, (u64) (NODE_POS::NIDX));
            return ret;
        }

        sb64 getThreshold() {
            sb64 ret = ssNodeData(0, (u64) (NODE_POS::THR));
            return ret;
        }

        sb64 getLeft() {
            sb64 ret = ssNodeData(0, (u64) (NODE_POS::LNODE));
            return ret;
        }

        sb64 getRight() {
            sb64 ret = ssNodeData(0, (u64) (NODE_POS::RNODE));
            return ret;
        }

        sb64 getVIdx() {
            sb64 ret = ssNodeData(0, (u64) (NODE_POS::VIDX));
            return ret;
        }

        sb64 getLabel() {
            sb64 ret = ssNodeData(0, (u64) (NODE_POS::LABEL));
            return ret;
        }

        const sbMatrix &getSSData() const {
            return ssNodeData;
        }

        void setSSData(const sbMatrix &data) {
            ssNodeData = data;
        }

        u64 cols() const {
            return ssNodeData.i64Cols();
        }

        u64 size() const {
            return ssNodeData.i64Size();
        }
        SSNode() = default;
        SSNode(u64 bits) {
            idx = {};
            threshold = {};
            left = {};
            right = {};
            v_idx = {};
            label = {};
            ssNodeData.resize(1, bits);
        }

        SSNode(sbMatrix &data) : ssNodeData(data) {
        }
        SSNode(const SSNode &) = default;

        SSNode(SSNode &&) = default;
    };


    struct SSMACNode {
    private:
        sb64 idx;
        sb64 threshold;
        sb64 left;
        sb64 right;
        sb64 v_idx;
        sb64 label;
    public:

        sbMatrix ssMACNodeData;

        const i64Matrix to_matrix(int shared_idx) {
            i64Matrix ret(1, cols());
            std::copy_n(ssMACNodeData.mShares[shared_idx].data(), cols(), ret.data());
            return ret;
        }

        static const i64Matrix to_matrix(int shared_idx, sbMatrix &in) {
            i64Matrix ret(1, in.i64Cols());
            std::copy_n(in.mShares[shared_idx].data(0), in.i64Cols(), ret.data());
            return ret;
        }

        sb64 getMACIdx() {
            sb64 ret = ssMACNodeData(0, (u64) (NODE_POS::NIDX));
            return ret;
        }

        sb64 getMACThreshold() {
            sb64 ret = ssMACNodeData(0, (u64) (NODE_POS::THR));
            return ret;
        }

        sb64 getMACLeft() {
            sb64 ret = ssMACNodeData(0, (u64) (NODE_POS::LNODE));
            return ret;
        }

        sb64 getMACRight() {
            sb64 ret = ssMACNodeData(0, (u64) (NODE_POS::RNODE));
            return ret;
        }

        sb64 getMACVIdx() {
            sb64 ret = ssMACNodeData(0, (u64) (NODE_POS::VIDX));
            return ret;
        }

        sb64 getMACLabel() {
            sb64 ret = ssMACNodeData(0, (u64) (NODE_POS::LABEL));
            return ret;
        }

        const sbMatrix &getSSMACData() const {
            return ssMACNodeData;
        }

        void setSSMACData(const sbMatrix &data) {
            ssMACNodeData = data;
        }
        //

        u64 cols() const {
            return ssMACNodeData.i64Cols();
        }

        u64 size() const {
            return ssMACNodeData.i64Size();
        }
        SSMACNode() = default;
        SSMACNode(u64 bits) {
            idx = {};
            threshold = {};
            left = {};
            right = {};
            v_idx = {};
            label = {};
            ssMACNodeData.resize(1, bits);
        }

        SSMACNode(sbMatrix &data) : ssMACNodeData(data) {
        }
        SSMACNode(const SSMACNode &) = default;

        SSMACNode(SSMACNode &&) = default;
    };

    /*  r < gen_num and r in [F2^k] */
    struct FssGen {
        FssGen() = default;

        FssGen(u64 num) : gen_num(num) {
            k.resize(num, 1);
        };

        FssGen(const FssGen &) = default;

        virtual ~FssGen() = default;

        sb64 r;
        sbMatrix k;
        u64 gen_num;

        void setGenNum(u64 num) {
            gen_num = num;
            k.resize(num, 1);
            k.mShares[0].setZero();
            k.mShares[1].setZero();
        }

        // void setGenNum(u64 num) {
        //     gen_num = num;
        //     k.resize(num, 1);
        //     k.mShares[0].setZero();
        //     k.mShares[1].setZero();
        // }

        const sb64 &getR() const {
            return r;
        }

        void setR(const sb64 &rr) {
            r = rr;
        }

        const sbMatrix &getK() const {
            return k;
        }

        void setK(const sbMatrix &kk) {
            k = kk;
        }

    };


    class DtreeModel {
        static const Decimal D = D0;
        static const u64 L_BITS = MODEL_BITS;

    public:
        DtreeModel(u64 fn = 8, u64 nn = 16) : _feature_num(fn), _nodes_num(nn) {
            _featureSchema.resize(fn, 1);
            ssFeatureSchema.resize(fn, L_BITS);
            mFssGenFeature.setGenNum(fn);

            _nodesSchema.resize(nn, 1);
            ssNodesSchema.resize(nn, L_BITS);
            mFssGenNodes.setGenNum(nn);

            selectedY = 0;
        }

//        DtreeModel(eMatrix<double> &model)
//                : plainModel(model) { setPlainModel(model); }

        DtreeModel(i64Matrix &model)
                : plainModel(model) { setPlainModel(model); }

        virtual ~DtreeModel() = default;

        static const Decimal getD() { return D; };
        void load_model_test();
        void load_model(DecTree* tree, int nodesMax);
        // void load_model(DecTree* tree, int nodesMax, int x);
        void setPlainModel(const eMatrix<double> &model);
        void setPlainModel(const i64Matrix &data);
        void setPlainMACModel(const i64Matrix &data);

        void load_mac_model(DecTree* tree, int nodesMax, int x);
        
        u64 rows() const { return plainModel.rows(); }

        u64 nodes() const { return plainModel.rows(); }

        u64 cols() const { return plainModel.cols(); }
        u64 bits() const { return ssModel.bitCount();}

        u64 size() const { return plainModel.size(); }

        u64 FN() const { return _feature_num; }
        u64 NN() const { return _nodes_num; }

//        const eMatrix<double> &getPlainModel() const {
//            return plainDataModel;
//        }

        const i64Matrix &getPlainModel() const {
            return plainModel;
        }


        const i64Matrix &getPlainMACModel() const {
            return plainMACModel;
        }

        const Node getNode(uint64_t idx) {
            auto size = plainFpModel.mData.row(idx).size();
            std::vector<uint64_t> tmp(size);
            std::copy_n(plainFpModel.i64Cast().row(idx).data(), size, tmp.data());
            return Node(tmp);
        }

        const Node getPlainNode(uint64_t idx) {
            auto size = plainModel.cols();
            std::vector<uint64_t> tmp(size);
            std::copy_n(plainModel.data() + idx*size, size, tmp.data());
            return Node(tmp);
        }

        const SSNode getSSNode(uint64_t nodeIdx) {
            SSNode ssNode(bits());
            std::copy_n(ssModel.mShares[0].data(nodeIdx), cols(), ssNode.ssNodeData.mShares[0].data());
            std::copy_n(ssModel.mShares[1].data(nodeIdx), cols(), ssNode.ssNodeData.mShares[1].data());
            return ssNode;
        }

        const SSFNode<D> getSSFNode(uint64_t nodeIdx) {
            SSFNode<D> ssNode;
            ssNode.ssNodeData.i64Cast().mShares[0] = ssData.i64Cast().mShares[0].row(nodeIdx);
            ssNode.ssNodeData.i64Cast().mShares[1] = ssData.i64Cast().mShares[1].row(nodeIdx);
            return ssNode;
        }

        const f64Matrix<D> &getPlainFpModel() const {
            return plainFpModel;
        }

        const sf64Matrix<D> &getSSData() const {
            return ssData;
        }

        void setSSData(const sf64Matrix<D> &data) {
            ssData = data;
        }

        void setSSModel(const sbMatrix &data) {
            ssModel = data;
        }

        void setSSMACModel(const sbMatrix &data) {
            ssMACModel = data;
        }

        void sample(); // not useful

        const i64Matrix &getFeatureSchema() const {
            return _featureSchema;
        }

        void initFeatureSchema(); // sample

        void load_feature(u64 FEATURE_NUM);

        void setSSFeatureSchema(const sbMatrix &data) {
            ssFeatureSchema = data;
        }


        const sbMatrix &getSSFeatureSchema() const {
            return ssFeatureSchema;
        }

        void setSSNodesSchema(const sbMatrix &data) {
            ssNodesSchema = data;
        }


        const sbMatrix &getSSNodesSchema() const {
            return ssNodesSchema;
        }

        const sb64 &getSSSelectedIdx() const {
            return ssSelectedIdx;
        }

        void setSSSelectedIdx(const sb64 &idx) {
            ssSelectedIdx = idx;
        }

        void setSelectedIdx(const i64 &idx) {
            _selectedIdx = idx;
        }

        static int realIdx(int pIdx) {
            return int(pIdx + 2) % 3;
        }
        //only test
        void fssPreprocess(int pIdx);
        //real
        void realTreeFssPreprocess(int pIdx);
        void realFeatureFssPreprocess(int pIdx);

    public:
        sf64Matrix<D> ssData; // not useful
        sbMatrix ssModel;
        sbMatrix ssFeatureSchema;
        sbMatrix ssNodesSchema;
        sb64 ssSelectedIdx;
        FssGen mFssGenFeature;
        FssGen mFssGenNodes;
        sb64 ssDelta;
        i64 selectedY;

        sbMatrix ssMACModel;
    private:
        i64Matrix plainModel;
        f64Matrix<D> plainFpModel; // not useful
        i64Matrix _featureSchema;
        i64Matrix _nodesSchema;
        u64 _feature_num;
        u64 _nodes_num;
        i64 _selectedIdx;

        i64Matrix plainMACModel;

    };
}
