#!/bin/bash

# this will setup testing inputboard from ground
source ./env
mos config-set masterboard.id=3 masterboard.slaves=iiiiiiii eth.ip="192.168.100.198" eth.netmask="255.255.255.0" eth.gw="192.168.100.1" masterboard.transport.udp.sendaddress="192.168.100.60" masterboard.transport.udp.sendport=5556

