#!/bin/bash
which wireshark sed awk make; if [[ $? -ne 0 ]]; then echo "install: wireshark, sed, awk, make"; fi
if [ "$?" -ne 0 ]; then cd $this_path; exit ; fi


this_path="$(pwd)"
echo $this_path
path_to_script="$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )"
echo $path_to_script
server_path="${path_to_script}/sw/server"
client_path="${path_to_script}/sw/client"

data_file0="file0.txt"
dest_file="file.txt"


cd $client_path
if [ "$?" -ne 0 ]; then cd $this_path; exit ; fi
rm $dest_file
make all
if [ "$?" -ne 0 ]; then cd $this_path; exit ; fi

# create 10 files from "data_file0"
if test -f "$data_file0"; then echo "$data_file0 exist"; fi

./duplicate_file0.sh

cd $client_path; 
$(cat README.txt) 
cat $dest_file

cd $this_path
