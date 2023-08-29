#!/bin/bash 

echo "DPF benchmark for depth=50:"
./out/build/linux/bin/dpf_batch_gen 0 12345 50 1250 & ./out/build/linux/bin/dpf_batch_gen 1 12345 50 1250 & ./out/build/linux/bin/dpf_batch_gen  2 12345 50 1250

sleep 1.5 

echo "\n\n\nDPF benchmark for depth=45:"
./out/build/linux/bin/dpf_batch_gen 0 12345 45 1125 & ./out/build/linux/bin/dpf_batch_gen 1 12345 45 1125 & ./out/build/linux/bin/dpf_batch_gen  2 12345 45 1125

sleep 1.5 

echo "\n\n\nDPF benchmark for depth=40:"
./out/build/linux/bin/dpf_batch_gen 0 12345 40 1000 & ./out/build/linux/bin/dpf_batch_gen 1 12345 40 1000 & ./out/build/linux/bin/dpf_batch_gen  2 12345 40 1000

sleep 1.5 

echo -e "\n\n\nDPF benchmark for depth=35:"
./out/build/linux/bin/dpf_batch_gen 0 12345 35 875 & ./out/build/linux/bin/dpf_batch_gen 1 12345 35 875  & ./out/build/linux/bin/dpf_batch_gen  2 12345 35 875 


sleep 1.5 

echo -e "\n\n\nDPF benchmark for depth=30:"
./out/build/linux/bin/dpf_batch_gen 0 12345 30 750 & ./out/build/linux/bin/dpf_batch_gen 1 12345 30 750 & ./out/build/linux/bin/dpf_batch_gen  2 12345 30 750

sleep 1.5 

echo -e "DPF benchmark for depth=25:"
./out/build/linux/bin/dpf_batch_gen 0 12345 25 625 & ./out/build/linux/bin/dpf_batch_gen 1 12345 25 625 & ./out/build/linux/bin/dpf_batch_gen  2 12345 25 625

sleep 1.5 

echo -e "DPF benchmark for depth=20:"
./out/build/linux/bin/dpf_batch_gen 0 12345 20 500 & ./out/build/linux/bin/dpf_batch_gen 1 12345 20 500  & ./out/build/linux/bin/dpf_batch_gen  2 12345 20 500 
