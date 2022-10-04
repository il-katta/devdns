
#pragma once

#include <string>
#include <map>
#include <sys/types.h>
#include <regex>

#include "pdns/dnsbackend.hh"
#include "pdns/namespaces.hh"
#include "pdns/misc.hh"

#include "engine.cpp"
#include "modules/gpgsqlbackend/gpgsqlbackend.hh"

class DevDnsBackend : public gPgSQLBackend {
public:
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
    string soa_record;
    string base_domain;
    string keystore_directory;
    DNSName d_qname;
    string d_sremote;
    string q_content;
    QType d_qtype;
    bool d_match;
    bool d_answered;
    std::optional<ComboAddress> d_remote;

    DevDsnEngine engine;

    void dlog(std::string message);

    bool get_devdns(DNSResourceRecord &r);

    bool get_sql(DNSResourceRecord &r);

    bool generateCertificate_callback(
            const std::string &type,
            const std::string &domainName,
            const std::string &token,
            const std::string &keyAuthorization
    );

    bool getZoneFromDnsRecord(std::string domainName, DomainInfo &di);
};
