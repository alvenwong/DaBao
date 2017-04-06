#!/bin/sh

clientsIPsFile=clients_ip.txt
serversIPsFile=servers_ip.txt

command=$1
slaves=$2

if [ "$command" = "start" ]; then
	if [ "$slaves" = "client" ]; then
		echo "start clients"
		cat $clientsIPsFile | while read ip
		do 
			echo $ip;
			ssh $ip "cd /home/wangzhuang/IoT_epoll/yxd_client/client;
						ulimit -n 1000000;
						sh start.sh;
						exit;" &
		done
	elif [ "$slaves" = "server" ]; then
		echo "start servers"
		cat $serversIPsFile | while read ip
		do 
			echo $ip;
			ssh $ip "cd /home/wangzhuang/IoT_epoll/yxd_client/server;
				export LD_LIBRARY_PATH=LD_LIBRARY_PATH:/usr/local/lib;
						ulimit -n 1000000;
						./server;
						exit;" &
		done
	fi

elif [ "$command" = "kill" ]; then
	if [ "$slaves" = "client" ]; then
		echo "kill clients"
		cat $clientsIPsFile | while read ip
		do 
			echo $ip;
			ssh $ip "cd /home/wangzhuang/IoT_epoll/yxd_client/client;
						sh kill.sh;
						exit;"
		done
	elif [ "$slaves" = "server" ]; then
		echo "kill servers"
		cat $serversIPsFile | while read ip
		do 
			echo $ip;
			ssh $ip "cd /home/wangzhuang/IoT_epoll/yxd_client/server;
						sh kill.sh;
						exit;" &
		done
	fi
elif [ "$command" = "update" ]; then
	if [ "$slaves" = "server" ]; then
		echo "update code"
		cat $serversIPsFile | while read ip
		do 
			echo $ip
			ssh $ip "cd /home/wangzhuang;
				rm -rf IoT_epoll;
				exit;" &
			scp -r /home/wangzhuang/IoT_epoll root@$ip:/home/wangzhuang
		done
	fi
else
	echo "unknown command"
fi
