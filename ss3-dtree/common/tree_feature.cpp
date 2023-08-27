#include "tree_feature.h"

void get_tree_and_feature(string filename, int featureDim) {
    srand((unsigned int) time(NULL)); // real random
    // tree.read_from_file(filename);
    DecTree tree;
    tree.read_from_file_pack(filename);
    int totalNum = tree.num_dec_nodes + tree.num_of_leaves;
    uint64_t featureMax = pow(2, ceil(log(featureDim) / log(2)));
    uint64_t attri;
    //for compute delta under arithmetic
    uint32_t r_server = 2;
    uint32_t r_client = 0;

//#ifdef DTREE_DEBUG
    cout << "tree.num_attributes in use " << tree.num_attributes << ", tree.featureDim in real " << featureDim << '\n';
    cout << "tree.depth " << tree.depth  << '\n';
    cout << "tree.num_dec_nodes " << tree.num_dec_nodes  << '\n';
    cout << "tree.num_of_leaves " << tree.num_of_leaves  << '\n';
    cout << "tree total nodes " << totalNum  << '\n';
//#endif
}
