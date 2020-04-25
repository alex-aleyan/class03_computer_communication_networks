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
make all
if [ "$?" -ne 0 ]; then cd $this_path; exit ; fi

cd $this_path

cd $client_path
if [ "$?" -ne 0 ]; then cd $this_path; exit ; fi
rm $dest_file
make all
if [ "$?" -ne 0 ]; then cd $this_path; exit ; fi

# create 10 files from "data_file0"
if test -f "$data_file0"; then echo "$data_file0 exist"; fi

./duplicate_file0.sh

wireshark -i lo -k &
sleep 3

cd $server_path; 
$(cat README.txt) &

cd $client_path; 
$(cat README.txt) 

echo -e "\n#############################Client received $dest_file with this content:###############################"
cat $dest_file
echo -e   "#############################Content ends @ line above                    ###############################\n\n"


cd $server_path; make clean; if [ "$?" -ne 0 ]; then cd $this_path; exit ; fi
cd $client_path; make clean; if [ "$?" -ne 0 ]; then cd $this_path; exit ; fi

./duplicate_remove.sh; if [ "$?" -ne 0 ]; then cd $this_path; exit ; fi

echo -e "\n#############################To manually run the client and server applications:#############################"


echo -e "\nFirst build the project using:"
echo -e "${path_to_script}/makeall.sh"

echo -e "\nNext please launch the server and client applications like this (order does not matter but server first is preferred):"
echo -e "$(cat ${client_path}/README.txt)"
echo -e "$(cat ${server_path}/README.txt)"


cd $this_path

echo -e "\n\n#############################See the file resulted from the transfer @ $server_path/file.txt \
\n\t$server_path/README.txt \
\n\t$client_path/README.txt"
