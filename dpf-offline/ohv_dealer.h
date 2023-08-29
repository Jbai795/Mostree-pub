#ifndef DPF_DEALER
#define DPF_DEALER

#include <emp-tool/emp-tool.h>
#include <iostream>
#include <stdlib.h>

#include "ohv.h"
/*
In this file, a dealer directly distributes one-hot vector between the parties (instead of using DPF).
*/
class OHVlayer
{
public:
    int32_t party;
    NetIO *io_pre, *io_next;
    u_int32_t num_ohv, dimension;

    std::vector<uint32_t> my_rdxs;

    std::vector<block> pre_vectors_shares, next_vector_shares;

    OHVlayer(uint32_t party_id, uint32_t num, uint32_t dim, NetIO *pre, NetIO *next)
    {
        party = party_id;
        num_ohv = num;
        dimension = dim;
        io_pre = pre;
        io_next = next;
    }
    ~OHVlayer() {}

    bool OHV_gen_send_recv()
    {
        OHV ohv0(this->dimension, this->num_ohv), ohv1(this->dimension, this->num_ohv);
        // std::cout << "OHV_gen_send_recv() 36" << std::endl;
        generate_correlated_OHV(&ohv0, &ohv1);

        char *ohv0_buf = new char[ohv0.get_buf_size()];
        char *ohv1_buf = new char[ohv1.get_buf_size()];

        ohv0.serialization(ohv0_buf);
        ohv1.serialization(ohv1_buf);

        io_pre->send_data(ohv0_buf, ohv0.get_buf_size());
        io_next->send_data(ohv1_buf, ohv1.get_buf_size());
        io_pre->flush();
        io_next->flush();
        // std::cout << "OHV_gen_send_recv() 47" << std::endl;
        char *ohv_pre_buf = new char[ohv0.get_buf_size()];
        char *ohv_next_buf = new char[ohv1.get_buf_size()];

        io_next->recv_data(ohv_next_buf, ohv0.get_buf_size());
        io_pre->recv_data(ohv_next_buf, ohv0.get_buf_size());
        // std::cout << "OHV_gen_send_recv() 53" << std::endl;
        delete[] ohv0_buf;
        delete[] ohv1_buf;
        delete[] ohv_pre_buf;
        delete[] ohv_next_buf;

        return true;
    }

    void extract()
    {
        // TODO:
    }
};

#endif