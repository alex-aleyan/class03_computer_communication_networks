#!/bin/bash

for i in {1..9}
do  
    cp file0.txt file${i}.txt
    echo "$i"; 
#    awk '{printf "line(%d)%s", NR, $0}' < file${i}.txt;
    #gawk -i '{printf("Line(%d) %s\n", NR, $0)}' file${i}.txt
    #echo $(awk '{printf("Line(%d) %s\n", NR, $0)}' file${i}.txt) > file${i}.txt
    #nl -s "." file${i}.txt > file${i}.txt
    #echo "$(nl -s " " file${i}.txt )" > file${i}.txt
    sed -i -e "s/^/file${i}:/" file${i}.txt; 
done
