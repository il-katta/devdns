
#pragma once

#include <string>
#include <map>
#include <sys/types.h>
#include <regex>

#include "pdns/dnsbackend.hh"
#include "pdns/namespaces.hh"
#include "pdns/misc.hh"


class DevDnsBackend : public DNSBackend {
public:
    DevDnsBackend(const string &suffix = "");

    DevDnsBackend(const string &command, int timeout, int abiVersion);

    ~DevDnsBackend();

    void lookup(const QType &, const DNSName &qdomain, int zoneId, DNSPacket *p = nullptr) override;

    bool list(const DNSName &target, int domain_id, bool include_disabled = false) override;

    bool get(DNSResourceRecord &r) override;

    string directBackendCmd(const string &query) override;

    static DNSBackend *maker();

private:
    std::unique_ptr<Regex> d_regex;
    DNSName d_qname;
    string q_content;
    QType d_qtype;
    bool d_discard;

    std::regex domain_regex1;
    std::regex domain_regex2;
};
