#include "fss.h"
// #include "tree-doram/client.h"
// #include "tree-doram/server.h"

int main(int argc, char **argv)
{

    uint32_t len = 20;
    DPFKEY key0(len), key1(len);

    uint32_t rdx = 15;

    // for(uint32_t i = 1; i <= 32; i++) {
    //     bool bit = get_bit(rdx, 32, i); 
    //     std::cout<< bit <<std::endl;
    // } 

    // bool a =  true, b = false;
    // std::cout<< (a ^ b) << std::endl;

    block beta = makeBlock(0, 0xFFFF);
    gen_dpf_key_pair(rdx, len, beta, &key0, &key1); 
    
    uint32_t buf_size = key0.get_buf_size(); 
    char* buf0 = new char[buf_size];
    char* buf1 = new char[buf_size]; 
    
    // this part test serialization and deserialization
    serialization(&key0, buf0);
    serialization(&key1, buf1); 

    deserialization(buf0, &key0);
    deserialization(buf1, &key1); 

    block hash_out0[2], hash_out1[2];
    block share0 = dpf_eval(0, &key0, rdx, hash_out0);
    block share1 = dpf_eval(1, &key1, rdx, hash_out1);

    std::cout<< "share: "  << (share0 ^ share1) <<std::endl;
    std::cout<< "hash_out 0: "  << (hash_out0[0] ^ hash_out1[0]) <<std::endl;
    std::cout<< "hash_out 1: "  << (hash_out0[1] ^ hash_out1[1]) <<std::endl;

    // this part test batch dpf evaluation
    block* vec0 = new block[1 << key0.get_depth()];
    block* vec1 = new block[1 << key1.get_depth()];
    block proof0[2], proof1[2];
    std::cout<< "run: " << std::endl;
    dpf_batch_eval(0, &key0, vec0, proof0);
    dpf_batch_eval(1, &key1, vec1, proof1); 
    std::cout<< "proof: "  << (proof0[1] ^ proof1[1]) <<std::endl;


    // for(uint32_t i = 0; i < 1 << key0.get_depth(); i++) {
    //     std::cout<< "i = " << i << " : " << (vec0[i] ^ vec1[i]) << std::endl;  
    // }

    // this part for VDPF 


    delete[] buf0;
    delete[] buf1; 
    delete[] vec0;
    delete[] vec1;
    
    return 0;
}
