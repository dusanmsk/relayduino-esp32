#!/bin/bash
source ./env


# TODO - znizit loglevel, zavolat Halt (system nebude tak vytrazeny a update bude rychlejsi)
OLD_LEVEL=`mos config-get debug.level`
mos config-set debug.level=0

#curl -v -F file=@build/fw.zip http://${ESP_IP_ADDRESS}/update
mos ota --port ws://$ESP_IP_ADDRESS/rpc build/fw.zip

echo "Sleeping then commiting ..."
sleep 10
mos call Ota.Commit
echo "Image commited"
mos config-set debug.level=$OLD_LEVEL

