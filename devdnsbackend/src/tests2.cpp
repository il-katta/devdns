#include <iostream>
#include <fstream>
#ifdef __cplusplus
extern "C" {
#endif
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>
#include "uacme/uacme.c"
#include "uacme/msg.h"
#include "uacme/curlwrap.h"
#ifdef __cplusplus
}
#endif

#include "engine.cpp"
#include "acme-lw.h"
#include <fmt/core.h>
#include <string>
#include <functional>



bool file_exists(const std::string &filepath) {
    return (access(filepath.c_str(), F_OK) != -1);
}

acme_t a;

void _out(char *csr, char *keyname, char **names) {
    free(keyname);
    json_free(a.json);
    json_free(a.account);
    json_free(a.dir);
    json_free(a.order);
    free(a.nonce);
    free(a.kid);
    free(a.headers);
    free(a.body);
    free(a.type);
    free(a.keyprefix);
    free(a.certprefix);
    //if (a.key)
    //    privkey_deinit(a.key);
    //if (key)
    //    privkey_deinit(key);
    free(csr);
    crypto_deinit();
    //curl_global_cleanup();
    /*
    if (names) {
        for (int i = 0; names[i]; i++)
            free(names[i]);
        free(names);
    }
    */
    acme_lw::AcmeClient::teardown();
}
bool callback(const std::string &type,
              const std::string &domainName,
              const std::string &token,
              const std::string &keyAuthorization) {
    return type == "dns-01";
}

std::string readFile(const std::string &fileName) {
    std::ifstream f(fileName);
    if (f.fail()) {
        std::cout << "Unable to open " << fileName << "\n";
        exit(1);
    }

    std::stringstream ss;
    ss << f.rdbuf();
    f.close();
    if (f.fail()) {
        std::cout << "Failure reading " << fileName << "\n";
        exit(1);
    }

    return ss.str();
}

int main() {
    memset(&a, 0, sizeof(a));
    keytype_t key_type = PK_RSA;
    int key_bits = 2048;
    a.email = "test.acme@loopback.it";
    std::vector<const char *> names = {"devdns.loopback.it", "www.devdns.loopback.it"};
    names.push_back(nullptr);
    names.shrink_to_fit();
    char ** _names = const_cast<char **>(names.data());

    g_loglevel = 9;
    a.directory = STAGING_URL;

    char *keyname;
    asprintf(&keyname, "%s.key", names[0]);

    std::cout << "acme_bootstrap ..." << std::endl;
    if (!acme_bootstrap(&a)) {
        std::cout << "acme_bootstrap fails" << std::endl;
        _out(nullptr, keyname, _names);
        return 1;
    }
    std::cout << "acme_bootstrap ok" << std::endl;

    if (file_exists(keyname)) {
        a.key = key_load(key_type, key_bits, keyname);
        account_retrieve(&a);
    } else {
        a.key = key_load(key_type, key_bits, keyname);
        std::cout << "account_new ..." << std::endl;
        if (!account_new(&a, true)) {
            std::cout << "account_new fails" << std::endl;
        } else {
            std::cout << "account_new ok" << std::endl;
        }
    }
    acme_lw::AcmeClient::init();
    acme_lw::AcmeClient acme{readFile(keyname)};
    std::list<std::string> certificateNames = {names[0], names[1]};
    std::function<bool(const std::string &, const std::string &, const std::string &, const std::string &)> cb = [](const std::string &type,
                 const std::string &domainName,
                 const std::string &token,
                 const std::string &keyAuthorization) {
        return callback(type,domainName,token, keyAuthorization);
    };
    auto certificate = acme.issueCertificate(certificateNames, cb);

    _out(nullptr, keyname, _names);
    return 0;

    auto csr = csr_gen(_names, false, a.key);
    if (!csr) {
        std::cout << "csr_gen fail" << std::endl;
        _out(csr, keyname, _names);
        return 1;
    }

    if (!cert_issue(&a, _names, csr)) {
        std::cout << "cert_issue fail" << std::endl;
        _out(csr, keyname, _names);
        return 1;
    }
    std::cout << fmt::format("hello {}", csr) << std::endl;
    return 0;
}
