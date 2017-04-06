# Project Description
This project is to leverage epoll to implement a high-density client which is able to generate more than one million TCP flows within just one physical mechine and a server which is capable of supporting millions of concurrent flows.

# Dependency
In client, you need to install libconfig (>1.4). In server, you need to install both libconfig (>1.4) and hiredis.

# How to use

## Client
### Compiling
$ cd client/ and make, you will get an executable file client.
### configuration Description
client/settings.cfg is the client's configuration file. The meaning of the parameters are as follows: <br/>
dest_ip: the server's IP<br/>
ports: the server's port<br/>
epoch: the packet sending period, i.e., any flow sends a packet to the server in every epoch (second)<br/>
dev: the name of NIC<br/>
requestRate: the proportion of packets requiring a responce from the server<br/>
connections: the number of concurrent flows<br/>
kill: kill the running program<br/>
SID_start, SID_end： the start and end index of flows, which can be regarded as the primary key. When there are more than one clients in the system, please prevent the range from overlapping<br/>
burst: the burst of packets<br/>
unit_delay: control the time of beginning sending packets<br/>
### Usage
After configuring settings.cfg, run this flows generator with <br/>
$ ./client
### Common errors
a. No route to host<br/>
try to stop or close the iptables with<br/>
$ systemctl stop firewalld.service<br/>
b. libconfig.so.9: cannot open shared object file<br/>
solve this error with <br/>
$ export LD_LIBRARY_PATH=LD_LIBRARY_PATH:/usr/local/lib<br/>
c. too many open files<br/>
check you "ulimit -n" and increase it, e.g.,<br/>
$ ulimit -n 10000000<br/>
d. Just successfully establish part of connections, while larger than 1024<br/>
check your mechine's net.nf_conntrack_max with <br/>
$ cat /proc/sys/net/nf_conntrack_max<br/>
If the value is consistent with the number of established connections, increase it. For example,<br/>
$ echo 1000000 > /proc/sys/net/nf_conntrack_max<br/>
Otherwise, check your mechine's fs.file-max with <br/>
$ cat /proc/sys/fs/file-max<br/>

## Server
### Compiling
$ cd server/ and make, you will get an executable file server
### configuration Description
server/settings.cfg is the server's configuration file. The meaning of the parameters are as follows:<br/>
ports: the port of the server<br/>
delay_in_ms: the virtual process delay for each packet (microsecond)<br/>
threads_num: the number of threads to process packets<br/>
device: the name of the NIC receiving packets<br/>
database_ip: database cluster ip<br/>
database_port: database cluster port<br/>
### Usage
After configuring settings.cfg, run this server program with <br/>
$ ./server
### Common errors
a. libhiredis.so.o.13: cannot open shared object file<br/>
export LD_LIBRARY_PATH=LD_LIBRARY_PATH:/usr/local/lib<br/>
