#!/bin/bash
cmd=$1
shift

cat /etc/sram_config.json.template | envsubst > /etc/sram_config.json 
service rsyslog start

if [ ! -z "$cmd" ]
then
    $cmd $@
fi