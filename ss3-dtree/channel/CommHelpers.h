#pragma once
#include <cryptoTools/Network/Channel.h>
#include <vector>
#include <cstdint>
#include "dpf/dpf.h"

namespace mostree {

inline void sendBytes(osuCrypto::Channel& ch,
                      const std::vector<uint8_t>& bytes)
{
    uint64_t n = static_cast<uint64_t>(bytes.size());
    ch.send(n);
    if (n) ch.send(bytes);
}

inline void recvBytes(osuCrypto::Channel& ch,
                      std::vector<uint8_t>& bytes)
{
    uint64_t n = 0;
    ch.recv(n);
    bytes.resize(static_cast<size_t>(n));
    if (n) ch.recv(bytes);
}

inline void sendDPFKeyBlob(osuCrypto::Channel& ch,
                           const DPFKeyBlob& blob)
{
    sendBytes(ch, blob.bytes);
}

inline void recvDPFKeyBlob(osuCrypto::Channel& ch,
                           DPFKeyBlob& blob)
{
    recvBytes(ch, blob.bytes);
}

} // namespace mostree
