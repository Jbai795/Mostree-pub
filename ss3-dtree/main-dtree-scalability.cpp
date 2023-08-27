// #include <iostream>
// #include <string>
// #include <vector>
// #include <algorithm>
// #include <chrono>

// #include <cryptoTools/Network/IOService.h>
// #include <cryptoTools/Common/CLP.h>
// #include "aby3/sh3/Sh3Types.h"
// #include "aby3/sh3/Sh3FixedPoint.h"
// #include "SS3Dtree.h"
// #include "DtreeModel.h"
// #include "main-dtree.h"
// // NTL
// #include <NTL/GF2E.h>
// using namespace NTL;

// using namespace std;
// using namespace oc;
// using namespace aby3;
// using namespace CW::LOGINTER;

// const Decimal Dec(Decimal::D8);
// // Parameters setting
// int DTREE_DEPTH = 3;
// const int PARTY_NUM = 3;

// int mostree_bin_3pc(int pIdx, CLP &cmd, Session &chlPrev, Session &chlNext) {
    
//     //set tree(8 selections)
//     const char *filename[8] = {
// 		"wine",
// 		"linnerud",
// 		"breast",
// 		"digits",
// 		"spambase",
// 		"diabetes",
// 		"boston",
// 		"mnist"
// 	};

// 	//------------Tree path way(currently, we only use dectree_rootdir)-------------
// 	string dectree_rootdir = "../../../../ss3-dtree/UCI_dectrees/";//according to exe's location


// 	//modify here if testing other trees
// 	int modelid = 3;//modelid=1 is not available since feature number is smaller than one of our default select index.
// 	cout << "Testing..." << filename[modelid] << endl;

// 	//use to hide the real depth of tree
// 	uint32_t depthHide = 35;//range from 3-35 for testing scalability

// 	uint64_t featureDim[12] = {7, 3, 12, 47, 57, 10, 13, 784};

// 	/*tree nodes and depth(n,d)
// 	{wine, 7,5}
// 	{linnerud:3,6}
// 	{breast:12,7}
// 	{digits: 47,15}
// 	{spambase:57,17}
// 	{diabetes: 10, 28}
// 	{boston:13, 30}
// 	{mnist:784, 20}
// 	*/

//     //read selected tree from a file
//     DecTree tree;
//     tree.read_from_file_pack(dectree_rootdir + filename[modelid]);
//     int totalNum = tree.num_dec_nodes + tree.num_of_leaves;
//     //uint64_t featureMax = pow(2, ceil(log(featureDim) / log(2)));


//     /*
//     print tree array
//     */
//     // if (pIdx == 0){
//     //     for(int i = 0; i < totalNum; i++){
//     //         std::cout << tree.thres[i] << " " << tree.left[i] << " " << tree.right[i] << " " << tree.map[i] << " " << tree.label[i] << endl;
//     //     }
//     // }
//     sleep(5);


//     uint64_t attri;
//     //for compute delta under arithmetic
//     uint32_t r_server = 2;
//     uint32_t r_client = 0;
    
    
//     const Decimal D = D0;
//     const u64 FEATURE_NUM = featureDim[modelid];
//     const i64 SELECTED_IDX = tree.depth;
//     const u64 ROOT_NODE = 0;
//     DTREE_DEPTH = tree.depth + 1;

//     //output trees' information
//     LOG(INFO) << "tree.num_attributes in use " << tree.num_attributes << ", tree.featureDim in real " << FEATURE_NUM << '\n';
//     LOG(INFO) << "tree.depth " << tree.depth  << '\n';
//     LOG(INFO) << "tree.num_dec_nodes " << tree.num_dec_nodes  << '\n';
//     LOG(INFO) << "tree.num_of_leaves " << tree.num_of_leaves  << '\n';
//     LOG(INFO) << "tree total nodes " << totalNum  << '\n';


//     //pad feature
//     u64 featureMax = pow(2, ceil(log(FEATURE_NUM)/log(2)));
//     //pad tree
//     totalNum = 25 * (depthHide + tree.depth);
//     u64 nodesMax = pow(2, ceil(log(totalNum)/log(2)));
//     if (pIdx == 0) {
//         LOG(INFO) << "* totalNum is : " << totalNum;
//         LOG(INFO) << "* nodesMax is : " << nodesMax;
//     }
//     /*
//      * Step 1  Init network and params
//      */
//     SS3Dtree ss3dtree(64);
//     ss3dtree.init(pIdx, chlPrev, chlNext, toBlock(pIdx));
//     DtreeModel model(featureMax,nodesMax);
//     if (pIdx == 0) {
//         LOG(INFO) << "* Feature number: " << FEATURE_NUM;
//         LOG(INFO) << "* Padded feature number: " << featureMax;
//         LOG(INFO) << "* Padded tree nodes number: " << nodesMax;
//         LOG(INFO) << "* Root node index: " << ROOT_NODE;
//     }

//     /*
//      * Step 2  Init dtree model and share model
//      */
//     // model.load_model_test();
//     //load real tree
//     if(modelid>=0 & modelid<=7){
//         model.load_model(&tree, nodesMax);
//     }else{
//         LOG(INFO) << "* Please give a correct model id: ";
//         return 0;
//     }

//     if (pIdx == 0)
//         LOG(INFO) << "Plain Model: \n" << model.getPlainModel();
//     sbMatrix ss_model_data = ss3dtree.getShareModelBinData(pIdx, model);

//     /*
//      * Step 3  Init feature and share feature
//      */
//     model.load_feature(featureMax);
//     sbMatrix ssFeatureSchema = ss3dtree.shareFeatureSchema(pIdx, model);
//     if (pIdx == 0) {
//         LOG(INFO) << "Model feature schema: \n" << model.getFeatureSchema();
//     }

//     /*
//      * Semi-honest oblivious selection feature protocol
//      * Step 4 Root node oblivious selection feature
//      */
//     // 4.1 FSS preprocess
//     //ss3dtree.fssPreprocess(pIdx, model);
//     ss3dtree.realFeatureFssPreprocess(pIdx,model); //8 for wine
//     ss3dtree.realTreeFssPreprocess(pIdx,model); //64 for wine
//     SSNode ssRootNode = model.getSSNode(ROOT_NODE);

//     // 4.2 Online: Travel tree
//     ss3dtree.mNext.resetStats();
//     ss3dtree.mPrev.resetStats();

//     auto start = std::chrono::steady_clock::now();
//     sb64 ssLabel = ss3dtree.travelTree(pIdx, DTREE_DEPTH+depthHide, ssRootNode, model, ss_model_data, ssFeatureSchema);
//     auto end = std::chrono::steady_clock::now();
//     double time = std::chrono::duration<double,std::milli>(end - start).count();

//     LOG(INFO) << "Tree travel time is " << time << " ms";

//     LOG(INFO) << "Party " << pIdx << " get mpc decision tree label: " << ss3dtree.reveal(ssLabel) << " success!";

//     double bytes = ss3dtree.mNext.getTotalDataSent() + ss3dtree.mPrev.getTotalDataSent();
//     std::stringstream ss;
//     ss << "Party " << pIdx << " communication: " << bytes << " bytes";
//     LOG(INFO) << ss.str() << std::flush;

//     return 0;
// }

// int dtree_main_3pc(oc::CLP &cmd) {
//     IOService ios;
    
//     std::vector<std::thread> thrds;
//     for (u64 i = 0; i < PARTY_NUM; ++i) {
//         this_thread::sleep_for(10ms);
//         if (cmd.isSet("p") == false || cmd.get<int>("p") == i) {
//             thrds.emplace_back(std::thread([i, &cmd, &ios]() {

//                 auto next = (i + 1) % 3;
//                 auto prev = (i + 2) % 3;
//                 auto cNameNext = std::to_string(std::min(i, next)) + std::to_string(std::max(i, next));
//                 auto cNamePrev = std::to_string(std::min(i, prev)) + std::to_string(std::max(i, prev));

//                 auto modeNext = i < next ? SessionMode::Server : SessionMode::Client;
//                 auto modePrev = i < prev ? SessionMode::Server : SessionMode::Client;
//                 auto portNext = 1212 + std::min(i, next);
//                 auto portPrev = 1212 + std::min(i, prev);

//                 Session epNext(ios, "127.0.0.1", portNext, modeNext, cNameNext);
//                 Session epPrev(ios, "127.0.0.1", portPrev, modePrev, cNamePrev);

//                 ostreamLock(std::cout << "party " << i << " next " << portNext << " mode=server?:"
//                                       << (modeNext == SessionMode::Server) << " name " << cNameNext << std::endl);
//                 ostreamLock(std::cout << "party " << i << " prev " << portPrev << " mode=server?:"
//                                       << (modePrev == SessionMode::Server) << " name " << cNamePrev << std::endl);
//                 auto chlNext = epNext.addChannel();
//                 auto chlPrev = epPrev.addChannel();

//                 chlNext.waitForConnection();
//                 chlPrev.waitForConnection();
//                 chlNext.send(i);
//                 chlPrev.send(i);
//                 u64 prevAct = 99, nextAct = 99;
//                 chlNext.recv(nextAct);
//                 chlPrev.recv(prevAct);

//                 if (next != nextAct)
//                     std::cout << " bad next party idx, act: " << nextAct << " exp: " << next << std::endl;
//                 if (prev != prevAct)
//                     std::cout << " bad prev party idx, act: " << prevAct << " exp: " << prev << std::endl;

//                 mostree_bin_3pc(i, cmd, epPrev, epNext);

//             }));
//         }
//     }

//     for (auto &t: thrds)
//         t.join();

//     return 0;
// }

// void help()
// {
//     std::cerr << "+ Mostree-3pc Command Help" << std::endl;
//     std::cerr << "01) -travel [-p] [0 1 2]      ~~ to travel decision tree on party [-p n]" << std::endl;
// }

// int main(int argc, char **argv) {
//     try {
//         bool set = false;
//         oc::CLP cmd(argc, argv);

//         if (cmd.isSet("travel")) {
//             set = true;
//             dtree_main_3pc(cmd);
//         }

//         if (set == false) {
//             help();
//         }

//     }
//     catch (std::exception& e)
//     {
//         LOG(INFO) << e.what() << std::endl;
//     }
// }

