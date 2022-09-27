
#pragma once

#include <string>
#include <map>
#include <sys/types.h>
#include <regex>

#include "pdns/dnsbackend.hh"
#include "pdns/namespaces.hh"
#include "pdns/misc.hh"

#include "engine.cpp"

class DevDnsBackend : public DNSBackend {
public:
    explicit DevDnsBackend(const string &suffix = "");

    //DevDnsBackend(const string &command, int timeout, int abiVersion);

    ~DevDnsBackend() override;

    void lookup(const QType &, const DNSName &qdomain, int zoneId, DNSPacket *p) override;

    bool list(const DNSName &target, int domain_id, bool include_disabled) override;

    bool get(DNSResourceRecord &r) override;

    string directBackendCmd(const string &query) override;

    static DNSBackend *maker();

private:
    std::unique_ptr<Regex> d_regex;
    string soa_record;
    string base_domain;
    DNSName d_qname;
    string q_content;
    QType d_qtype;
    bool d_discard;

    DevDsnEngine engine;


};
