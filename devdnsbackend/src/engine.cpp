#include <iostream>
#include <utility>
#include <regex>

#include "uacme.c"

class DevDsnEngine {
public:
    DevDsnEngine() {
        d_base_domain = "";
    }

    explicit DevDsnEngine(std::string base_domain) {
        d_base_domain = std::move(base_domain);
        std::string ip_regex = R"(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4})";
        std::string st_regex = R"(((?!-))(xn--)?[a-z0-9][a-z0-9-_]{0,61})";
        std::string s_regex1 = "^(" + st_regex + "\\.)?(" + ip_regex + ")(\\." + st_regex + "){2,3}\\.$";
        domain_regex1 = std::regex(s_regex1, std::regex_constants::ECMAScript | std::regex_constants::icase);

    }

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


    static void foo()
    {
        check_or_mkdir(true, "foo", S_IRWXU);
    }


private:
    std::string d_base_domain;
    std::regex domain_regex1;
};
