#ifndef OHV_HEADER
#define OHV_HEADER

#include <emp-tool/emp-tool.h>
#include <iostream>
#include <stdlib.h>

#include "fss.h"
/*
In this file, a dealer directly distributes one-hot vector between the parties (instead of using DPF).
*/

typedef struct OHV
{
    uint32_t dim_ohv;
    uint32_t num_ohv;
    std::vector<uint32_t> rdx_shares;
    std::vector<block> ohv_shares;

    OHV(uint32_t dim, uint32_t num)
    {
        dim_ohv = dim;
        num_ohv = num;
    }

    uint32_t get_buf_size()
    {
        uint32_t size = sizeof(uint32_t) * (num_ohv + 2) + sizeof(block) * num_ohv;
        return size;
    }

    void serialization(char *buf)
    {
        size_t count = 0;
        memcpy(buf + count, &this->dim_ohv, sizeof(uint32_t));
        count += sizeof(uint32_t);
        memcpy(buf + count, &this->num_ohv, sizeof(uint32_t));
        count += sizeof(uint32_t);

        for (size_t i = 0; i < this->num_ohv; i++)
        {
            memcpy(buf + count, &this->rdx_shares[i], sizeof(uint32_t));
            count += sizeof(uint32_t);
            memcpy(buf + count, &this->ohv_shares[i], sizeof(block));
            count += sizeof(block);
        }
    }

    void deserialization(char *buf)
    {
        size_t count = 0;
        memcpy(&this->dim_ohv, buf + count, sizeof(uint32_t));
        count += sizeof(uint32_t);
        memcpy(&this->num_ohv, buf + count, sizeof(uint32_t));
        count += sizeof(uint32_t);

        uint32_t tmp_int;
        block tmp_block;
        this->rdx_shares.resize(0);
        this->ohv_shares.resize(0);
        for (size_t i = 0; i < this->num_ohv; i++)
        {
            memcpy(&tmp_int, buf + count, sizeof(uint32_t));
            this->rdx_shares.push_back(tmp_int);
            count += sizeof(uint32_t);

            memcpy(&tmp_block, buf + count, sizeof(block));
            this->ohv_shares.push_back(tmp_block);
            count += sizeof(block);
        }
    }

    ~OHV() {}
} OHV;

// NOTE: dimenion should be power-of-2
uint32_t get_rdx_and_shares(PRG *prg, uint32_t dimension, uint32_t *share0, uint32_t *share1)
{
    uint32_t rdx;

    prg->random_data(&rdx, sizeof(uint32_t));
    prg->random_data(share0, sizeof(uint32_t));
    // std::cout << "get_rdx_and_shares() 83" << std::endl;
    rdx = rdx % dimension;
    *share0 = *share0 % dimension;
    *share1 = *share0 ^ rdx;
    return rdx;
}

void get_ohv_shares(PRG *prg, uint32_t rdx, block *share0, block *share1)
{

    block real = zero_block;
    set_bit(real, rdx);
    prg->random_block(share0, 1);
    *share1 = *share0 ^ real;
}

void generate_correlated_OHV(OHV *ohv0, OHV *ohv1)
{
    PRG prg;
    // std::cout << "generate_correlated_OHV() 100" << std::endl;
    ohv0->rdx_shares.resize(0);
    ohv0->ohv_shares.resize(0);
    ohv1->rdx_shares.resize(0);
    ohv1->ohv_shares.resize(0);
    // std::cout << "generate_correlated_OHV() 104" << std::endl;
    uint32_t rdx, rdx_share0, rdx_share1;
    block blk_share0, blk_share1;
    for (size_t i = 0; i < ohv0->num_ohv; i++)
    {
        // std::cout << "generate_correlated_OHV() 110 : " << i << std::endl;
        // std::cout << i << std::endl;
        rdx = get_rdx_and_shares(&prg, ohv0->dim_ohv, &rdx_share0, &rdx_share1);
        // std::cout << "generate_correlated_OHV() 114 : " << i << std::endl;
        ohv0->rdx_shares.push_back(rdx_share0);
        ohv1->rdx_shares.push_back(rdx_share1);

        get_ohv_shares(&prg, rdx, &blk_share0, &blk_share1);
        ohv0->ohv_shares.push_back(blk_share0);
        ohv1->ohv_shares.push_back(blk_share1);
        // std::cout << "generate_correlated_OHV() 120 : " << i << std::endl;
    }
}

#endif