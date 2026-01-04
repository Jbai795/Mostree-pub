#!/bin/bash 




echo -e "\n\n\n\nDPF benchmark for fss:"
./out/build/linux/bin/fss_main 0 12345 20 4179 & ./out/build/linux/bin/fss_main 1 12345 20 4179 & ./out/build/linux/bin/fss_main  2 12345 20 4179




