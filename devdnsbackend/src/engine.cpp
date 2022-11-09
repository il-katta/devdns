#pragma once

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pwd.h>
#include <string>
#include <functional>
#include <regex>
#include <utility>
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
        std::cout << "DevDsnEngine init" << std::endl;
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
        myip_regex1 = std::regex(
                "^my\\.ip(\\." + st_regex + "){2,3}\\.$",
                std::regex_constants::ECMAScript | std::regex_constants::icase
        );
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
        if (std::regex_search(s_name, matches, domain_regex1)) {
            ipaddress = matches[4].str();
            return true;
        }
        if (std::regex_search(s_name, matches, myip_regex1)) {
            ipaddress = "myip";
            return true;
        }
        return false;
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


private:
    std::string d_base_domain;
    std::regex domain_regex1;
    std::regex myip_regex1;
    std::unique_ptr<Storage> storage;

    string d_db_name;
    string d_db_host;
    string d_db_port;
    string d_db_user;
    string d_db_password;
    string d_db_extra_connection_parameters;
};
