FROM debian:latest as builder

ADD devdnsbackend /usr/local/devdnsbackend

RUN set -x && \
    apt-get -qq update && \
    apt-get install -qq -y g++ libboost-all-dev libtool make pkg-config libpq-dev libpqxx-dev libssl-dev libluajit-5.1-dev python3-venv \
        autoconf automake ragel bison flex \
        git curl wget cmake \
        libsqlite3-dev sqlite3 libsodium-dev libyaml-cpp-dev libcurl4-openssl-dev libfmt-dev \
        libgnutls28-dev && \
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
    git clone https://github.com/PowerDNS/pdns.git --branch auth-4.6.3 /usr/local/pdns && \
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
        --with-dynmodules="gpgsql pipe" \
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
    make DESTDIR=/output/ install && make install &&\
    # uacme
    git clone https://git.loopback.it/andrea/uacme.git --branch lib /usr/local/uacme && \
    cd /usr/local/uacme && \
    autoreconf -vi && \
    ./configure --prefix=/usr --disable-maintainer-mode --disable-docs && \
    make -j$(nproc) && \
    make DESTDIR=/output/ install && make install  && \
    # devdnsbackend build & install
    cd /usr/local/devdnsbackend && \
    mkdir -p build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr .. && \
    make devdnsbackend && \
    make DESTDIR=/output/ install && \
    apt-get -qq -y remove g++ libboost-all-dev libtool make pkg-config libssl-dev libluajit-5.1-dev python3-venv \
        autoconf automake ragel bison flex git curl wget cmake libsqlite3-dev libpqxx-dev sqlite3 libsodium-dev libyaml-cpp-dev libcurl4-openssl-dev && \
    apt-get clean all -y && apt-get autoclean -y


FROM debian:latest
COPY --from=builder /output/ /

RUN set -x && \
    apt-get -qq update && \
    apt-get -y install sqlite3 libboost-program-options1.74.0 libssl1.1 libluajit-5.1 python3 libpqxx-6.4 libsqlite3-0 libyaml-cpp0.6 libcurl4 libsodium23 libfmt7 && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN set -x && \
    mkdir -p /etc/powerdns && \
    sqlite3 /etc/powerdns/pdns.sqlite3 < /usr/share/doc/powerdns/schema.sqlite3.sql && \
    useradd pdns && \
    chown -R pdns:pdns /etc/powerdns

ADD init.sh /init.sh
RUN set -x && \
    chmod 0755 /init.sh && \
    mkdir -p /var/lib/powerdns && \
    touch /var/lib/powerdns/supermaster.conf
ENTRYPOINT ["/init.sh"]

ADD conf/pdns.d /etc/powerdns/pdns.d
ADD conf/named.conf /etc/powerdns/named.conf
ADD conf/pdns.conf /etc/powerdns/pdns.conf

USER pdns
CMD /usr/bin/pdns_server \
    --daemon=no \
    --guardian=no \
    --control-console \
    --loglevel=9
