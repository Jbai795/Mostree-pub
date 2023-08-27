#include "fss_read.h"

void fss_read(const string file, vector<u64>& zeroOrOne, u64 num){
    string line;
    int i = 0;
    ifstream filename(file);
    //file.open(file);
#ifdef DTREE_DEBUG
        std::cout << "Reading from " << file << std::endl;
#endif 
    while(getline(filename, line) && i < num){
        zeroOrOne.push_back(atoi(line.c_str()));
        i++;
    }
    filename.close();
}