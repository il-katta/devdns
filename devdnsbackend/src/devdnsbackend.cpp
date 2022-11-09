#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define PDNS_BACKEND

#include <string>
#include <map>
#include <iostream>
#include <fmt/core.h>

#include "pdns/dns.hh"
#include "pdns/dnsbackend.hh"
#include "pdns/dnspacket.hh"
#include "pdns/logger.hh"
#include "devdnsbackend.hpp"
#include "modules/gpgsqlbackend/gpgsqlbackend.cc"
#include "storage.cpp"

#include <openssl/evp.h>

DevDnsBackend::DevDnsBackend(
        const string &mode,
        const string &suffix,
        DevDsnEngine engine
) : gPgSQLBackend(mode, suffix), engine(engine) {
    log_info("{}@DevDnsBackend:{}", engine.whoami(), suffix);
    signal(SIGCHLD, SIG_IGN);
    setArgPrefix("devdns" + suffix);
    d_answered = false;
    d_match = false;
}

DevDnsBackend::DevDnsBackend(const string &mode, const string &suffix) : gPgSQLBackend(mode, suffix),
                                                                         engine(getArg("dbname"),
                                                                                getArg("host"),
                                                                                getArg("port"),
                                                                                getArg("user"),
                                                                                getArg("password"),
                                                                                getArg("extra-connection-parameters"),
                                                                                getArg("domain")) {
    log_info("{}@DevDnsBackend:{}", engine.whoami(), suffix);
    signal(SIGCHLD, SIG_IGN);
    setArgPrefix("devdns" + suffix);

    d_answered = false;
    d_match = false;
}

void DevDnsBackend::lookup_devdns(const QType &qtype, const DNSName &qname, int zoneId, DNSPacket *pkt_p) {
    d_qname = qname;
    d_qtype = qtype;
    d_answered = false;
    std::string d_sname = d_qname.empty() ? "" : d_qname.toString();
    if (pkt_p) {
        d_remote = std::make_optional<ComboAddress>(pkt_p->getRemote());
    } else {
        d_remote.reset();
    }
    d_sremote = d_remote.has_value() ? d_remote->toString() : "";
    dlog("lookup()");

    switch (d_qtype.getCode()) {
        case QType::A:
        case QType::ANY:
        case QType::SOA:
        case QType::TXT:
            dlog("lookup() check_request ...");
            d_match = engine.check_request(d_sname, q_content);
            if (d_match)
                dlog("lookup() check_request match");
            else
                dlog("lookup() check_request NOT match");
            break;
        default:
            dlog("lookup() lookup type not");
            d_match = false;
            break;
    }
}

void DevDnsBackend::lookup_sql(const QType &qtype, const DNSName &qname, int zoneId, DNSPacket *pkt_p) {
    gPgSQLBackend::lookup(qtype, qname, zoneId, pkt_p);
}

void DevDnsBackend::lookup(const QType &qtype, const DNSName &qname, int zoneId, DNSPacket *pkt_p) {
    dlog("lookup() using gPgSQL backend");
    lookup_sql(qtype, qname, zoneId, pkt_p);
    dlog("lookup() using devdns backend");
    lookup_devdns(qtype, qname, zoneId, pkt_p);
}

bool DevDnsBackend::get(DNSResourceRecord &r) {
    dlog("get()");
    if (get_devdns(r)) {
        return true;
    }
    return get_sql(r);
}

bool DevDnsBackend::get_devdns(DNSResourceRecord &r) {
    if (d_answered) return false;
    if (d_qname.empty()) return false;
    if (q_content.empty()) return false;
    r.disabled = false;
    r.qname = d_qname;
    r.qtype = d_qtype;
    r.ttl = 3600;
    r.auth = true;
    r.domain_id = 60;
    switch (d_qtype.getCode()) {
        case QType::A:
        case QType::ANY:
            if (q_content.compare("myip") == 0) {
                r.content = d_sremote;
                r.ttl = 0;
            } else {
                r.content = q_content;
            }
            r.qtype = QType::A;
            dlog("get() return '{}'", r.content);
            d_answered = false;
            d_qtype = QType::TXT;
            return true;
        default:
            return false;
    }
}

bool DevDnsBackend::get_sql(DNSResourceRecord &r) {
    //if (!d_match)
    //    dlog("get() using gPgSQL backend [not match]");
    if (d_qname.empty())
        dlog("get() using gPgSQL backend [d_qname empty]");
    if (q_content.empty())
        dlog("get() using gPgSQL backend [q_content empty]");
    bool ret = gPgSQLBackend::get(r);
    if (ret) {
        dlog("get() using gPgSQL backend. response: {}", r.content);
    }
    return ret;
}

bool DevDnsBackend::list(const DNSName &target, int inZoneId, bool include_disabled) {
    log(Logger::Debug, fmt::format("list {}", target.toString()));
    return gPgSQLBackend::list(target, inZoneId, include_disabled);
}

string DevDnsBackend::directBackendCmd(const string &query) {
    log(Logger::Debug, "directBackendCmd " + query);
    return gPgSQLBackend::directBackendCmd(query);
}

//! For the dynamic loader
DNSBackend *DevDnsBackend::maker() {
    log(Logger::Debug, "maker");
    try {
        return new DevDnsBackend("devdns", "");
    }
    catch (...) {
        log(Logger::Error, "Unable to instantiate a DevDnsBackend!");
        return nullptr;
    }
}

DevDnsBackend::~DevDnsBackend() {
    log(Logger::Info, "goodbye");
}

void DevDnsBackend::dlog(std::string message) {
    std::string d_sname = d_qname.empty() ? "" : d_qname.toString();
    if (d_sremote.empty()) {
        log(Logger::Debug, fmt::format("'{}' - {} - {}", d_sname, d_qtype.toString(), message));
    } else {
        log(Logger::Debug, fmt::format("'{}' - {} - {} - {}", d_sname, d_qtype.toString(), d_sremote, message));
    }
}

template<typename... T>
void DevDnsBackend::dlog(std::string str, T &&... args) {
    dlog(fmt::format(str, args...));
}

//
// Magic class that is activated when the dynamic library is loaded
//

class DevdnsFactory : public gPgSQLFactory {
public:
    DevdnsFactory(const string &mode) : gPgSQLFactory(mode), d_mode(mode) {
        log(Logger::Debug, fmt::format("DevdnsFactory init {}", mode));
    }

    void declareArguments(const string &suffix = "") override {
        log(Logger::Debug, "declareArguments " + suffix);
        gPgSQLFactory::declareArguments(suffix);
        declare(suffix, "domain", "base domain ( Example: dev.example.com )", "");
    }

    DNSBackend *make(const string &suffix = "") override {
        log(Logger::Debug, "DevdnsFactory make " + suffix);
        auto pDevDnsBackend = new DevDnsBackend(d_mode, suffix);
        return pDevDnsBackend;
    }

private:
    const string d_mode;
};

class DevdnsLoader {
public:
    DevdnsLoader() {
        BackendMakers().report(new DevdnsFactory("devdns"));
        g_log << Logger::Debug << kBackendId << " This is the dev dns backend version "
              #ifndef REPRODUCIBLE
              << " (" __DATE__ " " __TIME__ ")"
              #endif
              << " reporting" << endl;
    }
};

static DevdnsLoader devdnsloader;
