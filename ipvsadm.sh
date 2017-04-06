ipvsadm -C
ipvsadm -A -t 192.168.6.4:18880 -s sh
#ipvsadm -a -t 192.168.6.4:18880 -r 192.168.3.3 -m
#ipvsadm -a -t 192.168.6.4:18880 -r 192.168.3.4 -m
ipvsadm -a -t 192.168.6.4:18880 -r 192.168.3.5 -m
#ipvsadm -a -t 192.168.6.4:18880 -r 192.168.3.6 -m
ipvsadm -L -n
