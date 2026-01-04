#ifndef TREE_FSS
#define TREE_FSS

#define VERIFIABLE_DPF

#include <emp-tool/emp-tool.h>
#include <iostream>
#include <stdlib.h>

#include "twokeyprp.h"

using DPFBlock = emp::block;

typedef struct DPFKEY
{
    uint32_t depth;
    uint32_t rdx_share;
    DPFBlock s;
    bool t;
    DPFBlock *s_CW;
    bool *t_L_CW;
    bool *t_R_CW;
    DPFBlock hash_CW[2];

    DPFKEY(uint32_t bit_len)
    {
        // depth = bit_len - 7; // NOTE: log2(128) left for early termination
        depth = bit_len; // NOTE: log2(128) left for early termination

        // Allocate control words for all levels plus two outputs.
        // Single-output uses depth+1; two-output variant needs depth+2.
        s_CW = new DPFBlock[depth + 2];
        t_L_CW = new bool[depth];
        t_R_CW = new bool[depth];
    }

    void set_share(uint32_t share)
    {
        rdx_share = share;
    }

    uint32_t get_share()
    {
        return rdx_share;
    }

    uint32_t get_depth()
    {
        return depth;
    }

    DPFBlock get_s() { return s; }
    void set_s(const DPFBlock v)
    {
        s = v;
    }

    bool get_t() { return t; }
    void set_t(const bool v)
    {
        t = v;
    }

    // i \in [1,...,depth+1]
    DPFBlock get_s_CW(uint32_t i)
    {
        return s_CW[i - 1];
    }

    void set_s_CW(const uint32_t i, const DPFBlock w)
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

    void set_proof(DPFBlock *gen_proof)
    {
        hash_CW[0] = gen_proof[0];
        hash_CW[1] = gen_proof[1];
    }

    void get_proof(DPFBlock *ret_proof)
    {
        ret_proof[0] = hash_CW[0];
        ret_proof[1] = hash_CW[1];
    }

    uint32_t get_buf_size()
    {
        // Layout in serialization:
        // depth (u32) + rdx_share (u32) + s (block)
        // + s_CW[depth + 2] (blocks)
        // + t (bool) + t_L_CW[depth] (bool) + t_R_CW[depth] (bool)
        // + hash_CW[2] (blocks) if VERIFIABLE_DPF
        uint32_t dpf_key_size = 2 * sizeof(uint32_t)
                      + sizeof(DPFBlock) * (depth + 3)  // s + s_CW[depth+2]
                      + sizeof(bool) * (2 * depth + 1);
#ifdef VERIFIABLE_DPF
        dpf_key_size += 2 * sizeof(DPFBlock);
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

// struct DpfEvalInt2 {
//     int64_t v1;
//     int64_t v2;
// };

struct DpfEvalInt2 {
    uint64_t v1;
    uint64_t v2;
};


void init_network(NetIO *&io_pre, NetIO *&io_next, int party, int port);
void serialization(DPFKEY *key, char *buf);
void deserialization(const char *buf, DPFKEY *key);
bool get_bit(uint32_t rdx, uint len, uint32_t i);
void gen_dpf_key_pair(const uint32_t real_rdx, const uint32_t real_len, const DPFBlock beta, DPFKEY *key0, DPFKEY *key1);
// new function to generate DPF keys for two different output values, for example, f(x)=(beta1, beta2) if x = alpha, otherwise, f(x)=(0,0)
void gen_dpf_key_pair_two(const uint32_t real_rdx, const uint32_t real_len, const DPFBlock beta1, const DPFBlock beta2, DPFKEY *key0, DPFKEY *key1);
uint32_t gen_dpf_key_pair_with_index_share(const uint32_t len, DPFKEY *key0, DPFKEY *key1);
DPFBlock dpf_eval(uint32_t party, DPFKEY *key, uint32_t idx, DPFBlock *hash_out = nullptr);
int64_t dpf_eval_int64(uint32_t party, DPFKEY *key, uint32_t idx, DPFBlock *hash_out = nullptr);
int dpf_eval_int(uint32_t party, DPFKEY *key, uint32_t idx, DPFBlock *hash_out = nullptr);
DpfEvalInt2 dpf_eval_int_two(uint32_t party, DPFKEY *key, uint32_t idx, DPFBlock *hash_out);
void dpf_batch_eval(uint32_t party, DPFKEY *key, DPFBlock *vec, DPFBlock *proof = nullptr);
void dpf_batch_eval_int(uint32_t party, DPFKEY *key, DPFBlock *vec, int *vec_int, DPFBlock *proof = nullptr);


// add by jianli
struct DPFKeyBlob {
    std::vector<uint8_t> bytes; // serialization() 的输出
};

struct DPFKeyPairBlobs {
    DPFKeyBlob toParty1; // 发给 party1
    DPFKeyBlob toParty2; // 发给 party2
};

// party0：生成两把 key，并序列化
DPFKeyPairBlobs DPFGen(uint32_t len, uint32_t rdx, DPFBlock beta);
// party0：生成两把 key，并序列化，编入了beta1和beta2，在mostree的增量工作中，其中beta2被看作MAC key
DPFKeyPairBlobs DPFGenWithMAC(uint32_t len, uint32_t rdx, emp::block beta1, emp::block beta2);

struct DPFKeysAndBlobs {
    std::unique_ptr<DPFKEY> keyToParty1;
    std::unique_ptr<DPFKEY> keyToParty2;
    decltype(DPFGen(0,0,emp::block{})) blobs; // 不想写类型就用 decltype 推导
};





// party1/2：反序列化并 eval（evalId 必须是 0 或 1）
int DPFEvalInt(uint32_t evalId01, uint32_t len,
               const DPFKeyBlob& blob, uint32_t x,
               DPFBlock hash_out[2]);

void debug_print_dpfkey(DPFKEY& key, const std::string& tag);

//for debugging: generate DPF keys and return both keys and their serialized blobs
DPFKeysAndBlobs DPFGenWithKeys(uint32_t len, uint32_t rdx, emp::block seed);


#endif