#include "dpf_dealer.h"
#include "ohv_dealer.h"

#include "aby3/Common/logging.h"
#include <cryptoTools/Common/CLP.h>

#include <chrono>
#include <thread>
using namespace std::chrono_literals;

int offline_benchmark(oc::CLP &cmd, uint64_t party, NetIO *io_pre, NetIO *io_next)
{
    const char *filename[7] = {
        "wine",
        "breast",
        "digits",
        "spambase",
        "diabetes",
        "boston",
        "MINIST"};

    uint64_t tree_depth[7] = {5, 7, 15, 17, 28, 30, 20};
    uint64_t tree_nodes[7] = {23, 43, 337, 171, 787, 851, 4179};

    uint64_t modelid = 2;

    // `-i id` will set `modelid=id`
    // `-s` will set `SCALABILITY = 1`
    // `-m` will set `MALICIOUS = 1`
    if (cmd.isSet("i") == true)
    {
        modelid = cmd.get<int>("i");
    }
    if (modelid < 0 || modelid > 6)
    {
        LOG(ERROR) << "* Please give a correct model id: [0, 6]";
        return 0;
    }

    uint32_t depth = tree_depth[modelid];
    uint32_t dimension = tree_nodes[modelid];

    bool valid;
    auto t1 = clock_start();

    if (dimension <= 128)
    { // use one-hot-vector preprocessing
        OHVlayer player(party, depth, dimension, io_pre, io_next);
        valid = player.OHV_gen_send_recv();
    }
    else
    { // use DPF preprocessing
        uint32_t bits = ceil(log2(dimension));
        // run batch DPF generation
        DPFPlayer player(party, depth, bits, io_pre, io_next);

        valid = player.DPF_gen_send_recv();
    }
    if (party == 0)
    {
        LOG(INFO) << "Offline benchmark for Model: " << filename[modelid];
        LOG(INFO) << "Party " << party << " preprocessing time: " << time_from(t1) << " microseconds";
        LOG(INFO) << "Party " << party << " sent data: " << (io_next->counter + io_pre->counter) << " bytes";
    }

    if (!valid)
    {
        std::cout << "Check fails!" << std::endl;
        return -1;
    }

    return 0;
}

int offline_3pc(oc::CLP &cmd)
{

    std::vector<std::thread> thrds;
    for (uint64_t i = 0; i < 3; ++i)
    {
        std::this_thread::sleep_for((10ms));
        if (cmd.isSet("p") == false || cmd.get<int>("p") == i)
        {
            thrds.emplace_back(std::thread([i, &cmd]()
                                           {
                // std::cout<< "thread: " << i <<std::endl;
                
                // initilize network
                NetIO *io_pre, *io_next; 
                init_network(io_pre, io_next, i, 1212);

                // run benchmark
                offline_benchmark(cmd, i, io_pre, io_next); 

                delete io_pre;
                delete io_next; }));
        }
    }

    for (auto &t : thrds)
        t.join();

    return 0;
}

void help()
{
    std::cerr << "+ offline_batch_gen Command Help" << std::endl;
    std::cerr << "01) -offline [-p] [0 1 2] [-i modelid]   ~~ to perform offline preprocessing on party [-p n]" << std::endl;
}

int main(int argc, char **argv)
{

    try
    {
        bool set = false;
        oc::CLP cmd(argc, argv);

        if (cmd.isSet("offline"))
        {
            set = true;
            offline_3pc(cmd);
        }

        if (set == false)
        {
            help();
        }
    }
    catch (std::exception &e)
    {
        LOG(INFO) << e.what() << std::endl;
    }

    return 0;
}
