#!/bin/bash
set -e
cd "$( dirname "$0" )"
docker build --pull -t registry.loopback.it/andrea/devdns .
docker run --rm -ti \
    --hostname devdns \
    --name devdns \
    -p 5354:53 \
    -p 5354:53/udp \
    -p 8081:8081 \
    --volume "${PWD}/conf":/etc/powerdns \
    registry.loopback.it/andrea/devdns "$@" #|| \
#docker run --rm --volume "${PWD}/conf":/etc/powerdns -ti registry.loopback.it/andrea/devdns bash