#pragma once
#include "dpf.h"

#include <stdexcept>
#include <memory>


DPFKeyPairBlobs DPFGen(uint32_t len, uint32_t rdx, emp::block beta)
{
    DPFKEY key0(len), key1(len);
    gen_dpf_key_pair(rdx, len, beta, &key0, &key1);

    // std::cout<< "DPFGen: generated DPF keys for rdx = " << rdx << ", len = " << len << std::endl;

    uint32_t buf_size = key0.get_buf_size();
    if (buf_size != key1.get_buf_size())
        throw std::runtime_error("DPFGen: key buf size mismatch");

    DPFKeyPairBlobs out;
    out.toParty1.bytes.resize(buf_size);
    out.toParty2.bytes.resize(buf_size);

    serialization(&key0, reinterpret_cast<char*>(out.toParty1.bytes.data()));
    serialization(&key1, reinterpret_cast<char*>(out.toParty2.bytes.data()));
    return out;
}

DPFKeyPairBlobs DPFGenWithMAC(uint32_t len, uint32_t rdx, emp::block beta1, emp::block beta2)
{
    DPFKEY key0(len), key1(len);
    gen_dpf_key_pair_two(rdx, len, beta1, beta2, &key0, &key1);

    // std::cout<< "DPFGen: generated DPF keys for rdx = " << rdx << ", len = " << len << std::endl;

    uint32_t buf_size = key0.get_buf_size();
    if (buf_size != key1.get_buf_size())
        throw std::runtime_error("DPFGen: key buf size mismatch");

    DPFKeyPairBlobs out;
    out.toParty1.bytes.resize(buf_size);
    out.toParty2.bytes.resize(buf_size);

    serialization(&key0, reinterpret_cast<char*>(out.toParty1.bytes.data()));
    serialization(&key1, reinterpret_cast<char*>(out.toParty2.bytes.data()));
    return out;
}

int DPFEvalInt(uint32_t evalId01, uint32_t len,
               const DPFKeyBlob& blob, uint32_t x,
               emp::block hash_out[2])
{
    if (evalId01 != 0 && evalId01 != 1)
        throw std::runtime_error("DPFEvalInt: evalId must be 0/1");

    // 关键：必须用 len 构造，保证内部数组长度正确
    DPFKEY key(len);
    
    deserialization(reinterpret_cast<const char*>(blob.bytes.data()), &key);

    return dpf_eval_int(evalId01, &key, x, hash_out);
}



void debug_print_dpfkey(DPFKEY& key, const std::string& tag)
{
    std::cout << "=== DPFKEY DEBUG (" << tag << ") ===\n";
    std::cout << "depth = " << key.get_depth() << "\n";
    std::cout << "rdx_share = " << key.get_share() << "\n";

    auto s = key.get_s();
    std::cout << "s.low64  = " << _mm_extract_epi64(s, 0) << "\n";
    std::cout << "s.high64 = " << _mm_extract_epi64(s, 1) << "\n";
    std::cout << "t        = " << key.get_t() << "\n";
    //std::min(2u, key.get_depth())
    for (uint32_t i = 1; i <= 9; i++) {
        auto scw = key.get_s_CW(i);
        std::cout << "level " << i << " s_CW.low64  = "
                  << _mm_extract_epi64(scw, 0) << "\n";
        std::cout << "level " << i << " s_CW.high64 = "
                  << _mm_extract_epi64(scw, 1) << "\n";
        std::cout << "level " << i << " t_L=" << key.get_t_L_CW(i)
                  << " t_R=" << key.get_t_R_CW(i) << "\n";
    }
}

DPFKeysAndBlobs DPFGenWithKeys(uint32_t len, uint32_t rdx, emp::block seed)
{
    // 1) 先用你现有的 DPFGen 生成要发送的 blobs
    auto blobs = DPFGen(len, rdx, seed);

    // 2) 反序列化出两把 key（用于 debug 打印）
    std::unique_ptr<DPFKEY> k1(new DPFKEY(len));
    std::unique_ptr<DPFKEY> k2(new DPFKEY(len));


    deserialization((char*)blobs.toParty1.bytes.data(), k1.get());
    deserialization((char*)blobs.toParty2.bytes.data(), k2.get());

    // 3) 返回（key + blobs）
    return { std::move(k1), std::move(k2), std::move(blobs) };
}




void init_network(NetIO *&io_pre, NetIO *&io_next, int party, int port)
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


void serialization(DPFKEY *key, char *buf)
{
    // from key to buf
    // uint32_t buf_len = key->get_buf_size();
    uint32_t count = 0;
    memcpy(buf + count, &key->depth, sizeof(uint32_t));
    count += sizeof(uint32_t);
    memcpy(buf + count, &key->rdx_share, sizeof(uint32_t));
    count += sizeof(uint32_t);
    memcpy(buf + count, &key->s, sizeof(DPFBlock));
    count += sizeof(DPFBlock);
    memcpy(buf + count, &key->t, sizeof(key->t));
    count += sizeof(bool);
    // Serialize s_CW for all levels plus up to two outputs
    memcpy(buf + count, key->s_CW, (key->depth + 2) * sizeof(DPFBlock));
    count += (key->depth + 2) * sizeof(DPFBlock);
    memcpy(buf + count, key->t_L_CW, sizeof(bool) * key->depth);
    count += sizeof(bool) * key->depth;
    memcpy(buf + count, key->t_R_CW, sizeof(bool) * key->depth);
    count += sizeof(bool) * key->depth;

#ifdef VERIFIABLE_DPF
    memcpy(buf + count, &key->hash_CW, 2 * sizeof(DPFBlock));
#endif
}

void deserialization(const char *buf, DPFKEY *key)
{
    // from buf to key
    uint32_t count = 0;
    memcpy(&key->depth, buf + count, sizeof(uint32_t));
    count += sizeof(uint32_t);
    memcpy(&key->rdx_share, buf + count, sizeof(uint32_t));
    count += sizeof(uint32_t);
    memcpy(&key->s, buf + count, sizeof(DPFBlock));
    count += sizeof(DPFBlock);
    memcpy(&key->t, buf + count, sizeof(key->t));
    count += sizeof(bool);
    // Deserialize s_CW for all levels plus up to two outputs
    memcpy(key->s_CW, buf + count, (key->depth + 2) * sizeof(DPFBlock));
    count += (key->depth + 2) * sizeof(DPFBlock);
    memcpy(key->t_L_CW, buf + count, sizeof(bool) * key->depth);
    count += sizeof(bool) * key->depth;
    memcpy(key->t_R_CW, buf + count, sizeof(bool) * key->depth);
    count += sizeof(bool) * key->depth;

#ifdef VERIFIABLE_DPF
    memcpy(&key->hash_CW, buf + count, 2 * sizeof(DPFBlock));
#endif
}

bool get_bit(uint32_t rdx, uint len, uint32_t i)
{
    // i \in [1,2...,len]
    uint32_t mask = 1L << (len - i);
    uint32_t result = rdx & mask;
    return (result == 0) ? false : true;
}

void gen_dpf_key_pair(const uint32_t real_rdx, const uint32_t real_len, const DPFBlock beta, DPFKEY *key0, DPFKEY *key1)
{
    PRG prg;
    DPFBlock s0, s1;
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

    for (uint32_t i = 1; i <= real_len; i++)
    {
        DPFBlock children0[2], children1[2];
        prp->node_expand_1to2(children0, s0);
        prp->node_expand_1to2(children1, s1);

        bool t_L_0 = getLSB(children0[0]);
        bool t_R_0 = getLSB(children0[1]);
        bool t_L_1 = getLSB(children1[0]);
        bool t_R_1 = getLSB(children1[1]);

        bool alpha = get_bit(real_rdx, real_len, i);
        DPFBlock s_CW = children0[1 - alpha] ^ children1[1 - alpha];

        bool t_L_CW = t_L_0 ^ t_L_1 ^ alpha ^ true;
        bool t_R_CW = t_R_0 ^ t_R_1 ^ alpha;

        // set CW = s_CW || t_L_CW || t_R_CW
        key0->set_s_CW(i, s_CW);
        key1->set_s_CW(i, s_CW);
        key0->set_t_L_CW(i, t_L_CW);
        key1->set_t_L_CW(i, t_L_CW);
        key0->set_t_R_CW(i, t_R_CW);
        key1->set_t_R_CW(i, t_R_CW);

        DPFBlock s_keep_0 = children0[alpha];
        DPFBlock s_keep_1 = children1[alpha];
        bool t_keep_0 = (alpha == false ? t_L_0 : t_R_0);
        bool t_keep_1 = (alpha == false ? t_L_1 : t_R_1);
        bool t_keep_CW = (alpha == false ? t_L_CW : t_R_CW);

        s0 = s_keep_0 ^ (t0 == true ? s_CW : zero_block);
        s1 = s_keep_1 ^ (t1 == true ? s_CW : zero_block);

        t0 = t_keep_0 ^ (t0 & t_keep_CW);
        t1 = t_keep_1 ^ (t1 & t_keep_CW);
    }
    

    // NOTE: The following part only works for F_{2^k}
    // DPFBlock CW = beta ^ s0 ^ s1; 
    // key0->set_s_CW(real_len + 1, CW);
    // key1->set_s_CW(real_len + 1, CW);

    // NOTE: The following part works for group
    DPFBlock children0[2], children1[2];
    prp->node_expand_1to2(children0, s0);
    prp->node_expand_1to2(children1, s1);
    DPFBlock CW = 1 - children0[0] + children1[0];
    if (t1) {
        CW *= -1;
    }
    key0->set_s_CW(real_len + 1, CW);
    key1->set_s_CW(real_len + 1, CW);

    delete prp;


#ifdef VERIFIABLE_DPF
    Hash hash;
    DPFBlock data[2], digest[2];
    data[0] = makeBlock(0, real_rdx);
    data[1] = s0;                                           // rdx || s0
    hash.hash_once(key0->hash_CW, data, sizeof(DPFBlock) * 2); // P0's hash
    data[1] = s1;                                           // rdx || s1
    hash.hash_once(digest, data, sizeof(DPFBlock) * 2);        // p1's hash
    key0->hash_CW[0] ^= digest[0];
    key0->hash_CW[1] ^= digest[1];

    key1->hash_CW[0] = key0->hash_CW[0];
    key1->hash_CW[1] = key0->hash_CW[1];
#endif
}



void gen_dpf_key_pair_two(const uint32_t real_rdx, const uint32_t real_len, const DPFBlock beta1, const DPFBlock beta2, DPFKEY *key0, DPFKEY *key1)
{
    PRG prg;
    DPFBlock s0, s1;
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

    for (uint32_t i = 1; i <= real_len; i++)
    {
        DPFBlock children0[2], children1[2];
        prp->node_expand_1to2(children0, s0);
        prp->node_expand_1to2(children1, s1);

        bool t_L_0 = getLSB(children0[0]);
        bool t_R_0 = getLSB(children0[1]);
        bool t_L_1 = getLSB(children1[0]);
        bool t_R_1 = getLSB(children1[1]);

        bool alpha = get_bit(real_rdx, real_len, i);
        DPFBlock s_CW = children0[1 - alpha] ^ children1[1 - alpha];

        bool t_L_CW = t_L_0 ^ t_L_1 ^ alpha ^ true;
        bool t_R_CW = t_R_0 ^ t_R_1 ^ alpha;

        // set CW = s_CW || t_L_CW || t_R_CW
        key0->set_s_CW(i, s_CW);
        key1->set_s_CW(i, s_CW);
        key0->set_t_L_CW(i, t_L_CW);
        key1->set_t_L_CW(i, t_L_CW);
        key0->set_t_R_CW(i, t_R_CW);
        key1->set_t_R_CW(i, t_R_CW);

        DPFBlock s_keep_0 = children0[alpha];
        DPFBlock s_keep_1 = children1[alpha];
        bool t_keep_0 = (alpha == false ? t_L_0 : t_R_0);
        bool t_keep_1 = (alpha == false ? t_L_1 : t_R_1);
        bool t_keep_CW = (alpha == false ? t_L_CW : t_R_CW);

        s0 = s_keep_0 ^ (t0 == true ? s_CW : zero_block);
        s1 = s_keep_1 ^ (t1 == true ? s_CW : zero_block);

        t0 = t_keep_0 ^ (t0 & t_keep_CW);
        t1 = t_keep_1 ^ (t1 & t_keep_CW);
    }
    

    // NOTE: The following part only works for F_{2^k}
    // DPFBlock CW = beta ^ s0 ^ s1; 
    // key0->set_s_CW(real_len + 1, CW);
    // key1->set_s_CW(real_len + 1, CW);

    // NOTE: The following part works for group
    DPFBlock children0[2], children1[2];
    prp->node_expand_1to2(children0, s0);
    prp->node_expand_1to2(children1, s1);
    DPFBlock CW1 = beta1 - children0[0] + children1[0];
    if (t1) {
        CW1 *= -1;
    }
    key0->set_s_CW(real_len + 1, CW1);
    key1->set_s_CW(real_len + 1, CW1);

    DPFBlock CW2 = beta2 - children0[1] + children1[1];
    if (t1) {
        CW2 *= -1;
    }
    key0->set_s_CW(real_len + 2, CW2);
    key1->set_s_CW(real_len + 2, CW2);

    delete prp;


#ifdef VERIFIABLE_DPF
    Hash hash;
    DPFBlock data[2], digest[2];
    data[0] = makeBlock(0, real_rdx);
    data[1] = s0;                                           // rdx || s0
    hash.hash_once(key0->hash_CW, data, sizeof(DPFBlock) * 2); // P0's hash
    data[1] = s1;                                           // rdx || s1
    hash.hash_once(digest, data, sizeof(DPFBlock) * 2);        // p1's hash
    key0->hash_CW[0] ^= digest[0];
    key0->hash_CW[1] ^= digest[1];

    key1->hash_CW[0] = key0->hash_CW[0];
    key1->hash_CW[1] = key0->hash_CW[1];
#endif
}

// NOTE: this function returns idx
uint32_t gen_dpf_key_pair_with_index_share(const uint32_t len, DPFKEY *key0, DPFKEY *key1)
{

    // gen rdx shares
    PRG prg;
    uint32_t rdx, rdx_share0, rdx_share1;
    prg.random_data(&rdx, sizeof(uint32_t));
    prg.random_data(&rdx_share0, sizeof(uint32_t));
    rdx = rdx % (1 << len);
    rdx_share0 = rdx_share0 % (1 << len);
    rdx_share1 = rdx_share0 ^ rdx;

    // gen dpf key
    key0->set_share(rdx_share0);
    key1->set_share(rdx_share1);

    uint32_t real_rdx = rdx >> 7; // log2(128) = 7
    DPFBlock beta = zero_block;
    set_bit(beta, rdx % 128);

    gen_dpf_key_pair(real_rdx, len - 7, beta, key0, key1);

    // return rdx
    return rdx;
}

DPFBlock dpf_eval(uint32_t party, DPFKEY *key, uint32_t idx, DPFBlock *hash_out)
{
    TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));

    DPFBlock s = key->get_s();
    bool t = key->get_t();

    DPFBlock s_CW;
    bool t_L_CW, t_R_CW;
    for (uint32_t i = 1; i <= key->depth; i++)
    {
        s_CW = key->get_s_CW(i);
        t_L_CW = key->get_t_L_CW(i);
        t_R_CW = key->get_t_R_CW(i);

        DPFBlock children[2];
        prp->node_expand_1to2(children, s);

        DPFBlock s_L, s_R;
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
    DPFBlock data[2], s_digest[2];
    data[0] = makeBlock(0, idx);
    data[1] = s;
    if (hash_out != nullptr)
    {
        hash.hash_once(s_digest, data, 2 * sizeof(DPFBlock));
        hash_out[0] = (t == true ? s_digest[0] ^ key->hash_CW[0] : s_digest[0]);
        hash_out[1] = (t == true ? s_digest[1] ^ key->hash_CW[1] : s_digest[1]);
    }
#endif

    // NOTE: this only works for F_{2^k}
    return (t == true ? s ^ key->get_s_CW(key->depth + 1) : s); 
}

int dpf_eval_int(uint32_t party, DPFKEY *key, uint32_t idx, DPFBlock *hash_out)
{
    TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));

    DPFBlock s = key->get_s();
    bool t = key->get_t();

    DPFBlock s_CW;
    bool t_L_CW, t_R_CW;
    for (uint32_t i = 1; i <= key->depth; i++)
    {
        s_CW = key->get_s_CW(i);
        t_L_CW = key->get_t_L_CW(i);
        t_R_CW = key->get_t_R_CW(i);

        DPFBlock children[2];
        prp->node_expand_1to2(children, s);

        DPFBlock s_L, s_R;
        bool t_L, t_R;
        s_L = (t == true ? children[0] ^ s_CW : children[0]);
        s_R = (t == true ? children[1] ^ s_CW : children[1]);
        t_L = getLSB(children[0]) ^ (t & t_L_CW);
        t_R = getLSB(children[1]) ^ (t & t_R_CW);
        bool bit = get_bit(idx, key->get_depth(), i);
        s = (bit == true ? s_R : s_L);
        t = (bit == true ? t_R : t_L);
    }
    

#ifdef VERIFIABLE_DPF
    Hash hash;
    DPFBlock data[2], s_digest[2];
    data[0] = makeBlock(0, idx);
    data[1] = s;
    if (hash_out != nullptr)
    {
        hash.hash_once(s_digest, data, 2 * sizeof(DPFBlock));
        hash_out[0] = (t == true ? s_digest[0] ^ key->hash_CW[0] : s_digest[0]);
        hash_out[1] = (t == true ? s_digest[1] ^ key->hash_CW[1] : s_digest[1]);
    }
#endif

    // NOTE: this only works for F_{2^k}
    // return (t == true ? s ^ key->get_s_CW(key->depth + 1) : s); 

    // NOTE: this works for group
    DPFBlock children[2];
    prp->node_expand_1to2(children, s);
    DPFBlock tmp = children[0];
    // DPFBlock tmp = ConvertDPFBlock(s);
    if (t) {
        tmp += key->get_s_CW(key->depth + 1);
    }

    int ans = _mm_extract_epi64(tmp, 0);
    int sign = (party==0)?1:-1;

    delete prp; //delete after using
    return sign*ans;
}

// int64_t dpf_eval_int_t(uint32_t party, DPFKEY *key, uint32_t idx, DPFBlock *hash_out)
// {
//     TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));

//     DPFBlock s = key->get_s();
//     bool t = key->get_t();

//     DPFBlock s_CW;
//     bool t_L_CW, t_R_CW;
//     for (uint32_t i = 1; i <= key->depth; i++)
//     {
//         s_CW = key->get_s_CW(i);
//         t_L_CW = key->get_t_L_CW(i);
//         t_R_CW = key->get_t_R_CW(i);

//         DPFBlock children[2];
//         prp->node_expand_1to2(children, s);

//         DPFBlock s_L, s_R;
//         bool t_L, t_R;
//         s_L = (t == true ? children[0] ^ s_CW : children[0]);
//         s_R = (t == true ? children[1] ^ s_CW : children[1]);
//         t_L = getLSB(children[0]) ^ (t & t_L_CW);
//         t_R = getLSB(children[1]) ^ (t & t_R_CW);
//         bool bit = get_bit(idx, key->get_depth(), i);
//         s = (bit == true ? s_R : s_L);
//         t = (bit == true ? t_R : t_L);
//     }
    

// #ifdef VERIFIABLE_DPF
//     Hash hash;
//     DPFBlock data[2], s_digest[2];
//     data[0] = makeBlock(0, idx);
//     data[1] = s;
//     if (hash_out != nullptr)
//     {
//         hash.hash_once(s_digest, data, 2 * sizeof(DPFBlock));
//         hash_out[0] = (t == true ? s_digest[0] ^ key->hash_CW[0] : s_digest[0]);
//         hash_out[1] = (t == true ? s_digest[1] ^ key->hash_CW[1] : s_digest[1]);
//     }
// #endif

//     // NOTE: this only works for F_{2^k}
//     // return (t == true ? s ^ key->get_s_CW(key->depth + 1) : s); 

//     // NOTE: this works for group
//     DPFBlock children[2];
//     prp->node_expand_1to2(children, s);
//     DPFBlock tmp = children[0];
//     // DPFBlock tmp = ConvertDPFBlock(s);
//     if (t) {
//         tmp += key->get_s_CW(key->depth + 1);
//     }

//     int64_t ans = _mm_extract_epi64(tmp, 0);
//     int64_t sign = (party==0)?1:-1;

//     delete prp; //delete after using
//     return sign*ans;
// }


DpfEvalInt2 dpf_eval_int_two(uint32_t party, DPFKEY *key, uint32_t idx, DPFBlock *hash_out)
{
    TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));

    DPFBlock s = key->get_s();
    bool t = key->get_t();

    DPFBlock s_CW;
    bool t_L_CW, t_R_CW;
    for (uint32_t i = 1; i <= key->depth; i++)
    {
        s_CW = key->get_s_CW(i);
        t_L_CW = key->get_t_L_CW(i);
        t_R_CW = key->get_t_R_CW(i);

        DPFBlock children[2];
        prp->node_expand_1to2(children, s);

        DPFBlock s_L, s_R;
        bool t_L, t_R;
        s_L = (t == true ? children[0] ^ s_CW : children[0]);
        s_R = (t == true ? children[1] ^ s_CW : children[1]);
        t_L = getLSB(children[0]) ^ (t & t_L_CW);
        t_R = getLSB(children[1]) ^ (t & t_R_CW);
        bool bit = get_bit(idx, key->get_depth(), i);
        s = (bit == true ? s_R : s_L);
        t = (bit == true ? t_R : t_L);
    }
    

#ifdef VERIFIABLE_DPF
    Hash hash;
    DPFBlock data[2], s_digest[2];
    data[0] = makeBlock(0, idx);
    data[1] = s;
    if (hash_out != nullptr)
    {
        hash.hash_once(s_digest, data, 2 * sizeof(DPFBlock));
        hash_out[0] = (t == true ? s_digest[0] ^ key->hash_CW[0] : s_digest[0]);
        hash_out[1] = (t == true ? s_digest[1] ^ key->hash_CW[1] : s_digest[1]);
    }
#endif

    // NOTE: this only works for F_{2^k}
    // return (t == true ? s ^ key->get_s_CW(key->depth + 1) : s); 

    // NOTE: this works for group
    DPFBlock children[2];
    prp->node_expand_1to2(children, s);
    DPFBlock tmp1 = children[0];
    DPFBlock tmp2 = children[1];
    // DPFBlock tmp = ConvertDPFBlock(s);
    if (t) {
        tmp1 += key->get_s_CW(key->depth + 1);
        tmp2 += key->get_s_CW(key->depth + 2);
    }

    uint64_t ans1 = uint64_t(_mm_extract_epi64(tmp1, 0));
    uint64_t ans2 = uint64_t(_mm_extract_epi64(tmp2, 0));
    uint64_t sign = (party == 0) ? 1 : -1;

    delete prp;

    return { sign * ans1, sign * ans2 };

}

// DpfEvalInt2 dpf_eval_int_two(uint32_t party, DPFKEY *key, uint32_t idx, DPFBlock *hash_out)
// {
//     TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));

//     DPFBlock s = key->get_s();
//     bool t = key->get_t();

//     DPFBlock s_CW;
//     bool t_L_CW, t_R_CW;
//     for (uint32_t i = 1; i <= key->depth; i++)
//     {
//         s_CW = key->get_s_CW(i);
//         t_L_CW = key->get_t_L_CW(i);
//         t_R_CW = key->get_t_R_CW(i);

//         DPFBlock children[2];
//         prp->node_expand_1to2(children, s);

//         DPFBlock s_L, s_R;
//         bool t_L, t_R;
//         s_L = (t == true ? children[0] ^ s_CW : children[0]);
//         s_R = (t == true ? children[1] ^ s_CW : children[1]);
//         t_L = getLSB(children[0]) ^ (t & t_L_CW);
//         t_R = getLSB(children[1]) ^ (t & t_R_CW);
//         bool bit = get_bit(idx, key->get_depth(), i);
//         s = (bit == true ? s_R : s_L);
//         t = (bit == true ? t_R : t_L);
//     }
    

// #ifdef VERIFIABLE_DPF
//     Hash hash;
//     DPFBlock data[2], s_digest[2];
//     data[0] = makeBlock(0, idx);
//     data[1] = s;
//     if (hash_out != nullptr)
//     {
//         hash.hash_once(s_digest, data, 2 * sizeof(DPFBlock));
//         hash_out[0] = (t == true ? s_digest[0] ^ key->hash_CW[0] : s_digest[0]);
//         hash_out[1] = (t == true ? s_digest[1] ^ key->hash_CW[1] : s_digest[1]);
//     }
// #endif

//     // NOTE: this only works for F_{2^k}
//     // return (t == true ? s ^ key->get_s_CW(key->depth + 1) : s); 

//     // NOTE: this works for group
//     DPFBlock children[2];
//     prp->node_expand_1to2(children, s);
//     DPFBlock tmp1 = children[0];
//     DPFBlock tmp2 = children[1];
//     // DPFBlock tmp = ConvertDPFBlock(s);
//     if (t) {
//         tmp1 += key->get_s_CW(key->depth + 1);
//         tmp2 += key->get_s_CW(key->depth + 2);
//     }

//     int ans1 = _mm_extract_epi32(tmp1, 0);
//     int ans2 = _mm_extract_epi32(tmp2, 0);
//     int sign = (party == 0) ? 1 : -1;

//     delete prp;

//     return { sign * ans1, sign * ans2 };

// }



void dpf_batch_eval(uint32_t party, DPFKEY *key, DPFBlock *vec, DPFBlock *proof)
{
    TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));
    // uint32_t len = 1 << key->get_depth();

    bool *t_vec = new bool[1 << key->get_depth()];

    DPFBlock s = key->get_s();
    bool t = key->get_t();
    vec[0] = s;
    t_vec[0] = t;

    for (uint32_t layer = 1; layer <= key->get_depth(); layer++)
    {
        uint32_t last_layer_size = 1 << (layer - 1);
        for (int32_t i = last_layer_size - 1; i >= 0; i--)
        {
            prp->node_expand_1to2(&vec[2 * i], vec[i]);

            DPFBlock s_CW = key->get_s_CW(layer);
            bool t_L_CW = key->get_t_L_CW(layer);
            bool t_R_CW = key->get_t_R_CW(layer);

            DPFBlock s_L, s_R;
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
    delete prp;

#ifdef VERIFIABLE_DPF
    DPFBlock *proof_vec = new DPFBlock[2 * 1 << key->get_depth()];
    // memset(proof_vec, 0, 2 * 1 << key->get_depth());
    Hash hash;
    DPFBlock tag_leaf[2], s_digest[2];
#endif

    for (uint32_t i = 0; i < 1 << key->get_depth(); i++)
    {
#ifdef VERIFIABLE_DPF
        tag_leaf[0] = makeBlock(0, i);
        tag_leaf[1] = vec[i];

        hash.hash_once(s_digest, tag_leaf, 2 * sizeof(DPFBlock));
        proof_vec[2 * i] = (t_vec[i] == true ? s_digest[0] ^ key->hash_CW[0] : s_digest[0]);     // the first half of hash tag for leaf i
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
    
    delete[] t_vec;
}

void dpf_batch_eval_int(uint32_t party, DPFKEY *key, DPFBlock *vec, int *vec_int, DPFBlock *proof)
{
    TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));

    const uint32_t depth = key->get_depth();
    const uint32_t n = (1u << depth);

    bool *t_vec = new bool[n];

    DPFBlock s = key->get_s();
    bool t = key->get_t();
    vec[0] = s;
    t_vec[0] = t;

    // full domain evaluation
    for (uint32_t layer = 1; layer <= depth; layer++)
    {
        uint32_t last_layer_size = 1u << (layer - 1);
        for (int32_t i = (int32_t)last_layer_size - 1; i >= 0; i--)
        {
            prp->node_expand_1to2(&vec[2 * i], vec[i]);

            DPFBlock s_CW = key->get_s_CW(layer);
            bool t_L_CW = key->get_t_L_CW(layer);
            bool t_R_CW = key->get_t_R_CW(layer);

            DPFBlock s_L = (t_vec[i] ? vec[2 * i]     ^ s_CW : vec[2 * i]);
            DPFBlock s_R = (t_vec[i] ? vec[2 * i + 1] ^ s_CW : vec[2 * i + 1]);
            bool t_L = getLSB(vec[2 * i])     ^ (t_vec[i] & t_L_CW);
            bool t_R = getLSB(vec[2 * i + 1]) ^ (t_vec[i] & t_R_CW);

            vec[2 * i] = s_L;
            vec[2 * i + 1] = s_R;
            t_vec[2 * i] = t_L;
            t_vec[2 * i + 1] = t_R;
        }
    }

#ifdef VERIFIABLE_DPF
    DPFBlock *proof_vec = new DPFBlock[2 * n];
    Hash hash;
    DPFBlock tag_leaf[2], s_digest[2];
#endif

    for (uint32_t i = 0; i < n; i++)
    {
#ifdef VERIFIABLE_DPF
        tag_leaf[0] = makeBlock(0, i);
        tag_leaf[1] = vec[i];
        hash.hash_once(s_digest, tag_leaf, 2 * sizeof(DPFBlock));
        proof_vec[2 * i]     = (t_vec[i] ? s_digest[0] ^ key->hash_CW[0] : s_digest[0]);
        proof_vec[2 * i + 1] = (t_vec[i] ? s_digest[1] ^ key->hash_CW[1] : s_digest[1]);
#endif

        // NOTE: group version
        DPFBlock children[2];
        prp->node_expand_1to2(children, vec[i]);
        DPFBlock tmp = children[0];

        if (t_vec[i]) tmp += key->get_s_CW(depth + 1);

        int ans = _mm_extract_epi32(tmp, 0);
        int sign = (party == 0) ? 1 : -1;
        vec_int[i] = sign * ans;
    }

#ifdef VERIFIABLE_DPF
    if (proof != nullptr)
        hash.hash_once(proof, proof_vec, 2 * n * sizeof(DPFBlock));
    delete[] proof_vec;
#endif

    delete[] t_vec;
    delete prp;
}
