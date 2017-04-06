# Project Description
This project is to leverage epoll to implement a high-density client which is able to generate more than one million TCP flows within just one physical mechine and a server which is capable of supporting millions of concurrent flows.

# Dependency
In client, you need to install libconfig (>1.4). In server, you need to install both libconfig (>1.4) and hiredis.

# How to use

## Client
### Compiling
$ cd client/ and make, you will get an executable file client.
### configuration Description
client/settings.cfg is the client's configuration file. The meaning of the parameters are as follows:
dest_ip: the server's IP
ports: the server's port
epoch: the packet sending period, i.e., any flow sends a packet to the server in every epoch (second)
dev: the name of NIC
requestRate: the proportion of packets requiring a responce from the server
connections: the number of concurrent flows
kill: kill the running program
SID_start, SID_end： the start and end index of flows, which can be regarded as the primary key. When there are more than one clients in the system, please prevent the range from overlapping
burst: the burst of packets
unit_delay: control the time of beginning sending packets
### Usage
After configuring settings.cfg, run this flows generator with 
$ ./client
### Common errors
a. No route to host
try to stop or close the iptables with
$ systemctl stop firewalld.service
b. libconfig.so.9: cannot open shared object file
solve this error with 
$ export LD_LIBRARY_PATH=LD_LIBRARY_PATH:/usr/local/lib
c. too many open files
check you "ulimit -n" and increase it, e.g.,
$ ulimit -n 10000000
d. Just successfully establish part of connections, while larger than 1024
check your mechine's net.nf_conntrack_max with 
$ cat /proc/sys/net/nf_conntrack_max
If the value is consistent with the number of established connections, increase it. For example,
$ echo 1000000 > /proc/sys/net/nf_conntrack_max
Otherwise, check your mechine's fs.file-max with 
$ cat /proc/sys/fs/file-max

## Server
### Compiling
$ cd server/ and make, you will get an executable file server
### configuration Description
server/settings.cfg is the server's configuration file. The meaning of the parameters are as follows:
ports: the port of the server
delay_in_ms: the virtual process delay for each packet (microsecond)
threads_num: the number of threads to process packets
device: the name of the NIC receiving packets
database_ip: database cluster ip
database_port: database cluster port
### Usage
After configuring settings.cfg, run this server program with 
$ ./server
### Common errors
a. libhiredis.so.o.13: cannot open shared object file
export LD_LIBRARY_PATH=LD_LIBRARY_PATH:/usr/local/lib
