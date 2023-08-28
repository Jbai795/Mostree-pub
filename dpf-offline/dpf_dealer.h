#ifndef DPF_DEALER
#define DPF_DEALER

#include <emp-tool/emp-tool.h>
#include <iostream>
#include <stdlib.h>

#include "fss.h"

class DPFPlayer
{
public:
    int32_t party;
    uint32_t num_dpf, num_bits;
    NetIO *io_pre, *io_next;

    std::vector<uint32_t> my_rdx_vec, pre_rdx_share_vec, next_rdx_share_vec;

    char *rev_buf_pre, *rev_buf_next;
    block *rev_vec_pre, *rev_vec_next;

    DPFPlayer(uint32_t party_id, uint32_t num, uint32_t n_bits, NetIO *pre, NetIO *next)
    {
        party = party_id;
        num_dpf = num;
        num_bits = n_bits;
        io_pre = pre;
        io_next = next;

        DPFKEY key(n_bits);
        rev_buf_pre = new char[num_dpf * key.get_buf_size()];
        rev_buf_next = new char[num_dpf * key.get_buf_size()];
        rev_vec_pre = new block[num_dpf * (1 << (n_bits - 7))];
        rev_vec_next = new block[num_dpf * (1 << (n_bits - 7))];
    }

    ~DPFPlayer()
    {
        delete[] rev_buf_pre;
        delete[] rev_buf_next;
        delete[] rev_vec_pre;
        delete[] rev_vec_next;
    }

    // uint32_t determine(uint32_t rdx, uint32_t depth, block *msg)
    // {
    //     uint32_t value = rdx % 256;
    //     *msg = zero_block;
    //     set_bit(*msg, value);
    //     return (rdx >> 8);
    // }

    bool DPF_gen_send_recv()
    {
        bool valid = true;
        DPFKEY key0(num_bits), key1(num_bits);

        uint32_t buf_size = key0.get_buf_size();
        char *send_buf_pre = new char[num_dpf * buf_size];
        char *send_buf_next = new char[num_dpf * buf_size];

        PRG prg;
        for (uint32_t i = 0; i < num_dpf; i++)
        {
            uint32_t rdx = gen_dpf_key_pair_with_index_share(num_bits, &key0, &key1);

            my_rdx_vec.push_back(rdx); // store the rdx locally

            // store key to buf
            serialization(&key0, &send_buf_pre[i * buf_size]);
            serialization(&key1, &send_buf_next[i * buf_size]);
        }
        io_pre->send_data(send_buf_pre, num_dpf * buf_size);
        io_pre->flush();
        io_next->send_data(send_buf_next, num_dpf * buf_size);
        io_next->flush();
        // std::cout << "sent" <<std::endl;
        io_pre->recv_data(rev_buf_pre, num_dpf * buf_size);
        io_next->recv_data(rev_buf_next, num_dpf * buf_size);

        // this step do DPF extraction and check (for the malicious setting)
        valid = extract_dpf();

        delete[] send_buf_pre;
        delete[] send_buf_next;

        return valid;
    }

    bool extract_dpf()
    {
        bool valid = true;
#ifdef VERIFIABLE_DPF
        block pre_proof[2], next_proof[2];
        memset(pre_proof, 0, sizeof(block) * 2);
        memset(next_proof, 0, sizeof(block) * 2);
        Hash hash;
#endif
        for (uint32_t i = 0; i < num_dpf; i++)
        {

            DPFKEY key_pre(this->num_bits), key_next(this->num_bits);

            deserialization(&this->rev_buf_pre[i * key_pre.get_buf_size()], &key_pre);
            deserialization(&this->rev_buf_next[i * key_next.get_buf_size()], &key_next);

#ifdef VERIFIABLE_DPF
            block pre_proof_tmp[2], next_proof_tmp[2];
            dpf_batch_eval(party, &key_pre, this->rev_vec_pre, pre_proof_tmp);
            dpf_batch_eval(party, &key_next, this->rev_vec_next, next_proof_tmp);

            block digest[2];
            pre_proof_tmp[0] ^= pre_proof[0];
            pre_proof_tmp[1] ^= pre_proof[1];
            hash.hash_once(digest, pre_proof_tmp, sizeof(block) * 2);
            pre_proof[0] ^= digest[0];
            pre_proof[1] ^= digest[1];

            next_proof_tmp[0] ^= next_proof[0];
            next_proof_tmp[1] ^= next_proof[1];
            hash.hash_once(digest, next_proof_tmp, sizeof(block) * 2);
            next_proof[0] ^= digest[0];
            next_proof[1] ^= digest[1];
#else
            dpf_batch_eval(party, &key_pre, this->rev_vec_pre);
            dpf_batch_eval(party, &key_next, this->rev_vec_next);
#endif
            this->pre_rdx_share_vec.push_back(key_pre.get_share());
            this->next_rdx_share_vec.push_back(key_next.get_share());
        }

#ifdef VERIFIABLE_DPF
        block hash_tag_for_next[2], hash_tag_from_pre[2];
        hash.hash_once(hash_tag_for_next, pre_proof, sizeof(block) * 2);
        // send hash(pre_proof) --> next party
        io_next->send_block(hash_tag_for_next, 2);
        io_next->flush();

        // receive hash tag --> this party
        io_pre->recv_block(hash_tag_from_pre, 2);

        // check whether H(next_proof) == hash_tag_from_pre
        block correct_hash_tag_from_pre[2];
        hash.hash_once(correct_hash_tag_from_pre, next_proof, sizeof(block) * 2);
        bool equal = cmpBlock(correct_hash_tag_from_pre, hash_tag_from_pre, 2);
        if (!equal)
        {
            std::cout << "Previous tag NOT EQUAL!" << std::endl;
            valid = false;
        }

        // send next_proof --> previous party
        io_pre->send_block(next_proof, 2);
        io_pre->flush();
        
        // receve proof from next party
        block rev_next_proof[2];
        io_next->recv_block(rev_next_proof, 2);
        equal = cmpBlock(pre_proof, rev_next_proof, 2);
        if (!equal)
        {
            std::cout << "Next proof NOT EQUAL!" << std::endl;
            valid = false;
        }
#endif

        return valid;
    }
};

#endif