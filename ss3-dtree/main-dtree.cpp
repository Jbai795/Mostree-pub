#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Common/CLP.h>
#include "aby3/sh3/Sh3Types.h"
#include "aby3/sh3/Sh3FixedPoint.h"
#include "SS3Dtree.h"
#include "DtreeModel.h"
#include "main-dtree.h"
#include "MostreeLogEigenOps.h"
#include "dpf/dpf.h"
#include "ss3-dtree/channel/CommHelpers.h"

// NTL
#include <NTL/GF2E.h>
using namespace NTL;

using namespace std;
using namespace oc;
using namespace aby3;
using namespace CW::LOGINTER;

aby3::Sh3Evaluator sh3Eval;

const Decimal Dec(Decimal::D8);
// Parameters setting
int DTREE_DEPTH = 3;
const int PARTY_NUM = 3;

bool MALICIOUS = 1; // 1 is malicious, 0 means semi-honest
bool MACSHARE = 0; // 1 means MAC key is generated randomly, 0 means MAC key is selected.
bool SCALABILITY = 0; // 1 means testing for scalability using fake data, 0 means testing real datasets
//  set modelid = 2 if SCALABILITY = 1 !!!!

/*P0: (x0, x2), P1: (x1, x0), P2: (x2, x1)*/
int mostree_bin_3pc(int pIdx, CLP &cmd, Session &chlPrev, Session &chlNext) {
    
    //set trees(7 selections)
    const char *filename[7] = {
		"wine",
		"breast",
		"digits",
		"spambase",
		"diabetes",
		"boston",
		"mnist"
	};

	//------------Tree path way(currently, we only use dectree_rootdir)-------------
	string dectree_rootdir = "../../../../ss3-dtree/UCI_dectrees/";//according to exe's location

	//modify here if testing other trees
	int modelid = 2; 

    // `-i id` will set `modelid=id`
    // `-s` will set `SCALABILITY = 1`
    // `-m` will set `MALICIOUS = 1`
    if(cmd.isSet("i") == true) {
        modelid = cmd.get<int>("i"); 
    }
    if(cmd.isSet("s") == true) {
        SCALABILITY = 1;
        modelid = 2;
    }
    if(cmd.isSet("m") == true) {
        MALICIOUS = 1;
    }

    if(SCALABILITY == 1){
        if( modelid != 2){
            cout << "Set modelid = 2 if you set SCALABILITY = 1 !!!" << endl;
            return 0;
        }    
    }

    if(modelid > 6 || modelid < 0){
        LOG(INFO) << "* Please give a correct model id: [0, 6]";
        return 0;
    }

    if(SCALABILITY == 1){
        // performing scalability test, we use digits as the base tree. 
        if(MALICIOUS == 1){
        cout << "Performing scalability test in Malicious model" << endl;
        }else{
        cout << "Performing scalability test in Semi-honest model" << endl;
        }
    }else{
        if(MALICIOUS == 1){
        cout << "Testing..." << filename[modelid] << " in Malicious model" << endl;
        }else{
        cout << "Testing..." << filename[modelid] << " in Semi-honest model" << endl;
        }
    }
	
	//use to hide the real depth of tree
	uint32_t depthHide = 0;//not use

	uint64_t featureDim[7] = {7, 12, 47, 57, 10, 13, 784};

	/*tree nodes, feature numbers and depth(m,n,d)
	{wine, 23,7,5}
	{breast:43,12,7}
	{digits: 337,47,15}
	{spambase:171,57,17}
	{diabetes: 787,10, 28}
	{boston:851,13, 30}
	{mnist:4179,784, 20}
	*/

    // change here for scalablity tree depth ranges from 20 to 50
    int scal_depth = 50;

    //read selected tree from a file
    DecTree tree;
    tree.read_from_file_pack(dectree_rootdir + filename[modelid]);
    int totalNum;
    if(SCALABILITY == 1){
        totalNum = 25 * scal_depth;
        DTREE_DEPTH = scal_depth;
        if(pIdx==0)
            LOG(INFO) << "Tested Tree Depth is " << scal_depth;
    }else{
        totalNum = tree.num_dec_nodes + tree.num_of_leaves;
        DTREE_DEPTH = tree.depth + depthHide;
    }
    //uint64_t featureMax = pow(2, ceil(log(featureDim) / log(2)));


    /*
    print tree array
    */
    // if (pIdx == 0){
    //     for(int i = 0; i < totalNum; i++){
    //         std::cout << tree.thres[i] << " " << tree.left[i] << " " << tree.right[i] << " " << tree.map[i] << " " << tree.label[i] << endl;
    //     }
    // }
    
    const Decimal D = D0;
    const u64 FEATURE_NUM = featureDim[modelid];
    const u64 ROOT_NODE = 0;


    /*
    output trees' information
    */
    // LOG(INFO) << "tree.num_attributes in use " << tree.num_attributes << ", tree.featureDim in real " << FEATURE_NUM << '\n';
    // LOG(INFO) << "tree.depth " << tree.depth  << '\n';
    // LOG(INFO) << "tree.num_dec_nodes " << tree.num_dec_nodes  << '\n';
    // LOG(INFO) << "tree.num_of_leaves " << tree.num_of_leaves  << '\n';
    // LOG(INFO) << "tree total nodes " << totalNum  << '\n';


    //pad feature to the nearest 2^k
    u64 featureMax = pow(2, ceil(log(FEATURE_NUM)/log(2)));
    //pad tree to the nearest 2^k
    u64 nodesMax = pow(2, ceil(log(totalNum)/log(2)));

    /*
     * Step 1  Init network and params
     */
    SS3Dtree ss3dtree(64);
    ss3dtree.init(pIdx, chlPrev, chlNext, toBlock(pIdx));
    DtreeModel model(featureMax,nodesMax);
    // init mac key for 2to3
    //p0 is 2, p1 is 2, p2 is 2
    // u64 selected_part_mac_key = 3;
    // su64 ss_mac_key_for_2to3 = ss3dtree.getShareMACKeyU64Data(pIdx, selected_part_mac_key);


    /*
     * Step 2  Init dtree model/maced model and share model/maced model
     */
    // Init for a selected MAC key
    i64 mac_key = 2;

    // mac key
    sb64 ss_mac_key;
    sbMatrix ss_model_data;
    sbMatrix ss_mac_model_data;

    if(MACSHARE == 0){
        if(MALICIOUS == 1){
            LOG(INFO) <<"malicious mac key not share";
            // Init mac related data
            model.load_mac_model(&tree,nodesMax,mac_key);
            ss_model_data = ss3dtree.getShareModelBinData(pIdx, model);
            ss_mac_model_data = ss3dtree.getShareMACModelBinData(pIdx, model);
            ss_mac_key = ss3dtree.getShareMACKeyBinData(pIdx, mac_key);

        }else{
            LOG(INFO) <<"semi no mac key";
            model.load_model(&tree,nodesMax);
            ss_model_data = ss3dtree.getShareModelBinData(pIdx, model);
        } 
    }else{
        if(MALICIOUS == 1){
            LOG(INFO) <<"malicious mac key share";
            model.load_model(&tree,nodesMax);
            ss_model_data = ss3dtree.getShareModelBinData(pIdx, model);
            /*
            get a shared random mac key
            */
            ss_mac_key = ss3dtree.getShareRandBinData();
            // LOG(INFO) <<"************ " << ss_mac_key;
            // LOG(INFO) <<"************ " << ss3dtree.reveal(ss_mac_key);
            
            /*
            use mac key to encrypt model and share maced model
            */
            ss_mac_model_data = ss3dtree.getShareMACModelBinData(pIdx, model.ssModel, model.ssMACModel, ss_mac_key);
        }else{
            LOG(INFO) <<"semi no mac key";
            model.load_model(&tree,nodesMax);
            ss_model_data = ss3dtree.getShareModelBinData(pIdx, model);
        }
    }


    // /*
    //  * test steps, to see how to share a 64*1 matrix, each element is 8 bits. 
    //  * x < y
    //  */

    // //  假设party0 准备 value x
    
    // u8Matrix x;
    // if (pIdx == 0) {
    //     x.resize(MODEL_BITS, 1);
    //     for (u64 i = 0; i < x.size(); ++i) x(i) = 0;
    // }
    // su8Matrix ssValueScheme = ss3dtree.shareValueSchema(pIdx, x, model);

    // // prepare a plaintext value y
    // u8Matrix y;
    // y.resize(MODEL_BITS, 1);
    // for (u64 i = 0; i < y.size(); ++i) y(i) = 1;

    // /*
    //  * Step 2.5  compute DGK vector dgkScheme
    //  */

    // su8Matrix ssDGKSchema = ss3dtree.ssDGKFun(pIdx, model, y);

    // uint32_t depth = (uint32_t)std::ceil(std::log2((double)MODEL_BITS));

    // // BATCH DPF test offline phase
    // // party0 sends 2^depth DPF keys to party1 and party2 respectively
    // const uint32_t N = 1u << depth;  
    // // ================= party 0: keygen + send =================
    // if (pIdx == 0) {
    //     ss3dtree.sendDPFKeyBatchFromParty0(depth, ss_mac_key_for_2to3);//party0生成f(x) = (1,beta2) if x == alpha, else f(x) = (0,0) DPF keys，并发送给party1和party2
    // }
    
    // // ========= 接收 batch DPF keys + rdx shares =========
    // if (pIdx == 1 || pIdx == 2) ss3dtree.recvDPFKeyBatch();
    // std::cout << "[BATCH-TEST] party" << pIdx << " batch received.\n";
    
    // #ifdef SET_MALICIOUS 
    // //BATCH DPF test online phase
    // // ================= party 0: send batch =================
    // if(pIdx == 0){
    //     ss3dtree.sendMaskedRdxBatch(ssDGKSchema); // u8 vector
    // }
    // #endif

    // su64 compare_result;
    // su64 maced_compare_result;
    // // ================= party 1/2: recv batch =================
    // if (pIdx == 1 || pIdx == 2) {
    //     #ifdef SET_MALICIOUS 
    //     // ========= step 接收来自party0的 masked rdx batch =========
    //     ss3dtree.recvMaskedRdxBatch(); // u8 vector
    //     std::cout << "[BATCH-TEST] party" << pIdx << " masked rdx batch received.\n";
    //     #endif
    //     // ========= step 1: 本地准备 masked rdx batch =========
    //     std::vector<u8> my_to_peer;
    //     ss3dtree.prepareMaskedRdxBatch(ssDGKSchema, my_to_peer);    // su8Matrix

    //     // ========= step 2: 一次性交换 masked rdx =========
    //     std::vector<u8> peer_to_me;
    //     ss3dtree.exchangeMaskedRdxBatch(
    //         my_to_peer,
    //         peer_to_me
    //     );

    //     #ifdef SET_MALICIOUS
    //     ss3dtree.maskedRdxCheck(peer_to_me);
    //     #endif

    //     // ========= step 3: 本地 batch eval =========
    //     vector <uint64_t> dpfShares;
    //     dpfShares.resize(ss3dtree.getDPFN());
    //     vector <uint64_t> dpfMACShares;
    //     dpfMACShares.resize(ss3dtree.getDPFN());
    //     DpfEvalInt2 dpfOutput;
    //     uint64_t dpfSharesSum = 0;
    //     uint64_t dpfMACSharesSum = 0;
    //     for (size_t i = 0; i < ss3dtree.getDPFN(); ++i) {

    //         dpfOutput = ss3dtree.evalDPFBatchWithMAC(i, peer_to_me, ssDGKSchema); // 对应 peer_to_me 的 index
    //         dpfShares[i] = dpfOutput.v1;
    //         dpfMACShares[i] = dpfOutput.v2;
    //         dpfSharesSum += dpfShares[i];
    //         dpfMACSharesSum += dpfMACShares[i];
    //     }

    //     if(pIdx==1)
    //         dpfMACSharesSum = dpfSharesSum * ss_mac_key_for_2to3[0] + dpfMACSharesSum;
    //     else//pIdx==2
    //         dpfMACSharesSum = dpfSharesSum * ss_mac_key_for_2to3[1] + dpfMACSharesSum;
        
    //     //p1 and p2 reshare comparison result and maced comparison result
    //     compare_result = ss3dtree.getShareU64Data(pIdx, dpfSharesSum);
    //     maced_compare_result = ss3dtree.getShareU64Data(pIdx, dpfMACSharesSum);    
    // }

    // if(pIdx == 0){
    //     //p0 reshare
    //     compare_result = ss3dtree.getShareU64Data(pIdx, u64(0));
    //     maced_compare_result = ss3dtree.getShareU64Data(pIdx, u64(0));
    // }
    // //check reshare
    // su64 mac_multi_compare_result;
    // mac_multi_compare_result = ss3dtree.rss_u64_mul(ss_mac_key_for_2to3, compare_result);


    // su64 ret = mac_multi_compare_result - maced_compare_result;
    // auto revealed = ss3dtree.reveal(ret);
    // if (pIdx == 0) {
    //     LOG(INFO) << "Comparison result revealed: \n" << revealed;
    // }


    // /*
    //  * test steps, to see how to share a 64*1 matrix, each element is 8 bits. 
    //  * x < y
    //  */

    // //  假设party0 准备 value x
    
    // u8Matrix x;
    // if (pIdx == 0) {
    //     x.resize(MODEL_BITS, 1);
    //     for (u64 i = 0; i < x.size(); ++i) x(i) = 0;
    // }
    // su8Matrix ssValueScheme = ss3dtree.shareValueSchema(pIdx, x, model);

    // // prepare a plaintext value y
    // u8Matrix y;
    // y.resize(MODEL_BITS, 1);
    // for (u64 i = 0; i < y.size(); ++i) y(i) = 1;

    // /*
    //  * Step 2.5  compute DGK vector dgkScheme
    //  */

    // su8Matrix ssDGKSchema = ss3dtree.ssDGKSchema(pIdx, model, y);

    // // u8 a0 = ssDGKSchema(0,0)[0];
    // // u8 a1 = ssDGKSchema(0,0)[1];
    // // std::cout << "pIdx=" << pIdx << " ssDGKSchema(0,0)=(" << (unsigned)a0 << "," << (unsigned)a1 << ")" << std::endl;



    // //DPF tests if there is only one 0 in the vector f
    // //suppose party0 is DPF key generator and party1,party2 are DPF evaluators.
    // //party 0 generates two DPF keys for a random value alpha and sends the keys to party1 and party2 respectively.
    // //party1 and party2 evaluate the DPF keys to see whether there is only one 0 in the vector f.
    // //u64 alpha = 5; //randomly select an index where f_alpha = 0
    // // if(pIdx == 0){
    // //     u64 alpha = rand() % MODEL_BITS; //randomly select an index where f_alpha = 1
    // //     std::cout << "DPF test, randomly selected alpha index is: " << alpha << std::endl;
    // //     // party0_gen_and_send(0, ceil(log(MODEL_BITS)/log(2)), alpha);
    // //     ss3dtree.sendDPFKeysFromParty0(ceil(log(MODEL_BITS)/log(2)), alpha);
    // // }

    // // if(pIdx == 1 || pIdx == 2){
    // //     ss3dtree.recvDPFKeyFromParty0();
    // // }

    // uint32_t depth = (uint32_t)std::ceil(std::log2((double)MODEL_BITS));

    // #ifdef DPF_SINGLE
    // // ssDGKSchema(0,0)
    // if (pIdx == 0) {
    //     uint32_t alpha = rand() % (1 << depth); //randomly select an small index  < 2^depth where f_alpha = 1
    //     // std::cout << "DPF test, randomly selected alpha index is: " << alpha << std::endl;

    //     ss3dtree.sendDPFKeysFromParty0(depth, alpha);

    //     #ifdef CORRET_DPF_TEST
    //     // 临时 debug： party0接收来自p1和p2的DPF计算后的结果，检查正确性
    //     ss3dtree.recvAndCheckDPFShares();
    //     #endif
    // }

    // if (pIdx == 1 || pIdx == 2) {
    //     auto [alpha_recv, share] = ss3dtree.recvAndEvalDPFfromParty0(depth, ssDGKSchema(0,0));
    //     // std::cout << "P" << pIdx << " recv alpha=" << alpha_recv << " share=" << share << std::endl;

    //     #ifdef CORRET_DPF_TEST
    //     // 临时 debug：把 share 发回 party0
    //     ss3dtree.sendDPFShareToParty0(share);
    //     #endif
    // }
    // #endif // DPF_SINGLE


    // // BATCH DPF test offline phase
    // // party0 sends 2^depth DPF keys to party1 and party2 respectively
    // const uint32_t N = 1u << depth;  
    // // ================= party 0: keygen + send =================
    // if (pIdx == 0) {
    //     // std::cout << "[BATCH-TEST] party0 sending " << N << " DPF keys (depth=" << depth << ")\n";
    //     // std::cout<< "P0 mac key is " << uint64_t(ss_mac_key_for_2to3[0]+ss_mac_key_for_2to3[1]) << std::endl;

    //     ss3dtree.sendDPFKeyBatchFromParty0(depth, ss_mac_key_for_2to3);//party0生成f(x) = (1,beta2) if x == alpha, else f(x) = (0,0) DPF keys，并发送给party1和party2

    //     // 可选：做 correctness check，需要 party1/2 把 shares 发回来
    //     // ss3dtree.recvAndCheckDPFBatchShares(/*rounds=*/20, depth);
    // }
    
    // // ========= 接收 batch DPF keys + rdx shares =========
    // if (pIdx == 1 || pIdx == 2) ss3dtree.recvDPFKeyBatch();
    // std::cout << "[BATCH-TEST] party" << pIdx << " batch received.\n";
    
    // #ifdef SET_MALICIOUS 
    // //BATCH DPF test online phase
    // // ================= party 0: send batch =================
    // if(pIdx == 0){
    //     ss3dtree.sendMaskedRdxBatch(ssDGKSchema); // u8 vector
    // }
    // #endif

    // su64 compare_result;
    // su64 maced_compare_result;
    // // ================= party 1/2: recv batch =================
    // if (pIdx == 1 || pIdx == 2) {
    //     #ifdef SET_MALICIOUS 
    //     // ========= step 接收来自party0的 masked rdx batch =========
    //     ss3dtree.recvMaskedRdxBatch(); // u8 vector
    //     std::cout << "[BATCH-TEST] party" << pIdx << " masked rdx batch received.\n";
    //     #endif
    //     // ========= step 1: 本地准备 masked rdx batch =========
    //     std::vector<u8> my_to_peer;
    //     ss3dtree.prepareMaskedRdxBatch(ssDGKSchema, my_to_peer);    // su8Matrix

    //     // ========= step 2: 一次性交换 masked rdx =========
    //     std::vector<u8> peer_to_me;
    //     ss3dtree.exchangeMaskedRdxBatch(
    //         my_to_peer,
    //         peer_to_me
    //     );

    //     #ifdef SET_MALICIOUS
    //     ss3dtree.maskedRdxCheck(peer_to_me);
    //     #endif

    //     // ========= step 3: 本地 batch eval =========
    //     vector <uint64_t> dpfShares;
    //     dpfShares.resize(ss3dtree.getDPFN());
    //     vector <uint64_t> dpfMACShares;
    //     dpfMACShares.resize(ss3dtree.getDPFN());
    //     DpfEvalInt2 dpfOutput;
    //     uint64_t dpfSharesSum = 0;
    //     uint64_t dpfMACSharesSum = 0;
    //     for (size_t i = 0; i < ss3dtree.getDPFN(); ++i) {

    //         dpfOutput = ss3dtree.evalDPFBatchWithMAC(i, peer_to_me, ssDGKSchema); // 对应 peer_to_me 的 index
    //         dpfShares[i] = dpfOutput.v1;
    //         dpfMACShares[i] = dpfOutput.v2;
    //         dpfSharesSum += dpfShares[i];
    //         dpfMACSharesSum += dpfMACShares[i];

    //         // std::cout << "[BATCH-TEST] party" << pIdx << " idx=" << i << " share=" << dpfShares[i] << "\n";
    //         // std::cout << "[BATCH-TEST] party" << pIdx << " idx=" << i << " MAC share=" << dpfMACShares[i] << "\n";
    //         // 可选：发回 party0 做 correctness check
    //         // ss3dtree.sendDPFBatchShareToParty0(i, alpha, dpfShares[i]);
    //     }

    //     if(pIdx==1)
    //         dpfMACSharesSum = dpfSharesSum * ss_mac_key_for_2to3[0] + dpfMACSharesSum;
    //     else//pIdx==2
    //         dpfMACSharesSum = dpfSharesSum * ss_mac_key_for_2to3[1] + dpfMACSharesSum;
        
    //     //p1 and p2 reshare comparison result and maced comparison result
    //     compare_result = ss3dtree.getShareMACKeyU64Data(pIdx, dpfSharesSum);
    //     maced_compare_result = ss3dtree.getShareMACKeyU64Data(pIdx, dpfMACSharesSum);  

        
    //     // std::cout << "[BATCH-TEST] party" << pIdx << " total share sum= " << dpfSharesSum << "\n";
    //     // std::cout << "[BATCH-TEST] party" << pIdx << " total MAC share sum= " << dpfMACSharesSum << "\n";
    //     // std::cout << "test output is " << u64(7198098717934757398+ 11248645355774794219) << std::endl;
    //     // std::cout << "test output is " << u64(2592748365871554613+ 15853995707837997012) << std::endl;    
    // }

    // if(pIdx == 0){
    //     //p0 reshare
    //     compare_result = ss3dtree.getShareMACKeyU64Data(pIdx, u64(0));
    //     maced_compare_result = ss3dtree.getShareMACKeyU64Data(pIdx, u64(0));
    // }
    // // std::cout << "reveal compare_result compare_result[0] is " << compare_result[0] << " compare_result[0] is " << compare_result[1] << std::endl;
    // // std::cout << "test output is " << u64(10082085126823338503+ 13175973558201821505+13635429462393943225) << std::endl;    
    
    // // //reveal to check
    // // auto revealed1 = ss3dtree.reveal(compare_result);
    // // auto revealed2 = ss3dtree.reveal(maced_compare_result);
    // // if (pIdx == 0) {
    // //     LOG(INFO) << "Comparison result revealed: \n" << revealed1;
    // //     LOG(INFO) << "MACed Comparison result revealed: \n" << revealed2;
    // // }

    // //check reshare
    // su64 mac_multi_compare_result;
    // mac_multi_compare_result = ss3dtree.rss_u64_mul(ss_mac_key_for_2to3, compare_result);


    // su64 ret = mac_multi_compare_result - maced_compare_result;
    // auto revealed = ss3dtree.reveal(ret);
    // if (pIdx == 0) {
    //     LOG(INFO) << "Comparison result revealed: \n" << revealed;
    // }

    


    


    
    /*
     * Step 3  Init feature and share feature
     */
    model.load_feature(featureMax);
    sbMatrix ssFeatureSchema = ss3dtree.shareFeatureSchema(pIdx, model);
    // auto revealed = ss3dtree.reveal(ssFeatureSchema);
    // if (pIdx == 0) {
    //     LOG(INFO) <<"-------- ssFeatureSchema ---------- " << ssFeatureSchema;
    //     LOG(INFO) << "Model feature schema: \n" << revealed;
    // }

    /*
     * Step 4  Parameter generation
     */

    // mac check
    sb64 gamma;
    sb64 rho[DTREE_DEPTH];

    // triples for poly_mul
    sb64 a[DTREE_DEPTH];
    sb64 b[DTREE_DEPTH];
    sb64 c[DTREE_DEPTH];

    // triples for and_ss
    sb64 as[DTREE_DEPTH*140];// 140 for each comparison, we use a bit big number
    sb64 bs[DTREE_DEPTH*140];
    sb64 cs[DTREE_DEPTH*140];

    ss3dtree.mNext.resetStats();
    ss3dtree.mPrev.resetStats();

    if(MALICIOUS == 1){
        auto start_para = std::chrono::steady_clock::now();
        //generate random values for mac checking including: gamma, depth numbers rho
        gamma = ss3dtree.getShareRandBinData();
        // LOG(INFO) <<"************ " << gamma;
        // LOG(INFO) <<"************ " << ss3dtree.reveal(gamma);

        for(int i = 0; i < DTREE_DEPTH; i++){
            rho[i] = ss3dtree.getShareRandBinData();
            // LOG(INFO) <<"************ " << rho[i];
            // LOG(INFO) <<"************ " << ss3dtree.reveal(rho[i]);
        }

        //generate triples for poly_mul
        for(int i = 0; i < DTREE_DEPTH; i++){
            a[i] = ss3dtree.getShareRandBinData();
            b[i] = ss3dtree.getShareRandBinData();
            c[i] = ss3dtree.poly_mul(a[i], b[i]);
        }

        // if(modelid < 4 ){
            //generate triples for and_ss
        for(int i = 0; i < DTREE_DEPTH*140; i++){
            as[i] = ss3dtree.getShareRandBinData();
            bs[i] = ss3dtree.getShareRandBinData();
            cs[i] = ss3dtree.and_ss(as[i], bs[i]);
        }
        // }
        


        if(pIdx==0)
            LOG(INFO) << "**********************Offline******************** ";
        auto end_para = std::chrono::steady_clock::now();
        double time_para = std::chrono::duration<double,std::milli>(end_para - start_para).count();
        double bytes_para = ss3dtree.mNext.getTotalDataSent() + ss3dtree.mPrev.getTotalDataSent();
        if(pIdx==0){
            LOG(INFO) << "Party " << pIdx << " offline time is : " << time_para << " ms";
            // ss << "Party " << pIdx << " communication: " << bytes << " bytes";
            LOG(INFO) << "Party " << pIdx << " offline communication is : " << bytes_para << " bytes";
        }
        
    }

    /*
     * FSS init and oblivious selection feature protocol
     * Step 4 Root node oblivious selection feature
     */
    // 4.1 FSS preprocess
    ss3dtree.realFeatureFssPreprocess(pIdx,model); //8 for wine
    ss3dtree.realTreeFssPreprocess(pIdx,model); //64 for wine
    SSNode ssRootNode = model.getSSNode(ROOT_NODE);


    // 4.2 Online: Travel tree
    ss3dtree.mNext.resetStats();
    ss3dtree.mPrev.resetStats();

    // classification label
    sb64 ssLabel;

    auto start = std::chrono::steady_clock::now();
    if(pIdx==0)
        LOG(INFO) << "**********************Online******************** ";
    if(MALICIOUS == 1){
        ssLabel = ss3dtree.travelTree(pIdx, DTREE_DEPTH, ssRootNode, model, ss_model_data, ss_mac_model_data, ssFeatureSchema, ss_mac_key, gamma, rho, a, b, c, as, bs, cs);
    }else{
        ssLabel = ss3dtree.travelTree(pIdx, DTREE_DEPTH, ssRootNode, model, ss_model_data, ssFeatureSchema);
    }
    auto end = std::chrono::steady_clock::now();
    double time = std::chrono::duration<double,std::milli>(end - start).count();

    
    LOG(INFO) << "Party " << pIdx << " get mpc decision tree label: " << ss3dtree.reveal(ssLabel) << " success!";

    if(pIdx==0)
        LOG(INFO) << "Party " << pIdx << " online travel time is : " << time << " ms";

    double bytes = ss3dtree.mNext.getTotalDataSent() + ss3dtree.mPrev.getTotalDataSent();
    if(pIdx==0)
        LOG(INFO) << "Party " << pIdx << " online communication is : " << bytes << " bytes";
    // std::stringstream ss;
    // ss << "Party " << pIdx << " communication: " << bytes << " bytes";
    // LOG(INFO) << ss.str() << std::flush;

    return 0;
}

int dtree_main_3pc(oc::CLP &cmd) {
    IOService ios;
    
    std::vector<std::thread> thrds;
    for (u64 i = 0; i < PARTY_NUM; ++i) {
        this_thread::sleep_for(10ms);
        if (cmd.isSet("p") == false || cmd.get<int>("p") == i) {
            thrds.emplace_back(std::thread([i, &cmd, &ios]() {

                auto next = (i + 1) % 3;
                auto prev = (i + 2) % 3;
                auto cNameNext = std::to_string(std::min(i, next)) + std::to_string(std::max(i, next));
                auto cNamePrev = std::to_string(std::min(i, prev)) + std::to_string(std::max(i, prev));

                auto modeNext = i < next ? SessionMode::Server : SessionMode::Client;
                auto modePrev = i < prev ? SessionMode::Server : SessionMode::Client;
                auto portNext = 1212 + std::min(i, next);
                auto portPrev = 1212 + std::min(i, prev);

                Session epNext(ios, "127.0.0.1", portNext, modeNext, cNameNext);
                Session epPrev(ios, "127.0.0.1", portPrev, modePrev, cNamePrev);

                ostreamLock(std::cout << "party " << i << " next " << portNext << " mode=server?:"
                                      << (modeNext == SessionMode::Server) << " name " << cNameNext << std::endl);
                ostreamLock(std::cout << "party " << i << " prev " << portPrev << " mode=server?:"
                                      << (modePrev == SessionMode::Server) << " name " << cNamePrev << std::endl);
                auto chlNext = epNext.addChannel();
                auto chlPrev = epPrev.addChannel();

                chlNext.waitForConnection();
                chlPrev.waitForConnection();
                chlNext.send(i);
                chlPrev.send(i);
                u64 prevAct = 99, nextAct = 99;
                chlNext.recv(nextAct);
                chlPrev.recv(prevAct);

                if (next != nextAct)
                    std::cout << " bad next party idx, act: " << nextAct << " exp: " << next << std::endl;
                if (prev != prevAct)
                    std::cout << " bad prev party idx, act: " << prevAct << " exp: " << prev << std::endl;

                mostree_bin_3pc(i, cmd, epPrev, epNext);

            }));
        }
    }

    for (auto &t: thrds)
        t.join();

    return 0;
}

void help()
{
    std::cerr << "+ Mostree-3pc Command Help" << std::endl;
    std::cerr << "01) -travel [-p] [0 1 2] [-i modelid] [-s] [-m]     ~~ to travel decision tree on party [-p n]" << std::endl;
}

int main(int argc, char **argv) {
    try {
        bool set = false;
        oc::CLP cmd(argc, argv);

        if (cmd.isSet("travel")) {
            set = true;
            dtree_main_3pc(cmd);
        }

        if (set == false) {
            help();
        }

    }
    catch (std::exception& e)
    {
        LOG(INFO) << e.what() << std::endl;
    }
}

