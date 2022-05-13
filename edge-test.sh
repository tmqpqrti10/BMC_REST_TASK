#!/bin/sh

ip=10.0.6.104
id=root
password=ketilinux
path=/tmp

ssh-keygen -f "/root/.ssh/known_hosts" -R "$ip"
ssh-keyscan -t rsa $ip >>~/.ssh/known_hosts


edge=output/bin/KETI-Edge
test=output/bin/KETI-Edge-Test

cmake CMakeLists.txt
make -j 30 

mv $edge $test 
echo " overlay copy ..."

# cp $edge ../target_sys/firmware/


echo "scp ..."

sshpass -p $password scp $test $id@$ip:$path
# sshpass -p $password scp $edge $id@$ip:$path
