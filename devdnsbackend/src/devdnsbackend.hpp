
#pragma once

#include <string>
#include <map>
#include <sys/types.h>
#include <regex>

#include "pdns/namespaces.hh"
#include "pdns/misc.hh"
#include "fmt/core.h"
#include "engine.cpp"
#include "modules/gpgsqlbackend/gpgsqlbackend.hh"

class DevDnsBackend : public gPgSQLBackend {
public:
    explicit DevDnsBackend(const string &mode, const string &suffix, DevDsnEngine engine);
    explicit DevDnsBackend(const string &mode, const string &suffix);

    //DevDnsBackend(const string &command, int timeout, int abiVersion);

    ~DevDnsBackend() override;

    void lookup(const QType &, const DNSName &qdomain, int zoneId, DNSPacket *p) override;

    bool list(const DNSName &target, int domain_id, bool include_disabled) override;

    bool get(DNSResourceRecord &r) override;

    string directBackendCmd(const string &query) override;

    static DNSBackend *maker();

private:
    std::unique_ptr<Regex> d_regex;
    DNSName d_qname;
    string d_sremote;
    string q_content;
    QType d_qtype;
    bool d_match;
    bool d_answered;
    std::optional<ComboAddress> d_remote;

    DevDsnEngine engine;

    void dlog(std::string message);

    template <typename... T>
    void dlog(std::string str, T&&... args);

    void lookup_devdns(const QType &, const DNSName &qdomain, int zoneId, DNSPacket *p);

    void lookup_sql(const QType &, const DNSName &qdomain, int zoneId, DNSPacket *p);

    bool get_devdns(DNSResourceRecord &r);

    bool get_sql(DNSResourceRecord &r);
};
