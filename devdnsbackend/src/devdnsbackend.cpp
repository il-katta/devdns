#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <map>
#include <iostream>
#include <fmt/core.h>

#include "pdns/dns.hh"
#include "pdns/dnsbackend.hh"
#include "pdns/dnspacket.hh"
#include "pdns/logger.hh"
#include "devdnsbackend.hpp"

static const char *kBackendId = "[DEVDNSBackend]";

void log(Logger::Urgency level, const std::string &message) {
    //g_log << level << kBackendId << " " << message << std::endl;
}

DevDnsBackend::DevDnsBackend(const string &suffix) {
    log(Logger::Info, "DevDnsBackend " + suffix);
    signal(SIGCHLD, SIG_IGN);
    setArgPrefix("devdns" + suffix);
    soa_record = getArg("soa-record");
    base_domain = getArg("domain");
    engine = DevDsnEngine(base_domain);
    log(Logger::Info, fmt::format("soa record '{}'", soa_record));
    d_discard = false;
}


void DevDnsBackend::lookup(const QType &qtype, const DNSName &qname, int zoneId, DNSPacket *pkt_p) {
    auto s_name = qname.toString();
    //log(Logger::Info, "lookup " + qname.toString());
    if (!engine.check_request(s_name, q_content)) {
        d_discard = true;
    } else {
        d_discard = false;
        d_qname = qname;
        d_qtype = qtype;
    }
}

bool DevDnsBackend::get(DNSResourceRecord &r) {
    //log(Logger::Info, fmt::format("get {} {}", d_qtype.toString(), d_qname.toString()));
    if (d_discard) return false;
    d_discard = true; // only one response per record
    if (d_qname.empty()) {
        log(Logger::Info, "get [NO NAME]");
        return false;
    }
    if (q_content.empty()) {
        log(Logger::Info, "get [NO CONTENT]");
        return false;
    }
    r.qname = d_qname;
    r.qtype = d_qtype;
    r.ttl = 3600;
    r.auth = true;
    r.domain_id = 60;
    switch (d_qtype.getCode()) {
        case QType::SOA:
            r.content = soa_record;
            break;
        case QType::A:
            r.content = q_content;
            break;
        case QType::ANY:
            r.content = q_content;
            r.qtype = QType::A;
            break;
        default:
            log(Logger::Info, "get invalid type");
            return false;
    }
    //log(Logger::Info, fmt::format("get return '{}'", r.content));
    return true;
}


bool DevDnsBackend::list(const DNSName &target, int inZoneId, bool include_disabled) {
    log(Logger::Info, fmt::format("list {}", target.toString()));
    return false;
}

string DevDnsBackend::directBackendCmd(const string &query) {
    log(Logger::Info, "directBackendCmd " + query);
    return "";
}

//! For the dynamic loader
DNSBackend *DevDnsBackend::maker() {
    log(Logger::Info, "maker");
    try {
        return new DevDnsBackend();
    }
    catch (...) {
        log(Logger::Error, "Unable to instantiate a DevDnsBackend!");
        return nullptr;
    }
}

DevDnsBackend::~DevDnsBackend() {
    log(Logger::Info, "goodbye");
}


//
// Magic class that is activated when the dynamic library is loaded
//

class DevdnsFactory : public BackendFactory {
public:
    DevdnsFactory() : BackendFactory("devdns") {
        log(Logger::Info, "DevdnsFactory init");
    }

    void declareArguments(const string &suffix = "") override {
        log(Logger::Info, "declareArguments " + suffix);
        declare(suffix, "soa-record", "soa record",
                "a.misconfigured.dns.server.invalid hostmaster.example.com 0 10800 3600 604800 3600");
        declare(suffix, "domain", "base domain ( Example: dev.example.com )", "");
    }

    DNSBackend *make(const string &suffix) override {
        log(Logger::Info, "DevdnsFactory make " + suffix);
        return new DevDnsBackend(suffix);
    }
};

class DevdnsLoader {
public:
    DevdnsLoader() {
        BackendMakers().report(new DevdnsFactory);
        g_log << Logger::Info << kBackendId << " This is the dev dns backend version "
              #ifndef REPRODUCIBLE
              << " (" __DATE__ " " __TIME__ ")"
              #endif
              << " reporting" << endl;
    }
};

static DevdnsLoader devdnsloader;
