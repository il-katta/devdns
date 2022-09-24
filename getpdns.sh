#!/bin/bash
set -xe
rm -rf pdns
git clone https://github.com/PowerDNS/pdns.git --branch auth-4.6.3
cp -r devdnsbackend pdns/modules/
cat <<EOF > pdns/modules/Makefile.am
SUBDIRS= @moduledirs@

DIST_SUBDIRS = \
	bindbackend \
	devdnsbackend \ 
	geoipbackend \
	gmysqlbackend \
	godbcbackend \
	gpgsqlbackend \
	gsqlite3backend \
	ldapbackend \
	lmdbbackend \
	lua2backend \
	pipebackend \
	remotebackend \
	tinydnsbackend
EOF
cd pdns
autoreconf -vi
./configure \
    --prefix=/usr \
    --sysconfdir=/etc/powerdns \
    --sbindir=/usr/bin \
    --with-modules="" \
    --with-dynmodules="devdns pipe" \
    --docdir=/usr/share/doc/powerdns \
    --with-libsodium \
    --enable-tools \
    --enable-ixfrdist \
    --enable-dns-over-tls \
    --disable-dependency-tracking \
    --disable-silent-rules \
    --enable-reproducible \
    --enable-unit-tests \
    --enable-systemd \
    --with-service-user=powerdns --with-service-group=powerdns 

make