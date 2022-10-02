#!/bin/bash
set -xe
if ! [ -f /etc/powerdns/pdns.sqlite3 ] ; then
    sqlite3 /etc/powerdns/pdns.sqlite3 < /usr/share/doc/powerdns/schema.sqlite3.sql
fi
exec "$@"