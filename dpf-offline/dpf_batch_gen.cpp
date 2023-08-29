#include "dpf_dealer.h"
// #include "tree-doram/client.h"
// #include "tree-doram/server.h"

// DO USE & in order to asign new value
void init_network_dpf(NetIO *&io_pre, NetIO *&io_next, int party, int port)
{
    // this is for client
    if (party == 0)
    {
        io_pre = new NetIO(nullptr, port);          // p2 <-> p0 server
        io_next = new NetIO("127.0.0.1", port + 1); // p0 <-> p1 client
    }
    if (party == 1)
    {
        io_pre = new NetIO(nullptr, port + 1);      // p0 <-> p1 server
        io_next = new NetIO("127.0.0.1", port + 2); // p1 <-> p2 client
    }
    if (party == 2)
    {
        io_next = new NetIO("127.0.0.1", port); // p2 <-> p0 client
        io_pre = new NetIO(nullptr, port + 2);  // p1 <-> p2 server
    }
}

int main(int argc, char **argv)
{
    /*
        uint32_t len = 13;
        DPFKEY key0(len), key1(len);

        uint32_t rdx = 1;

        // for(uint32_t i = 1; i <= 32; i++) {
        //     bool bit = get_bit(rdx, 32, i);
        //     std::cout<< bit <<std::endl;
        // }

        // bool a =  true, b = false;
        // std::cout<< (a ^ b) << std::endl;

        block beta = makeBlock(0, 0xFFFF);
        gen_dpf_key_pair_with_index_share(len, &key0, &key1);

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

        delete[] buf0;
        delete[] buf1;
        delete[] vec0;
        delete[] vec1;
    */

    // run Dealer batch DPF generation
    int port, party;
    parse_party_and_port(argv, &party, &port);
    NetIO *io_pre, *io_next;
    init_network_dpf(io_pre, io_next, party, port);
    // std::cout << "io setted up" << std::endl;

    uint32_t depth = atoi(argv[3]);
    uint32_t dimension = atoi(argv[4]);
    if (dimension <= 128)
    {
        std::cout << "NOTE: dimension > 128" << std::endl;
        return 0;
    }

    uint32_t bits = ceil(log2(dimension+1));

    // run batch DPF generation 
    DPFPlayer player(party, depth, bits, io_pre, io_next);

    auto t1 = clock_start();
    bool valid = player.DPF_gen_send_recv();
    if(party == 0){
        std::cout<< "[" << depth << ", " << bits << "] " << "Times: " << time_from(t1)<< " microseconds"<<std::endl;
        std::cout<< "[" << depth << ", " << bits << "] " << "Sent data: " << (io_next->counter + io_pre->counter) << " bytes" <<std::endl;
    }

    if (!valid)
    {
        std::cout << "Check fails!" << std::endl;
        return 0;
    }

    delete io_pre;
    delete io_next;

    // std::cout << "ALL GOD :)" << std::endl;

    return 0;
}
