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

cd $server_path
if [ "$?" -ne 0 ]; then cd $this_path; exit ; fi
rm $dest_file
make test_abort
if [ "$?" -ne 0 ]; then cd $this_path; exit ; fi

$(cat README.txt)

cd $this_path
