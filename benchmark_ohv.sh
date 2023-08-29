#!/bin/bash 


echo -e "DPF benchmark for Breast:"
./out/build/linux/bin/ohv_batch_gen 0 12345 7 43 & ./out/build/linux/bin/ohv_batch_gen 1 12345 7 43 & ./out/build/linux/bin/ohv_batch_gen  2 12345 7 43

sleep 1.5 

echo -e "\n\n\n\nDPF benchmark for Wine:"
./out/build/linux/bin/ohv_batch_gen 0 12345 5 23 & ./out/build/linux/bin/ohv_batch_gen 1 12345 5 23 & ./out/build/linux/bin/ohv_batch_gen  2 12345 5 23





