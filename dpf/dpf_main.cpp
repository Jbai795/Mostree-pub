#include "dpf.h"
// #include "tree-doram/client.h"
// #include "tree-doram/server.h"

// int main(int argc, char **argv)
// {
//     // test();

//     uint32_t len = 6;
//     DPFKEY key0(len), key1(len);

//     uint32_t rdx = 39;

//     // for(uint32_t i = 1; i <= 32; i++) {
//     //     bool bit = get_bit(rdx, 32, i); 
//     //     std::cout<< bit <<std::endl;
//     // } 

//     // bool a =  true, b = false;
//     // std::cout<< (a ^ b) << std::endl;

//     block beta = makeBlock(0, 0x1);
//     gen_dpf_key_pair(rdx, len, beta, &key0, &key1); 
    
//     uint32_t buf_size = key0.get_buf_size(); 
//     char* buf0 = new char[buf_size];
//     char* buf1 = new char[buf_size]; 
    
//     // this part test serialization and deserialization
//     serialization(&key0, buf0);
//     serialization(&key1, buf1);
//     debug_print_dpfkey(key0, "party1 before send");
//     debug_print_dpfkey(key1, "party2 before send"); 

//     deserialization(buf0, &key0);
//     deserialization(buf1, &key1); 
//     debug_print_dpfkey(key0, "party1 after recv");
//     debug_print_dpfkey(key1, "party2 after recv");

//     block hash_out0[2], hash_out1[2];
//     int share0 = dpf_eval_int(0, &key0, rdx, hash_out0);
//     int share1 = dpf_eval_int(1, &key1, rdx, hash_out1);
//     std::cout << "share0 is " << share0 << std::endl;
//     std::cout << "share1 is " << share1 << std::endl;
    
//     std::cout<< "share: "  <<share0 + share1 <<std::endl;
//     std::cout<< "hash_out 0: "  << (hash_out0[0] ^ hash_out1[0]) <<std::endl;
//     std::cout<< "hash_out 1: "  << (hash_out0[1] ^ hash_out1[1]) <<std::endl;

//     // this part test batch dpf evaluation
//     // block* vec0 = new block[1 << key0.get_depth()];
//     // block* vec1 = new block[1 << key1.get_depth()];
//     // int* vec_int0 = new int[1 << key0.get_depth()];
//     // int* vec_int1 = new int[1 << key1.get_depth()];
//     // block proof0[2], proof1[2];
//     // std::cout<< "run: " << std::endl;
//     // dpf_batch_eval_int(0, &key0, vec0, vec_int0, proof0);
//     // dpf_batch_eval_int(1, &key1, vec1, vec_int1, proof1); 
//     // std::cout<< "proof: "  << (proof0[1] ^ proof1[1]) <<std::endl;

//     // for(uint32_t i = 0; i < 1 << key0.get_depth(); i++) {
//     //     std::cout<< "i = " << i << " : " << (vec_int0[i] + vec_int1[i]) << std::endl;  
//     //     // std::cout<< "i = " << i << " : " << "vec_int0 = " << vec_int0[i] << "and vec_int1 = " << vec_int1[i] << "total = " << (vec_int0[i] + vec_int1[i]) << std::endl;  
//     // }

//     // this part for VDPF 


//     delete[] buf0;
//     delete[] buf1; 
//     // delete[] vec0;
//     // delete[] vec1;
//     // delete[] vec_int0;
//     // delete[] vec_int1;
    
//     return 0;
// }


//test two outputs DPF
int main(int argc, char **argv)
{
    // test();

    uint32_t len = 6;
    //addtion one more depth for two outputs DPF
    DPFKEY key0(len), key1(len);

    uint32_t rdx = 39;

    // for(uint32_t i = 1; i <= 32; i++) {
    //     bool bit = get_bit(rdx, 32, i); 
    //     std::cout<< bit <<std::endl;
    // } 

    // bool a =  true, b = false;
    // std::cout<< (a ^ b) << std::endl;

    block beta1 = makeBlock(0, 0x1);
    block beta2 = makeBlock(0, 0x2);
    gen_dpf_key_pair_two(rdx, len, beta1, beta2, &key0, &key1); 
    
    uint32_t buf_size = key0.get_buf_size(); 
    char* buf0 = new char[buf_size];
    char* buf1 = new char[buf_size]; 
    
    // this part test serialization and deserialization
    serialization(&key0, buf0);
    serialization(&key1, buf1);
    debug_print_dpfkey(key0, "party1 before send");
    debug_print_dpfkey(key1, "party2 before send"); 

    deserialization(buf0, &key0);
    deserialization(buf1, &key1); 
    debug_print_dpfkey(key0, "party1 after recv");
    debug_print_dpfkey(key1, "party2 after recv");

    block hash_out0[2], hash_out1[2];
    DpfEvalInt2 share0 = dpf_eval_int_two(0, &key0, rdx, hash_out0);
    DpfEvalInt2 share1 = dpf_eval_int_two(1, &key1, rdx, hash_out1);
    std::cout << "share0.v1 is " << share0.v1 << ", share0.v2 is " << share0.v2 << std::endl;
    std::cout << "share1.v1 is " << share1.v1 << ", share1.v2 is " << share1.v2 << std::endl;
    
    std::cout<< "share0: "  << (share0.v1 + share1.v1) <<std::endl;
    std::cout<< "share1: "  << (share0.v2 + share1.v2) <<std::endl;
    std::cout<< "hash_out 0: "  << (hash_out0[0] ^ hash_out1[0]) <<std::endl;
    std::cout<< "hash_out 1: "  << (hash_out0[1] ^ hash_out1[1]) <<std::endl;

    // this part test batch dpf evaluation
    // block* vec0 = new block[1 << key0.get_depth()];
    // block* vec1 = new block[1 << key1.get_depth()];
    // int* vec_int0 = new int[1 << key0.get_depth()];
    // int* vec_int1 = new int[1 << key1.get_depth()];
    // block proof0[2], proof1[2];
    // std::cout<< "run: " << std::endl;
    // dpf_batch_eval_int(0, &key0, vec0, vec_int0, proof0);
    // dpf_batch_eval_int(1, &key1, vec1, vec_int1, proof1); 
    // std::cout<< "proof: "  << (proof0[1] ^ proof1[1]) <<std::endl;

    // for(uint32_t i = 0; i < 1 << key0.get_depth(); i++) {
    //     std::cout<< "i = " << i << " : " << (vec_int0[i] + vec_int1[i]) << std::endl;  
    //     // std::cout<< "i = " << i << " : " << "vec_int0 = " << vec_int0[i] << "and vec_int1 = " << vec_int1[i] << "total = " << (vec_int0[i] + vec_int1[i]) << std::endl;  
    // }

    // this part for VDPF 


    delete[] buf0;
    delete[] buf1; 
    // delete[] vec0;
    // delete[] vec1;
    // delete[] vec_int0;
    // delete[] vec_int1;
    
    return 0;
}
