#!/bin/sh
set -xe
#su pdns -c "pdnsutil create-zone example.com ns1.example.com"
#su pdns -c "pdnsutil add-record example.com '' MX '25 mail.example.com'"
#su pdns -c "pdnsutil add-record example.com. www A 192.0.2.1"

#sqlite3 /etc/powerdns/pdns.sqlite3 < /usr/share/doc/powerdns/schema.sqlite3.sql
#pdnsutil -v create-zone example.com ns1.example.com
#pdnsutil add-record example.com '' MX '25 mail.example.com'
#pdnsutil add-record example.com. www A 192.0.2.1

exec "$@"