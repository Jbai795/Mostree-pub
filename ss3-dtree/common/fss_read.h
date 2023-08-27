#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "aby3/Common/Defines.h"
#include "aby3/Common/logging.h"
#include "aby3/Common/NTLHelper.h"

using namespace std;
using namespace aby3;

void fss_read(const string file, vector<u64>& zeroOrOne, u64 num);
