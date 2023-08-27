#include "DtreeModel.h"
#include <random>
string fssp0k1 = "../../../../ss3-dtree/fss-keys/p0k1.txt";
string fssp0k2 = "../../../../ss3-dtree/fss-keys/p0k2.txt";
string fssp1k0 = "../../../../ss3-dtree/fss-keys/p1k0.txt";
string fssp1k2 = "../../../../ss3-dtree/fss-keys/p1k2.txt";
string fssp2k0 = "../../../../ss3-dtree/fss-keys/p2k0.txt";
string fssp2k1 = "../../../../ss3-dtree/fss-keys/p2k1.txt";
using namespace std;
namespace mostree {


    void DtreeModel::setPlainModel(const eMatrix<double> &data) {
//        plainModel = data;
//        plainFpModel.resize(data.rows(), data.cols());
//        for (u64 i = 0; i < data.size(); ++i)
//            plainFpModel(i) = data(i);
    }

    void DtreeModel::setPlainModel(const i64Matrix &data) {
        plainModel = data;
        // std::cout << "data.rows(): " << data.rows() << "\n";
        // std::cout << "data.cols(): " << data.cols() << "\n";
        ssModel.resize(data.rows(), data.cols()*L_BITS);
    }

    void DtreeModel::setPlainMACModel(const i64Matrix &data) {
        // plainMACModel = data;
        // std::cout << "data.rows(): " << data.rows() << "\n";
        // std::cout << "data.cols(): " << data.cols() << "\n";
        ssMACModel.resize(data.rows(), data.cols()*L_BITS);
    }

    void DtreeModel::initFeatureSchema() {
        for (int i = 0; i < _featureSchema.size(); ++i) {
            _featureSchema(i) = 10+i;
        }
    }
    void DtreeModel::load_feature(u64 feature) {
        for (int i = 0; i < _featureSchema.size(); ++i) {
            _featureSchema(i) = 3150+i;
        }
    }

    /*
     * Only Test
     * TODO
     */
    void DtreeModel::load_model_test() {
        u64 test_model[] = {
                0, 11 , 1, 2, 2,    0,
                1, 999, 1, 1, 0,    111,
                2, 12,  3, 4, 1,    0,
                3, 999, 3, 3, 0,  333,
                4, 999, 4, 4, 0,  444,
                5, 999, 5, 5, 0,  555,
                6, 999, 6, 6, 0,  666,
                7, 999, 7, 7, 0,  777,
        };
        i64Matrix model(8, 6);
        std::copy_n(test_model, sizeof(test_model)/sizeof(u64), model.data());
        setPlainModel(model);
    }

    /*
     * Load real tree(the tree is padded as a complete tree)
     * Use vector
     */
    void DtreeModel::load_model(DecTree* tree, int nodesMax) {
        int totalNum = tree->num_dec_nodes + tree->num_of_leaves;
        //if the total nodes number is 2^(k-1)<n<2^k, we pad the tree to be an array with size of nearest 2^k, which saves significant storage
        // int nodesMax = pow(2, ceil(log(totalNum)/log(2)));
        int n = nodesMax*6;
        //u64 *test_model = new u64[n];
        // padded:wine:192
        vector <u64> test_model;
        for(int i = 0; i < totalNum*6; i=i+6){
            test_model.push_back(i/6);

            test_model.push_back(tree->thres[i/6]);

            test_model.push_back(tree->left[i/6]);

            test_model.push_back(tree->right[i/6]);

            test_model.push_back(tree->map[i/6]);

            test_model.push_back(tree->label[i/6]);
        }
        //pad 0
        for (int i = totalNum*6; i < n; i=i+6){
            test_model.push_back(i/6);

            test_model.push_back(0);   

            test_model.push_back(0);

            test_model.push_back(0);

            test_model.push_back(0);

            test_model.push_back(0);
        }

        i64Matrix model(nodesMax, 6);
        std::copy_n(test_model.data(), test_model.size(), model.data());
        setPlainModel(model);
        setPlainMACModel(model);
    }

    // /*
    //  * Load real tree(the tree is padded as a complete tree)
    //  * Use vector
    //  */
    // void DtreeModel::load_model(DecTree* tree, int nodesMax, int x) {
    //     int totalNum = tree->num_dec_nodes + tree->num_of_leaves;
    //     //if the total nodes number is 2^(k-1)<n<2^k, we pad the tree to be an array with size of nearest 2^k, which saves significant storage
    //     // int nodesMax = pow(2, ceil(log(totalNum)/log(2)));
    //     int n = nodesMax*6;
    //     //u64 *test_model = new u64[n];
    //     // padded:wine:192
    //     GF2E alpha = I642POLY(x);
    //     GF2E temp;
    //     vector <u64> test_model;
    //     vector <i64> mac_model;
    //     for(int i = 0; i < totalNum*6; i=i+6){
    //         test_model.push_back(i/6);
    //         temp = I642POLY(i/6);
    //         mac_model.push_back(POLY2I64(temp));

    //         test_model.push_back(tree->thres[i/6]);
    //         temp = alpha*I642POLY(tree->thres[i/6]);
    //         mac_model.push_back(POLY2I64(temp));

    //         test_model.push_back(tree->left[i/6]);
    //         temp = alpha*I642POLY(tree->left[i/6]);
    //         mac_model.push_back(POLY2I64(temp));

    //         test_model.push_back(tree->right[i/6]);
    //         temp = alpha*I642POLY(tree->right[i/6]);
    //         mac_model.push_back(POLY2I64(temp));

    //         test_model.push_back(tree->map[i/6]);
    //         temp = alpha*I642POLY(tree->map[i/6]);
    //         mac_model.push_back(POLY2I64(temp));

    //         test_model.push_back(tree->label[i/6]);
    //         temp = alpha*I642POLY(tree->label[i/6]);
    //         mac_model.push_back(POLY2I64(temp));
    //     }
    //     //pad 0
    //     for (int i = totalNum*6; i < n; i=i+6){
    //         test_model.push_back(i/6);
    //         temp = I642POLY(i/6);
    //         mac_model.push_back(POLY2I64(temp));

    //         test_model.push_back(0);
    //         mac_model.push_back(0);

    //         test_model.push_back(0);
    //         mac_model.push_back(0);

    //         test_model.push_back(0);
    //         mac_model.push_back(0);

    //         test_model.push_back(0);
    //         mac_model.push_back(0);

    //         test_model.push_back(0);
    //         mac_model.push_back(0);
    //     }

    //     i64Matrix model(nodesMax, 6);
    //     std::copy_n(test_model.data(), test_model.size(), model.data());
    //     setPlainModel(model);
    //     i64Matrix macmodel(nodesMax, 6);
    //     std::copy_n(mac_model.data(), mac_model.size(), macmodel.data());
    //     setPlainMACModel(macmodel);
    // }



    void DtreeModel::sample() {
        //TODO
    }

    /*
     * Only Test
     * TODO
     */
    void DtreeModel::fssPreprocess(int pIdx) {
        //  r < gen_num
        int real_idx = realIdx(pIdx);
        if (real_idx == 0) {
            i64 r02 = 4;
            i64 r01 = 4;
            mFssGenFeature.r = {{r02, r01}};
            std::vector<u64> k0{0,1,1,0,1,0,1,1};
            std::vector<u64> k1{1,0,1,1,0,1,1,0};
            if ( mFssGenFeature.k.rows() != k0.size() || (mFssGenFeature.k.rows() != _feature_num))
                throw std::invalid_argument("P0 mFssGenFeature.k number invalid!");
            std::copy_n(k0.data(), k0.size(), mFssGenFeature.k.mShares[0].data());
            std::copy_n(k1.data(), k1.size(), mFssGenFeature.k.mShares[1].data());

        } else if (real_idx == 1) {
            i64 r10 = 1;
            i64 r12 = 7;
            mFssGenFeature.r = {{r10, r12}};
            std::vector<u64> k0{1,1,0,1,0,0,0,1};
            std::vector<u64> k1{0,1,1,1,1,0,1,1};
            if ( mFssGenFeature.k.rows() != k0.size() || (mFssGenFeature.k.rows() != _feature_num))
                throw std::invalid_argument("P1 mFssGenFeature.k number invalid!");
            std::copy_n(k0.data(), k0.size(), mFssGenFeature.k.mShares[0].data());
            std::copy_n(k1.data(), k1.size(), mFssGenFeature.k.mShares[1].data());

        } else if (real_idx == 2) {
            i64 r21 = 6;
            i64 r20 = 5;
            mFssGenFeature.r = {{r21, r20}};
            std::vector<u64> k0{1,0,0,1,0,1,1,0};
            std::vector<u64> k1{1,1,0,1,1,0,0,1};
            if ( mFssGenFeature.k.rows() != k0.size() || (mFssGenFeature.k.rows() != _feature_num))
                throw std::invalid_argument("P2 mFssGenFeature.k number invalid!");
            std::copy_n(k0.data(), k0.size(), mFssGenFeature.k.mShares[0].data());
            std::copy_n(k1.data(), k1.size(), mFssGenFeature.k.mShares[1].data());
        } else
            throw std::logic_error("Invalid pIdx");

    }

    void DtreeModel::realFeatureFssPreprocess(int pIdx) {
        //  r < gen_num
        int real_idx = realIdx(pIdx);
        if (real_idx == 0) {
            i64 r02 = 4;
            i64 r01 = 4;
            mFssGenFeature.r = {{r02, r01}};
            std::vector<u64> k0;
            fss_read(fssp0k1,k0,FN());
            std::vector<u64> k1;
            fss_read(fssp0k2,k1,FN());
            LOG(INFO) << "mFssGenFeature.k.rows(): \n" << mFssGenFeature.k.rows();
            LOG(INFO) << "k0.size(): \n" << k0.size();
            LOG(INFO) << "_feature_num: \n" << _feature_num;
            if ( mFssGenFeature.k.rows() != k0.size()|| (mFssGenFeature.k.rows() != FN()))
                throw std::invalid_argument("P0 mFssGenFeature.k number invalid!");
            std::copy_n(k0.data(), k0.size(), mFssGenFeature.k.mShares[0].data());
            std::copy_n(k1.data(), k1.size(), mFssGenFeature.k.mShares[1].data());

        } else if (real_idx == 1) {
            i64 r10 = 1;
            i64 r12 = 7;
            mFssGenFeature.r = {{r10, r12}};
            std::vector<u64> k0;
            fss_read(fssp1k0,k0,FN());
            std::vector<u64> k1;
            fss_read(fssp1k2,k1,FN());
            if ( mFssGenFeature.k.rows() != k0.size() || (mFssGenFeature.k.rows() != FN()))
                throw std::invalid_argument("P1 mFssGenFeature.k number invalid!");
            std::copy_n(k0.data(), k0.size(), mFssGenFeature.k.mShares[0].data());
            std::copy_n(k1.data(), k1.size(), mFssGenFeature.k.mShares[1].data());

        } else if (real_idx == 2) {
            i64 r21 = 6;
            i64 r20 = 5;
            mFssGenFeature.r = {{r21, r20}};
            std::vector<u64> k0;
            fss_read(fssp2k0,k0,FN());
            std::vector<u64> k1;
            fss_read(fssp2k1,k1,FN());
            if ( mFssGenFeature.k.rows() != k0.size() || (mFssGenFeature.k.rows() != FN()))
                throw std::invalid_argument("P2 mFssGenFeature.k number invalid!");
            std::copy_n(k0.data(), k0.size(), mFssGenFeature.k.mShares[0].data());
            std::copy_n(k1.data(), k1.size(), mFssGenFeature.k.mShares[1].data());
        } else
            throw std::logic_error("Invalid pIdx");

    }

    void DtreeModel::realTreeFssPreprocess(int pIdx) {
        //  r < gen_num
        int real_idx = realIdx(pIdx);
        if (real_idx == 0) {
            i64 r02 = 4;
            i64 r01 = 4;
            mFssGenNodes.r = {{r02, r01}};
            std::vector<u64> k0;
            fss_read(fssp0k1,k0,NN());
            std::vector<u64> k1;
            fss_read(fssp0k2,k1,NN());
            LOG(INFO) << "mFssGenNodes.k.rows(): \n" << mFssGenNodes.k.rows();
            LOG(INFO) << "k0.size(): \n" << k0.size();
            LOG(INFO) << "_nodes_num: \n" << NN();
            if ( mFssGenNodes.k.rows() != k0.size()|| (mFssGenNodes.k.rows() != NN()))
                throw std::invalid_argument("P0 mFssGenNodes.k number invalid!");
            std::copy_n(k0.data(), k0.size(), mFssGenNodes.k.mShares[0].data());
            std::copy_n(k1.data(), k1.size(), mFssGenNodes.k.mShares[1].data());

        } else if (real_idx == 1) {
            i64 r10 = 1;
            i64 r12 = 7;
            mFssGenNodes.r = {{r10, r12}};
            std::vector<u64> k0;
            fss_read(fssp1k0,k0,NN());
            std::vector<u64> k1;
            fss_read(fssp1k2,k1,NN());
            if ( mFssGenNodes.k.rows() != k0.size() || (mFssGenNodes.k.rows() != NN()))
                throw std::invalid_argument("P1 mFssGenNodes.k number invalid!");
            std::copy_n(k0.data(), k0.size(), mFssGenNodes.k.mShares[0].data());
            std::copy_n(k1.data(), k1.size(), mFssGenNodes.k.mShares[1].data());

        } else if (real_idx == 2) {
            i64 r21 = 6;
            i64 r20 = 5;
            mFssGenNodes.r = {{r21, r20}};
            std::vector<u64> k0;
            fss_read(fssp2k0,k0,NN());
            std::vector<u64> k1;
            fss_read(fssp2k1,k1,NN());
            if ( mFssGenNodes.k.rows() != k0.size() || (mFssGenNodes.k.rows() != NN()))
                throw std::invalid_argument("P2 mFssGenNodes.k number invalid!");
            std::copy_n(k0.data(), k0.size(), mFssGenNodes.k.mShares[0].data());
            std::copy_n(k1.data(), k1.size(), mFssGenNodes.k.mShares[1].data());
        } else
            throw std::logic_error("Invalid pIdx");

    }

    
}
