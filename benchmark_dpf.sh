#!/bin/bash 




echo -e "\n\n\n\nDPF benchmark for MINIST:"
./out/build/linux/bin/dpf_batch_gen 0 12345 20 4179 & ./out/build/linux/bin/dpf_batch_gen 1 12345 20 4179 & ./out/build/linux/bin/dpf_batch_gen  2 12345 20 4179

sleep 1.5 

echo -e "\n\n\n\nDPF benchmark for Boston:"
./out/build/linux/bin/dpf_batch_gen 0 12345 30 851 & ./out/build/linux/bin/dpf_batch_gen 1 12345 30 851 & ./out/build/linux/bin/dpf_batch_gen  2 12345 30 851

sleep 1.5 

echo -e "\n\n\n\nDPF benchmark for diabetes:"
./out/build/linux/bin/dpf_batch_gen 0 12345 28 787 & ./out/build/linux/bin/dpf_batch_gen 1 12345 28 787 & ./out/build/linux/bin/dpf_batch_gen  2 12345 28 787

sleep 1.5 

echo -e "\n\n\n\nDPF benchmark for spambase:"
./out/build/linux/bin/dpf_batch_gen 0 12345 17 256 & ./out/build/linux/bin/dpf_batch_gen 1 12345 17 256  & ./out/build/linux/bin/dpf_batch_gen  2 12345 17 256 

sleep 1.5 

echo "DPF benchmark for digits:"
./out/build/linux/bin/dpf_batch_gen 0 12345 15 337 & ./out/build/linux/bin/dpf_batch_gen 1 12345 15 337 & ./out/build/linux/bin/dpf_batch_gen  2 12345 15 337




