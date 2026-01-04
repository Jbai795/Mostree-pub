#pragma once
#include "aby3/Common/logging.h"              // CW::LOGINTER::LogMessage
#include <Eigen/Dense>
#include <type_traits>

namespace CW { namespace LOGINTER {

// 默认：直接输出
template<typename T>
inline void printScalar(LogMessage& os, const T& v)
{
    os << v;
}

// 针对 1-byte 整数：按数字输出（避免当 char 打印成空白）
inline void printScalar(LogMessage& os, const unsigned char& v) { os << (unsigned)v; }
inline void printScalar(LogMessage& os, const signed char& v)   { os << (int)v; }
inline void printScalar(LogMessage& os, const char& v)          { os << (int)(unsigned char)v; }

// 通用 Eigen Matrix 打印
template<typename Scalar, int R, int C, int Opt, int MR, int MC>
inline LogMessage&
operator<<(LogMessage& os,
           const Eigen::Matrix<Scalar, R, C, Opt, MR, MC>& m)
{
    os << "Eigen::Matrix rows=" << m.rows()
       << " cols=" << m.cols() << "\n";

    for (int i = 0; i < m.rows(); ++i) {
        for (int j = 0; j < m.cols(); ++j) {
            printScalar(os, m(i, j));
            os << " ";
        }
        os << "\n";
    }
    return os;
}

}} // namespace CW::LOGINTER
