#pragma once
#pragma GCC diagnostic ignored "-Wunused-variable"

#include <fstream>
#include <iostream>
#include <cmath>
#include <cryptoTools/Common/CLP.h>
#include <cryptoTools/Network/IOService.h>
#include <aby3/Common/NTLHelper.h>
#include <aby3/Common/logging.h>
#include "common/tree_read_from_file.h"

using namespace mostree;

int dtree_main_3pc(oc::CLP& cmd);