#include <iostream>
#include <acme-lw.h>


bool callback(const std::string &type,
              const std::string &domainName,
              const std::string &token,
              const std::string &keyAuthorization) {
    return false;
}

int main() {
    acme_lw::AcmeClient acme{""};
    auto certificate = acme.issueCertificate({"test.loopback.it"}, callback);
}