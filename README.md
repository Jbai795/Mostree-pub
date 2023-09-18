# Mostree and Applications

## Introduction

This library provides the semi-honest and malicious implementation for Mostree (DPF-based) in a three-party setting.

The repo includes the following application:
- [x] Decision tree (inference)
- [x] DPF keys generation and verification
- [x] Oblivious selection protocol (Semi-honest/Malicious)
- [x] Multiplication operation on GF2E (Semi-honest/Malicious)
- [x] MUX (Semi-honest/Malicious)
- [x] Comparison operation on boolean shared (Semi-honest/Malicious)
- [x] And operation on boolean shared (Semi-honest/Malicious)
- [x] Negate operation on boolean shared (Semi-honest/Malicious)

A tutorial can be found [here](https://github.com/Jbai795/Mostree-pub/blob/master/ss3-dtree/main-dtree.cpp). It includes a description of how to use the API and a discussion at the end on how the framework is implemented.

## Build

This project is has been tested on Ubuntu 20.04. The dependencies are:

 * [libOTe](https://github.com/osu-crypto/libOTe)
 * [Boost](http://www.boost.org/) ( _networking_ Boost 1.75.0)
 * [function2](https://github.com/Naios/function2)
 * [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)
 * [NTL](https://github.com/libntl/ntl) ( _Polynomial_ )
 * [emp-tool](https://github.com/emp-toolkit/emp-tool) (for offline preprocessing benchmark)
 * [aby3](https://github.com/ladnir/aby3)  [ _options_ ]
 * [backward](https://github.com/bombela/backward-cpp)  [ _options_ ]
 * gcc 9.4.0
 * python 3

### Step 1
Complete the above third-party dependency and be installed in _deps/build/_.

- Install libraries from apt-get
```
[sudo] apt-get update 
[sudo] apt-get install git python3 wget build-essential libssl-dev g++ python-dev autotools-dev libicu-dev libbz2-dev libboost-all-dev libgmp-dev
```

- Install Cmake 
```
wget https://github.com/Kitware/CMake/releases/download/v3.27.3/cmake-3.27.3.tar.gz
tar -zxvf cmake-3.27.3.tar.gz 
cd cmake-3.27.3 
./bootstrap 
make
make install 
```

- install boost
```
wget https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_1_75_0.tar.bz2
tar -xvf boost_1_75_0.tar.bz2
cd boost_1_75_0
./bootstrap.sh --prefix=/usr/
./b2 install
```

- install NTL
```
wget https://libntl.org/ntl-11.5.1.tar.gz
gunzip ntl-11.5.1.tar.gz
tar xf ntl-11.5.1.tar
cd ntl-11.5.1/src
./configure 
make
make install
```

- ABY3 
```
git clone https://github.com/ladnir/aby3.git
cd aby3 
python3 build.py --setup 
python3 build.py 
```

- [emp-toolkit](https://github.com/emp-toolkit/emp-tool) (for offline DPF preprocessing)
```
wget https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py
python install.py --deps --tool
```

<!-- ### Step 1
- Complete the above third-party dependency and be installed in _deps/build/_. More details, refer to https://github.com/ladnir/aby3#build -->

### Step 2

*Build*

<!-- - In short, this will build the project

```
git clone https://github.com/Jbai795/Mostree-pub
mkdir bin
cd bin/
cmake -DCMAKE_BUILD_TYPE=Release ..
cd ../bin
make mostree-main
```

- Build use a short one -->

```
git clone https://github.com/Jbai795/Mostree-pub
cd Mostree-pub
python3 build.py
```

*Run*

NOTE: Run programs within the diectory `./out/build/linux/bin/` in the main directory `Mostree-pub/`, otherwise you will encounter open errors.

- Standalone: log benchmark info in one terminal
```
cd ./out/build/linux/bin
./mostree-main -travel
```
- Multi terminals 
```
cd ./out/build/linux/bin
./mostree-main -travel -p 0  # terminal 0
./mostree-main -travel -p 1  # terminal 1
./mostree-main -travel -p 2  # terminal 2
```

## Configuration
One can check benchmark performance of mostree over different datasets and settings. To configure mostree concretely, modify configuration in `Mostree-pub/ss3-tree/main-dtree.cpp`.

#### 1. Setting decision trees
The arrary `filename` shows seven trees.   
```c++
const char *filename[7] = {
    "wine",
    "breast",
    "digits",
    "spambase",
    "diabetes",
    "boston",
    "mnist"
};
```
To benchmarn mostree over different tress, configure `modelid` at line 50. For example, if one want to run mostree over `digits`, set
```c++ 
//modify here if testing other trees
int modelid = 2;
```

#### 2. Semi-honest security vs. malicious security
Our implementation supports both semi-honest and malicious security. To evaluate malicious security, set `bool MALICIOUS = 1` at line 28. If one set `bool MALICIOUS = 0`, then Mostree will run the code with semi-honest security. 

#### 3. Test scalability of Mostree
Besiding evaluating Mostree over real datasets, one can also check scalability of Mostree using fake datasets. One can set `SCALABILITY` at line 30. In particular, setting `SCALABILITY=0` will use the real UCI dataset and setting `SCALABILITY=1` will use the fake datasets to test scalability of Mostree. 
```c++
bool SCALABILITY = 1; // 1 means testing for scalability using fake data, 0 means testing real datasets
//  set modelid if SCALABILITY = 1
```

#### 4. Summary
One can set `MALICIOUS`, `SCALABILITY` and `modelid` to test Mostree's performance under different settings. For example, if one want to check performance of Mostree for `minist` with malicious security, then just set `MALICIOUS=1`, `SCALABILITY=0`, and `modelid=6`.  
If one want to check scalibility of mostree in the semi-honest setting, then just set `MALICIOUS=0`, `SCALABILITY=1`; in this case `modelid` doesn't function. 


## Network Settings

To text mostree under different network setting, we use the following `tc` commands to simulate different network settings.  

LAN, RTT:0.1ms, 1Gbps

`sudo tc qdisc add dev lo root netem delay 0.04ms rate 1024mbit`

MAN, RTT:6ms, 100Mbps

`sudo tc qdisc add dev lo root netem delay 3ms rate 100mbit`

WAN, RTT:80ms, 40Mbps

`sudo tc qdisc add dev lo root netem delay 40ms rate 40mbit`

ping localhost to see RTT

`ping localhost -c 6`

delete simulated configuration: (must delete the old one before setting new simulation)

`sudo tc qdisc delete dev lo root netem delay 0.04ms rate 1024mbit`

## Offline correlation generation benchmark 
This part is about running time and communication performance from offline correlation generation for oblivious selection. We note that DPF-based preprocessing is suitable for oblivious selection over vectors of large dimension. Therefore, the dealer uses DPF-based preproprocessing for `digits`, `spambase`, `diabetes`, `Boston`, and `MINIST`. For small models `wine` and `breast`, the dealer simply distributes random one-hot vectors in the offline phase.

NOTE: Currently we have not merged this preprocessing into Mostree (Mostree assumes the one-hot vectors are already generated when running the online evaluation phase). Nevertheless, this part is sufficient for benchmark purposes.

#### 1. DPF-based preprocessing
One can run batch DPF generation performance as follows. 
```
cd ./out/build/linux/bin
./dpf_batch_gen 0 [port] [decision tree depth] [number of tree nodes] # terminal 0
./dpf_batch_gen 1 [port] [decision tree depth] [number of tree nodes] # terminal 1
./dpf_batch_gen 2 [port] [decision tree depth] [number of tree nodes] # terminal 2
```
For example, for the `MINIST` dataset with a tree depth of 20 and 4179 nodes:  

```
cd ./out/build/linux/bin 
./dpf_batch_gen 0 12345 20 4179 # terminal 0
./dpf_batch_gen 1 12345 20 4179 # terminal 1
./dpf_batch_gen 2 12345 20 4179 # terminal 2
```
We also provide benchmark files `benchmark_dpf.sh` and `benchmark_dpf_scale.sh`. One can run in the main directory to check the running time and communication overhead for different decision trees. 

#### 2. One-Hot-Vector-based preprocessing

One can run One-Hot-Vector-based preprocessing as follows.

```
cd ./out/build/linux/bin
./dpf_batch_gen 0 [port] [decision tree depth] [number of tree nodes] # terminal 0
./dpf_batch_gen 1 [port] [decision tree depth] [number of tree nodes] # terminal 1
./dpf_batch_gen 2 [port] [decision tree depth] [number of tree nodes] # terminal 2
```
For example, for the `breast` dataset with tree depth of 7 and 43 nodes:  

```
cd ./out/build/linux/bin
./dpf_batch_gen 0 12345 7 43 # terminal 0
./dpf_batch_gen 1 12345 7 43 # terminal 1
./dpf_batch_gen 2 12345 7 43 # terminal 2
```
We also provide benchmark file `benchmark_ohv.sh`. One can run in the main directory to check the running time and communication overhead for `wine` and `breast`. 

## Disclaimer
This code is just a proof-of-concept for benchmarking purposes. It has not had any security review, has a number of implementational TODOs, and thus, should not be directly used in any real-world applications.


## Help

Contact Jianli Bai(_jbai795@aucklanduni.ac.nz_), Xiaowu Zhang(_muou55555@gmail.com_) or Xiangfu Song(_bintasong@amail.com_) for any assistance on building or running the library.

## Citing

```
@misc{mostree-main,
    author = {Jianli Bai, Xiaowu Zhang, Xiangfu Song},
    title = {{Mostree: Malicious Secure Private Decision Tree Evaluation with Sublinear Communication}},
    howpublished = {\url{https://github.com/Jbai795/Mostree-pub}},
}
```

