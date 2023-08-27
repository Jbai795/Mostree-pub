#ifndef TREE_FSS
#define TREE_FSS

#define VERIFIABLE_DPF

#include <emp-tool/emp-tool.h>
#include <iostream>
#include <stdlib.h>

#include "twokeyprp.h"

using namespace emp;

typedef struct DPFKEY
{
    uint32_t depth;
    block s;
    bool t;
    block *s_CW;
    bool *t_L_CW;
    bool *t_R_CW;
    block hash_CW[2];

    DPFKEY(uint32_t bit_len)
    {
        depth = bit_len;
        s_CW = new block[depth + 1];
        t_L_CW = new bool[depth];
        t_R_CW = new bool[depth];
    }

    uint32_t get_depth()
    {
        return depth;
    }

    block get_s() { return s; }
    void set_s(const block v)
    {
        s = v;
    }

    bool get_t() { return t; }
    void set_t(const bool v)
    {
        t = v;
    }

    // i \in [1,...,depth+1]
    block get_s_CW(uint32_t i)
    {
        return s_CW[i - 1];
    }

    void set_s_CW(const uint32_t i, const block w)
    {
        s_CW[i - 1] = w;
    }

    // i \in [1,...,depth]
    bool get_t_L_CW(const uint32_t i)
    {
        return t_L_CW[i - 1];
    }
    void set_t_L_CW(const uint32_t i, const bool t)
    {
        t_L_CW[i - 1] = t;
    }

    // i \in [1,...,depth]
    bool get_t_R_CW(uint32_t i)
    {
        return t_R_CW[i - 1];
    }
    void set_t_R_CW(const uint32_t i, const bool t)
    {
        t_R_CW[i - 1] = t;
    }

    void set_proof(block *gen_proof)
    {
        hash_CW[0] = gen_proof[0];
        hash_CW[1] = gen_proof[1];
    }

    block get_proof(block *ret_proof)
    {
        ret_proof[0] = hash_CW[0];
        ret_proof[1] = hash_CW[1];
    }

    uint32_t get_buf_size()
    {
        uint32_t dpf_key_size = sizeof(uint32_t) + sizeof(block) * (depth + 2) + sizeof(bool) * (2 * depth + 1);
#ifdef VERIFIABLE_DPF
        dpf_key_size += 2 * sizeof(block);
#endif
        return dpf_key_size;
    }
    ~DPFKEY()
    {
        delete[] s_CW;
        delete[] t_L_CW;
        delete[] t_R_CW;
    }
} DPFKEY;

void serialization(DPFKEY *key, char *buf)
{
    // from key to buf
    uint32_t buf_len = key->get_buf_size();
    uint32_t count = 0;
    memcpy(buf + count, &key->depth, sizeof(uint32_t));
    count += sizeof(uint32_t);
    memcpy(buf + count, &key->s, sizeof(block));
    count += sizeof(block);
    memcpy(buf + count, &key->t, sizeof(key->t));
    count += sizeof(bool);
    memcpy(buf + count, key->s_CW, (key->depth + 1) * sizeof(block));
    count += (key->depth + 1) * sizeof(block);
    memcpy(buf + count, key->t_L_CW, sizeof(bool) * key->depth);
    count += sizeof(bool) * key->depth;
    memcpy(buf + count, key->t_R_CW, sizeof(bool) * key->depth);
    count += sizeof(bool) * key->depth;

#ifdef VERIFIABLE_DPF
    memcpy(buf + count, &key->hash_CW, 2 * sizeof(block));
#endif
}

void deserialization(const char *buf, DPFKEY *key)
{
    // from buf to key
    uint32_t count = 0;
    memcpy(&key->depth, buf + count, sizeof(uint32_t));
    count += sizeof(uint32_t);
    memcpy(&key->s, buf + count, sizeof(block));
    count += sizeof(block);
    memcpy(&key->t, buf + count, sizeof(key->t));
    count += sizeof(bool);
    memcpy(key->s_CW, buf + count, (key->depth + 1) * sizeof(block));
    count += (key->depth + 1) * sizeof(block);
    memcpy(key->t_L_CW, buf + count, sizeof(bool) * key->depth);
    count += sizeof(bool) * key->depth;
    memcpy(key->t_R_CW, buf + count, sizeof(bool) * key->depth);
    count += sizeof(bool) * key->depth;

#ifdef VERIFIABLE_DPF
    memcpy(&key->hash_CW, buf + count, 2 * sizeof(block));
#endif
}

bool get_bit(uint32_t rdx, uint len, uint32_t i)
{
    // i \in [1,2...,len]
    uint32_t mask = 1L << (len - i);
    uint32_t result = rdx & mask;
    return (result == 0) ? false : true;
}

void gen_dpf_key_pair(const uint32_t rdx, const uint32_t len, const block beta, DPFKEY *key0, DPFKEY *key1)
{
    PRG prg;
    block s0, s1;
    bool t0, t1;
    prg.random_block(&s0, 1);
    prg.random_block(&s1, 1);
    prg.random_bool(&t0, 1);
    t1 = t0 ^ true;

    key0->set_s(s0);
    key0->set_t(t0);
    key1->set_s(s1);
    key1->set_t(t1);

    TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));

    for (uint32_t i = 1; i <= len; i++)
    {
        block children0[2], children1[2];
        prp->node_expand_1to2(children0, s0);
        prp->node_expand_1to2(children1, s1);

        bool t_L_0 = getLSB(children0[0]);
        bool t_R_0 = getLSB(children0[1]);
        bool t_L_1 = getLSB(children1[0]);
        bool t_R_1 = getLSB(children1[1]);

        bool alpha = get_bit(rdx, len, i);
        block s_CW = children0[1 - alpha] ^ children1[1 - alpha];

        bool t_L_CW = t_L_0 ^ t_L_1 ^ alpha ^ true;
        bool t_R_CW = t_R_0 ^ t_R_1 ^ alpha;

        // set CW = s_CW || t_L_CW || t_R_CW
        key0->set_s_CW(i, s_CW);
        key1->set_s_CW(i, s_CW);
        key0->set_t_L_CW(i, t_L_CW);
        key1->set_t_L_CW(i, t_L_CW);
        key0->set_t_R_CW(i, t_R_CW);
        key1->set_t_R_CW(i, t_R_CW);

        block s_keep_0 = children0[alpha];
        block s_keep_1 = children1[alpha];
        bool t_keep_0 = (alpha == false ? t_L_0 : t_R_0);
        bool t_keep_1 = (alpha == false ? t_L_1 : t_R_1);
        bool t_keep_CW = (alpha == false ? t_L_CW : t_R_CW);

        s0 = s_keep_0 ^ (t0 == true ? s_CW : zero_block);
        s1 = s_keep_1 ^ (t1 == true ? s_CW : zero_block);

        t0 = t_keep_0 ^ (t0 & t_keep_CW);
        t1 = t_keep_1 ^ (t1 & t_keep_CW);
    }
    delete prp; 

    block CW = beta ^ s0 ^ s1; // NOTE: This only works for F_{2^k}
    key0->set_s_CW(len + 1, CW);
    key1->set_s_CW(len + 1, CW);
    

#ifdef VERIFIABLE_DPF
    Hash hash;
    block data[2], digest[2];
    data[0] = makeBlock(0, rdx);
    data[1] = s0;                                           // rdx || s0
    hash.hash_once(key0->hash_CW, data, sizeof(block) * 2); // P0's hash
    data[1] = s1;                                           // rdx || s1
    hash.hash_once(digest, data, sizeof(block) * 2);        // p1's hash
    key0->hash_CW[0] ^= digest[0];
    key0->hash_CW[1] ^= digest[1];

    key1->hash_CW[0] = key0->hash_CW[0];
    key1->hash_CW[1] = key0->hash_CW[1];
#endif
}

block dpf_eval(uint32_t party, DPFKEY *key, uint32_t idx, block *hash_out = nullptr)
{
    TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));

    block s = key->get_s();
    bool t = key->get_t();

    block s_CW;
    bool t_L_CW, t_R_CW;
    for (uint32_t i = 1; i <= key->depth; i++)
    {
        s_CW = key->get_s_CW(i);
        t_L_CW = key->get_t_L_CW(i);
        t_R_CW = key->get_t_R_CW(i);

        block children[2];
        prp->node_expand_1to2(children, s);

        block s_L, s_R;
        bool t_L, t_R;
        s_L = (t == true ? children[0] ^ s_CW : children[0]);
        s_R = (t == true ? children[1] ^ s_CW : children[1]);
        t_L = getLSB(children[0]) ^ (t & t_L_CW);
        t_R = getLSB(children[1]) ^ (t & t_R_CW);
        bool bit = get_bit(idx, key->get_depth(), i);
        s = (bit == true ? s_R : s_L);
        t = (bit == true ? t_R : t_L);
    }
    delete prp; 

#ifdef VERIFIABLE_DPF
    Hash hash;
    block data[2], s_digest[2];
    data[0] = makeBlock(0, idx);
    data[1] = s;
    if (hash_out != nullptr)
    {
        hash.hash_once(s_digest, data, 2 * sizeof(block));
        hash_out[0] = (t == true ? s_digest[0] ^ key->hash_CW[0] : s_digest[0]);
        hash_out[1] = (t == true ? s_digest[1] ^ key->hash_CW[1] : s_digest[1]);
    }
#endif
    return (t == true ? s ^ key->get_s_CW(key->depth + 1) : s); // NOTE: this only works for F_{2^k}
}

// vec lenth = 1 << key->get_depth()
// block dpf_batch_eval(uint32_t party, DPFKEY *key, block *vec)
// {
//     TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));
//     // uint32_t len = 1 << key->get_depth();

//     bool *t_vec = new bool[1 << key->get_depth()];

//     block s = key->get_s();
//     bool t = key->get_t();
//     vec[0] = s;
//     t_vec[0] = t;

//     for (uint32_t layer = 1; layer <= key->get_depth(); layer++)
//     {
//         uint32_t last_layer_size = 1 << (layer - 1);
//         for (int32_t i = last_layer_size - 1; i >= 0; i--)
//         {
//             prp->node_expand_1to2(&vec[2 * i], vec[i]);

//             block s_CW = key->get_s_CW(layer);
//             bool t_L_CW = key->get_t_L_CW(layer);
//             bool t_R_CW = key->get_t_R_CW(layer);

//             block s_L, s_R;
//             bool t_L, t_R;
//             s_L = (t_vec[i] == true ? vec[2 * i] ^ s_CW : vec[2 * i]);
//             s_R = (t_vec[i] == true ? vec[2 * i + 1] ^ s_CW : vec[2 * i + 1]);
//             t_L = getLSB(vec[2 * i]) ^ (t_vec[i] & t_L_CW);
//             t_R = getLSB(vec[2 * i + 1]) ^ (t_vec[i] & t_R_CW);

//             vec[2 * i] = s_L;
//             vec[2 * i + 1] = s_R;
//             t_vec[2 * i] = t_L;
//             t_vec[2 * i + 1] = t_R;
//         }
//     }

//     for (uint32_t i = 0; i < 1 << key->get_depth(); i++)
//     {
//         if (t_vec[i] == true)
//         {
//             vec[i] ^= key->get_s_CW(key->depth + 1);
//         }
//     }

//     delete[] t_vec;
// }

// vec lenth = 1 << key->get_depth()
void dpf_batch_eval(uint32_t party, DPFKEY *key, block *vec, block *proof = nullptr)
{
    TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));
    // uint32_t len = 1 << key->get_depth();

    bool *t_vec = new bool[1 << key->get_depth()];

    block s = key->get_s();
    bool t = key->get_t();
    vec[0] = s;
    t_vec[0] = t;

    for (uint32_t layer = 1; layer <= key->get_depth(); layer++)
    {
        uint32_t last_layer_size = 1 << (layer - 1);
        for (int32_t i = last_layer_size - 1; i >= 0; i--)
        {
            prp->node_expand_1to2(&vec[2 * i], vec[i]);

            block s_CW = key->get_s_CW(layer);
            bool t_L_CW = key->get_t_L_CW(layer);
            bool t_R_CW = key->get_t_R_CW(layer);

            block s_L, s_R;
            bool t_L, t_R;
            s_L = (t_vec[i] == true ? vec[2 * i] ^ s_CW : vec[2 * i]);
            s_R = (t_vec[i] == true ? vec[2 * i + 1] ^ s_CW : vec[2 * i + 1]);
            t_L = getLSB(vec[2 * i]) ^ (t_vec[i] & t_L_CW);
            t_R = getLSB(vec[2 * i + 1]) ^ (t_vec[i] & t_R_CW);

            vec[2 * i] = s_L;
            vec[2 * i + 1] = s_R;
            t_vec[2 * i] = t_L;
            t_vec[2 * i + 1] = t_R;
        }
    }

#ifdef VERIFIABLE_DPF
    block *proof_vec = new block[2 * 1 << key->get_depth()];
    // memset(proof_vec, 0, 2 * 1 << key->get_depth());
    Hash hash;
    block tag_leaf[2], s_digest[2];
#endif

    for (uint32_t i = 0; i < 1 << key->get_depth(); i++)
    {
#ifdef VERIFIABLE_DPF
        tag_leaf[0] = makeBlock(0, i);
        tag_leaf[1] = vec[i];

        hash.hash_once(s_digest, tag_leaf, 2 * sizeof(block));
        proof_vec[2 * i] = (t_vec[i] == true ? s_digest[0] ^ key->hash_CW[0] : s_digest[0]);             // the first half of hash tag for leaf i
        proof_vec[2 * i + 1] = (t_vec[i] == true ? s_digest[1] ^ key->hash_CW[1] : s_digest[1]); // the second half of hash tag for leaf i
#endif
        if (t_vec[i] == true)
        {
            vec[i] ^= key->get_s_CW(key->depth + 1);
        }
    }

#ifdef VERIFIABLE_DPF
    if (proof != nullptr)
        hash.hash_once(proof, proof_vec, 2 * 1 << key->get_depth());
    delete[] proof_vec;
#endif
    delete prp; 
    delete[] t_vec;
}

#endif