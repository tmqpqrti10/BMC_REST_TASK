#!/bin/sh

ip=10.0.6.104
id=root
password=ketilinux
path=/usr/sbin

ssh-keygen -f "/root/.ssh/known_hosts" -R "$ip"
ssh-keyscan -t rsa $ip >>~/.ssh/known_hosts


edge=output/bin/KETI-Edge
rest=output/bin/KETI-REST
psu=output/bin/KETI-PSU
smltr=smltr/smltr
kvm=kvm/KETI-KVM
cmake CMakeLists.txt
make -j 30 

cd kvm
cmake CMakeLists.txt
make
cd ../

echo " overlay copy ..."
#cp ./output/bin/* ../target_sys/firmware/
cp $edge ../target_sys/firmware/
cp $rest ../target_sys/firmware/
cp $kvm ../target_sys/firmware/
#cp $psu ../overlay/usr/sbin/

echo "scp ..."

#sshpass -p $password scp $edge $rest $kvm $smltr $id@$ip:$path
