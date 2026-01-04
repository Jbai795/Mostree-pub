#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Common/CLP.h>
#include "aby3/sh3/Sh3Types.h"
#include "aby3/sh3/Sh3FixedPoint.h"
#include "aby3/sh3/Sh3Encryptor.h"

#include "SS3Com.h"
#include "main-share-x.h"

// // NTL
// #include <NTL/GF2E.h>
// using namespace NTL;

using namespace std;
using namespace oc;
using namespace aby3;
using namespace CW::LOGINTER;

const int PARTY_NUM = 3;

// // Example: Revealing the reconstructed result and checking
// void revealU8Matrix(const su8Matrix& valueSchema) {
//     bool correct = true;
//     for (size_t i = 0; i < valueSchema.rows(); ++i) {
//         for (size_t j = 0; j < valueSchema.cols(); ++j) {
//             // Reconstruct the value from the shares and check if it matches the expected value (0)
//             u8 reconstructed = (valueSchema.mShares[0](i, j) + valueSchema.mShares[1](i, j)) % 256;
//             if (reconstructed != 0) {
//                 std::cout << "Reconstruction error at [" << i << "," << j << "], value: " << (int)reconstructed << std::endl;
//                 correct = false;
//             }
//         }
//     }
//     if (correct) {
//         std::cout << "All values correctly reconstructed as 0." << std::endl;
//     } else {
//         std::cout << "There were errors in reconstruction." << std::endl;
//     }
// }



int bin_3pc(int pIdx, CLP &cmd, Session &chlPrev, Session &chlNext) {
    u64 bitLength = 64;

    // 1) 建“业务用”channels（不要用 main_3pc 那两个握手用的 channel）
    auto cNext = chlNext.addChannel("u8share");
    auto cPrev = chlPrev.addChannel("u8share");
    cNext.waitForConnection();
    cPrev.waitForConnection();

    // 2) 组 CommPkg（aby3 的 Sh3Encryptor 就吃这个）
    CommPkg comm;
    comm.mNext = std::move(cNext);
    comm.mPrev = std::move(cPrev);

    // 3) 初始化 Sh3Encryptor（用 comm + seed 的 init 方式）
    Sh3Encryptor enc;
    enc.init(pIdx, comm, oc::sysRandomSeed(), 256);

    // 4) party0 准备全 0 明文 u8Matrix
    u8Matrix plain;
    if (pIdx == 0) {
        plain.resize(bitLength, 1);
        for (u64 i = 0; i < plain.size(); ++i) plain(i) = 0;
    }

    // 5) 目标：得到 su8Matrix valueSchema（RSS sharing）
    su8Matrix valueSchema;
    valueSchema.resize(bitLength, 1);

    // 6) 调用你写的 localU8Matrix / remoteU8Matrix
    if (pIdx == 0) {
        enc.localU8Matrix(comm, plain, valueSchema);
    } else {
        enc.remoteU8Matrix(comm, valueSchema);
    }

    // 7) 使用 ostreamLock 保证输出顺序
    std::stringstream ss;
    ss << "Party " << pIdx << " got u8 shares:\n";
    valueSchema.printAllShares(pIdx, ss);  // 传递字符串流以便同步打印

    ostreamLock(std::cout << ss.str());  // 保证不交错

    // revealU8Matrix(valueSchema);

    LOG(INFO) << "Party " << pIdx << " preprocessing via localU8Matrix done!";
    return 0;
}






int main_3pc(oc::CLP &cmd) {
    IOService ios;
    
    std::vector<std::thread> thrds;
    for (u64 i = 0; i < PARTY_NUM; ++i) {
        this_thread::sleep_for(10ms);
        if (cmd.isSet("p") == false || cmd.get<int>("p") == i) {
            thrds.emplace_back(std::thread([i, &cmd, &ios]() {

                auto next = (i + 1) % 3;
                auto prev = (i + 2) % 3;
                auto cNameNext = std::to_string(std::min(i, next)) + std::to_string(std::max(i, next));
                auto cNamePrev = std::to_string(std::min(i, prev)) + std::to_string(std::max(i, prev));

                auto modeNext = i < next ? SessionMode::Server : SessionMode::Client;
                auto modePrev = i < prev ? SessionMode::Server : SessionMode::Client;
                auto portNext = 1212 + std::min(i, next);
                auto portPrev = 1212 + std::min(i, prev);

                Session epNext(ios, "127.0.0.1", portNext, modeNext, cNameNext);
                Session epPrev(ios, "127.0.0.1", portPrev, modePrev, cNamePrev);

                ostreamLock(std::cout << "party " << i << " next " << portNext << " mode=server?:"
                                      << (modeNext == SessionMode::Server) << " name " << cNameNext << std::endl);
                ostreamLock(std::cout << "party " << i << " prev " << portPrev << " mode=server?:"
                                      << (modePrev == SessionMode::Server) << " name " << cNamePrev << std::endl);
                auto chlNext = epNext.addChannel();
                auto chlPrev = epPrev.addChannel();

                chlNext.waitForConnection();
                chlPrev.waitForConnection();
                chlNext.send(i);
                chlPrev.send(i);
                u64 prevAct = 99, nextAct = 99;
                chlNext.recv(nextAct);
                chlPrev.recv(prevAct);

                if (next != nextAct)
                    std::cout << " bad next party idx, act: " << nextAct << " exp: " << next << std::endl;
                if (prev != prevAct)
                    std::cout << " bad prev party idx, act: " << prevAct << " exp: " << prev << std::endl;

                bin_3pc(i, cmd, epPrev, epNext);

            }));
        }
    }

    for (auto &t: thrds)
        t.join();

    return 0;
}

void help()
{
    std::cerr << "+ Mostree-3pc Command Help" << std::endl;
    std::cerr << "01) -travel [-p] [0 1 2] [-i modelid] [-s] [-m]     ~~ to travel decision tree on party [-p n]" << std::endl;
}

int main(int argc, char **argv) {
    try {
        bool set = false;
        oc::CLP cmd(argc, argv);

        if (cmd.isSet("travel")) {
            set = true;
            main_3pc(cmd);
        }

        if (set == false) {
            help();
        }

    }
    catch (std::exception& e)
    {
        LOG(INFO) << e.what() << std::endl;
    }
}
