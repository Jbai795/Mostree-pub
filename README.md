# Mostree and Applications

## Introduction

This library provides the semi-honest and malicious implementation for Mostree in a three-party setting.

The repo includes the following application:
- [x] Decision tree (inference)
- [x] FSS keys generation and verification
- [x] Oblivious selection protocol (Semi-honest/Malicious)
- [x] Multiplication operation on GF2E (Semi-honest/Malicious)
- [x] MUX (Semi-honest/Malicious)
- [x] Comparison operation on boolean shared (Semi-honest/Malicious)
- [x] And operation on boolean shared (Semi-honest/Malicious)
- [x] Negate operation on boolean shared (Semi-honest/Malicious)

A tutorial can be found [here](https://github.com/Jbai795/Mostree-pub/blob/master/ss3-dtree/main-dtree.cpp). It includes a description of how to use the API and a discussion at the end on how the framework is implemented.

## Build

The library is *cross platform* and has been tested on Linux 20.04. The dependencies are:

 * [libOTe](https://github.com/osu-crypto/libOTe)
 * [Boost](http://www.boost.org/) ( _networking_ Boost 1.75.0)
 * [function2](https://github.com/Naios/function2)
 * [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)
 * [NTL](https://github.com/libntl/ntl) ( _Polynomial_ )
 * [aby3](https://github.com/ladnir/aby3)  [ _options_ ]
 * [backward](https://github.com/bombela/backward-cpp)  [ _options_ ]
 * gcc 9.4.0
 * python 3

### Step 1
- Complete the above third-party dependency and be installed in _deps/build/_. More details, refer to https://github.com/ladnir/aby3#build

### Step 2

*Build*

- In short, this will build the project

```
git clone https://github.com/Jbai795/PDTE-Mostree.git
mkdir bin
cd bin/
cmake -DCMAKE_BUILD_TYPE=Release ..
cd ../bin
make mostree-main
```

- Build use a short one

```
cd Mostree-pub
python build.py
```

*Run*

To see all the command line options, execute the program 

- Standalone
```
cd bin
./mostree-main -travel`
```
- Multi server 
```
cd bin
./mostree-main -travel -p 0  # machine 0
./mostree-main -travel -p 1  # machine 1
./mostree-main -travel -p 2  # machine 2
```

## Network Settings

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

## Help

Contact Jianli Bai(_jbai795@aucklanduni.ac.nz_), Xiaowu Zhang(_muou55555@gmail.com_) or Xiangfu Song(_bintasong@amail.com_) for any assistance on building or running the library.

## Citing

```
@misc{mostree-main,
    author = {Jianli Bai, Xiaowu Zhang, Xiangfu Song},
    title = {{Mostree: Malicious Secure Private Decision Tree Evaluation with Sublinear Communication}},
    howpublished = {\url{https://github.com/Jbai795/PDTE-Mostree.git}},
}
```

