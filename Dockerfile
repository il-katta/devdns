FROM debian:11-slim as pdns-builder
ARG PDNS_TAG
ENV PDNS_TAG=${PDNS_TAG:-auth-4.7.4}
RUN set -x && \
    apt-get -qq update && \
    apt-get install -qq -y g++ libboost-all-dev libtool make pkg-config libpq-dev libpqxx-dev libssl-dev libluajit-5.1-dev python3-venv libsqlite3-dev sqlite3 \
        libsodium-dev libyaml-cpp-dev libcurl4-openssl-dev libfmt-dev \
        autoconf automake ragel bison flex cmake \
        git curl wget && \
    apt-get clean all -y && apt-get autoclean -y && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
    mkdir -p /usr/src/libdecaf && \
    # libdecaf
    ln -s /usr/bin/python3 /usr/bin/python && \
    wget "https://sourceforge.net/projects/ed448goldilocks/files/libdecaf-1.0.0.tgz/download" -O - | tar xz -C /usr/src/libdecaf --strip-components=1 && \
    cd /usr/src/libdecaf && \
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr . && \
    make && \
    make DESTDIR=/output/ install && make install && \
    # powerdns build & install
    git clone https://github.com/PowerDNS/pdns.git --branch "$PDNS_TAG" /usr/local/pdns && \
    cd /usr/local/pdns && \
    autoreconf -vi && \
    CFLAGS="-march=native -O2 -pipe" \
    CPPFLAGS="${CFLAGS} -I/usr/include/decaf" \
    CXXFLAGS="${CXXFLAGS}" \
    ./configure \
    --prefix=/usr \
    --sysconfdir=/etc/powerdns \
    --sbindir=/usr/bin \
    --with-modules="" \
    --with-dynmodules="gpgsql bind" \
    --docdir=/usr/share/doc/powerdns \
    --with-sqlite3 \
    --with-libsodium \
    --with-libdecaf \
    --enable-tools \
    --enable-ixfrdist \
    --enable-dns-over-tls \
    --disable-dependency-tracking \
    --disable-silent-rules \
    --enable-reproducible \
    --enable-unit-tests \
    --disable-systemd \
    --with-service-user=pdns --with-service-group=pdns && \
    make -j$(nproc) && \
    make DESTDIR=/output/ install && make install && \
    apt-get -qq -y remove libboost-all-dev libtool make pkg-config libpq-dev libpqxx-dev libssl-dev libluajit-5.1-dev python3-venv libsqlite3-dev sqlite3 \
        libsodium-dev libyaml-cpp-dev libcurl4-openssl-dev libfmt-dev \
        autoconf automake ragel bison flex \
        git curl wget cmake && \
    apt-get clean all -y && apt-get autoclean -y

FROM debian:11-slim as builder
ARG PDNS_TAG
ENV PDNS_TAG=${PDNS_TAG:-auth-4.7.2}

COPY --from=pdns-builder /output/ /

ADD devdnsbackend /usr/local/devdnsbackend

RUN set -x && \
    apt-get -qq update && \
    apt-get install -qq -y g++ libboost-all-dev libtool make pkg-config libpq-dev libpqxx-dev libssl-dev libluajit-5.1-dev python3-venv libsqlite3-dev sqlite3 \
        libsodium-dev libyaml-cpp-dev libcurl4-openssl-dev libfmt-dev \
        autoconf automake ragel bison flex \
        git curl wget cmake && \
    apt-get clean all -y && apt-get autoclean -y && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
    # devdnsbackend build & install
    cd /usr/local/devdnsbackend && \
    mkdir -p build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DPDNS_TAG="$PDNS_TAG" .. && \
    make devdnsbackend && \
    make DESTDIR=/output/ install && \
    apt-get -qq -y remove libboost-all-dev libtool make pkg-config libpq-dev libpqxx-dev libssl-dev libluajit-5.1-dev python3-venv libsqlite3-dev sqlite3 \
        libsodium-dev libyaml-cpp-dev libcurl4-openssl-dev libfmt-dev \
        autoconf automake ragel bison flex \
        git curl wget cmake && \
    apt-get clean all -y && apt-get autoclean -y


FROM debian:11-slim

COPY --from=pdns-builder /output/ /
COPY --from=builder /output/ /

RUN set -x && \
    apt-get -qq update && \
    apt-get -y install sqlite3 libboost-program-options1.74.0 libssl1.1 libluajit-5.1 python3 libpqxx-6.4 libsqlite3-0 libyaml-cpp0.6 libcurl4 libsodium23 libfmt7 && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN set -x && \
    useradd pdns && \
    chown -R pdns:pdns /etc/powerdns

ADD init.sh /init.sh
RUN set -x && \
    chmod 0755 /init.sh && \
    mkdir -p /var/lib/powerdns && \
    touch /var/lib/powerdns/supermaster.conf

ADD conf/pdns.d /etc/powerdns/pdns.d
ADD conf/named.conf /etc/powerdns/named.conf
ADD conf/pdns.conf /etc/powerdns/pdns.conf

USER pdns
CMD /usr/bin/pdns_server \
    --daemon=no \
    --guardian=no \
    --control-console \
    --loglevel=9

ENV LD_LIBRARY_PATH /usr/lib/pdns/

ENTRYPOINT ["/init.sh"]
