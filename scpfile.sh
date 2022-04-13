
#!/bin/sh
ip=10.0.6.104
id=root
password=ketilinux
file=/home/keti/buildroot/source/AST2600_BMC/output/bin/KETI-Edge
path=/usr/sbin
sshpass -p $password scp output/bin/KETI-Edge root@10.0.6.104:/usr/sbin
sshpass -p $password scp KETI-REST/KETI-REST root@10.0.6.104:/usr/sbin
