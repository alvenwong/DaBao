#!/bin/sh

function modifyServerIP()
{
	sed -i "1s/.*/dest_ip = \"$1\"/" ../settings.cfg
}

function modifyPort()
{
	sed -i "2s/.*/ports = [$1]/" ../settings.cfg
}

function modifyEpoch()
{
	sed -i "3s/.*/epoch = $1/" ../settings.cfg
}

function modifyDevice()
{
	sed -i "4s/.*/dev = \"$1\"/" ../settings.cfg
}

function modifyRequestRate()
{
	sed -i "5s/.*/requestRate = $1/" ../settings.cfg
}

function modifyConnections()
{
	sed -i "6s/.*/connections = $1/" ../settings.cfg
}

function modifyKill()
{
	sed -i "7s/.*/kill = $1/" ../settings.cfg
}


function modifySIDStart()
{
	sed -i "8s/.*/SID_start = $1/" ../settings.cfg
}

function modifySIDEnd()
{
	sed -i "9s/.*/SID_end = $1/" ../settings.cfg

}

function modifyBurst()
{
	sed -i "11s/.*/burst = $1/" ../settings.cfg
}

function modifyUnitDelay()
{
	sed -i "12s/.*/burst = $1/" ../settings.cfg
}

while getopts "d:p:e:d:r:c:k:s:t:b:u:" arg;
do
	case $arg in
		d)
			modifyServerIP $OPTARG 
			;;
		p)
			modifyPort	$OPTARG
			;;
		e)
			modifyEpoch $OPTARG
			;;
		r)
			modifyRequestRate $OPTARG
			;;
		c) 
			modifyConnections $OPTARG
			;;
		k)
			modifyConnections $OPTARG
			;;
		s)
			modifySIDStart $OPTARG
			;;
		t)
			modifySIDEnd $OPTARG
			;;
		b)
			modifyBurst $OPTARG
			;;
		u)
			modifyUnitDelay $OPTARG
			;;
		?)
			echo "unknown argument"
	exit 1
	;;
	esac
done
