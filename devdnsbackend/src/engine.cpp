#pragma once

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

#include <stdlib.h>
#include <pwd.h>
#ifdef __cplusplus
}
#endif

#include <string>
#include <functional>
#include <regex>
#include <utility>
#include <optional>

#include "acme-lw.h"
#include "logger.h"
#include "storage.cpp"

using namespace std;

static const char *kBackendId = "[DEVDNSBackend]";

class DevDsnEngine {
public:
    explicit DevDsnEngine(
            string db_name,
            string db_host,
            string db_port,
            string db_user,
            string db_password,
            string db_extra_connection_parameters,
            string base_domain = ""
    ) {
        d_base_domain = std::move(base_domain);
        d_db_name = db_name;
        d_db_host = db_host;
        d_db_port = db_port;
        d_db_user = db_user;
        d_db_password = db_password;
        d_db_extra_connection_parameters = db_extra_connection_parameters;
        std::string ip_regex = R"(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4})";
        std::string st_regex = R"(((?!-))(xn--)?[a-z0-9][a-z0-9-_]{0,61})";
        std::string s_regex1 = "^(" + st_regex + "\\.)?(" + ip_regex + ")(\\." + st_regex + "){2,3}\\.$";
        domain_regex1 = std::regex(s_regex1, std::regex_constants::ECMAScript | std::regex_constants::icase);
        storage = std::make_unique<Storage>(db_name, db_host, db_port, db_user, db_password,
                                            db_extra_connection_parameters);
    }

    DevDsnEngine(const DevDsnEngine &engine) : DevDsnEngine{
            engine.d_db_name,
            engine.d_db_host,
            engine.d_db_user,
            engine.d_db_password,
            engine.d_db_extra_connection_parameters,
            engine.d_base_domain
    } {}

    bool check_request(const std::string &s_name, std::string &ipaddress) {
        if (!d_base_domain.empty()) {
            if (!str_ends_with(s_name, d_base_domain + ".")) {
                return false;
            }
        }
        std::smatch matches;
        if (!std::regex_search(s_name, matches, domain_regex1)) {
            return false;
        }
        ipaddress = matches[4].str();;
        return true;
    }

    static bool str_ends_with(std::string const &str, std::string const &end) {
        if (str.length() >= end.length()) {
            return str.compare(str.length() - end.length(), end.length(), end) == 0;
        } else {
            return false;
        }
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

    static bool file_exists(const std::string &filepath) {
        return (access(filepath.c_str(), F_OK) != -1);
    }


    std::optional<acme_lw::Certificate> generateCertificate(
            const string &email,
            vector<string> names,
            const function<bool(const string &, const string &, const string &, const string &)> &callback,
            const string &keystore_directory,
            bool staging = false
    ) {
        memset(&a, 0, sizeof(a));
        keytype_t key_type = PK_RSA;
        int key_bits = 2048;
        a.email = email.c_str();
        if (staging) {
            a.directory = STAGING_URL;
        } else {
            a.directory = PRODUCTION_URL;
        }
        g_loglevel = 9;
        char *keyname;
        asprintf(&keyname, "%s/%s.key", keystore_directory.c_str(), names[0].c_str());
        std::cout << "acme_bootstrap " << names[0] << " ..." << std::endl;
        if (!acme_bootstrap(&a)) {
            std::cout << "acme_bootstrap fails" << std::endl;
            generateCertificate_out(keyname);
            return std::make_optional<acme_lw::Certificate>();
        }
        std::cout << "acme_bootstrap ok" << std::endl;

        if (file_exists(keyname)) {
            a.key = key_load(key_type, key_bits, keyname);
            account_retrieve(&a);
        } else {
            a.key = key_load(key_type, key_bits, keyname); // generate a new one
            std::cout << "account_new ..." << std::endl;
            if (!account_new(&a, true)) {
                std::cout << "account_new fails" << std::endl;
            } else {
                std::cout << "account_new ok" << std::endl;
            }
        }

        acme_lw::AcmeClient::init();
        auto key = readFile(keyname);
        acme_lw::AcmeClient acme{key};
        list<std::string> certificateNames{};
        for (const string &name: names) {
            certificateNames.insert(certificateNames.end(), name);
        }
        acme_lw::Certificate certificate = acme.issueCertificate(certificateNames, callback);

        generateCertificate_out(keyname);

        return std::make_optional<acme_lw::Certificate>(certificate);
    }


    string whoami() {
        struct passwd *pw;
        uid_t uid;
        uid = geteuid();
        pw = getpwuid(uid);
        if (pw) {
            return pw->pw_name;
        }
        return "";
    }

    static inline string str_trim_end(string s, char c = 0) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [c](unsigned char ch) {
            if (c == 0)
                return !std::isspace(ch);
            else
                return ch != c;
        }).base(), s.end());
        return s;
    }

    ~DevDsnEngine() {
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
        crypto_deinit();
        acme_lw::AcmeClient::teardown();
    }


private:
    acme_t a{};
    std::string d_base_domain;
    std::regex domain_regex1;
    std::unique_ptr<Storage> storage;

    string d_db_name;
    string d_db_host;
    string d_db_port;
    string d_db_user;
    string d_db_password;
    string d_db_extra_connection_parameters;

    void generateCertificate_out(char *keyname = nullptr) const {
        if (keyname) {
            free(keyname);
        }
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
};
