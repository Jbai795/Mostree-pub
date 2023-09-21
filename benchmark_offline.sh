#!/bin/bash 


for i in `seq 0 6`;
do
    echo -e "\n--------------------------------- begin modelid = $i -----------------------------------"
	./out/build/linux/bin/offline_batch_gen -offline -i $i 
    echo -e "--------------------------------- end modelid = $i -------------------------------------\n"
done
