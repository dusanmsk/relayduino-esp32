#!/bin/bash

# this will setup testing outputboard from ground
source ./env
mos config-set masterboard.id=3 masterboard.slaves=oooooooo eth.ip="192.168.100.199" eth.netmask="255.255.255.0" eth.gw="192.168.100.1" masterboard.transport.udp.listenport=6666 



