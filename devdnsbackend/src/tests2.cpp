#include "engine.cpp"
#include <iostream>
#include <fmt/core.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <stdio.h>

extern "C" {
#include "uacme/uacme.c"
#include "uacme/msg.h"
}

bool file_exists(const std::string &filepath) {
    return (access(filepath.c_str(), F_OK) != -1);
}

extern "C" {
int main() {
    acme_t a;
    memset(&a, 0, sizeof(a));
    keytype_t key_type = PK_RSA;
    int key_bits = 2048;
    a.email = "test.acme@loopback.it";
    std::vector<const char*> names = {"devdns.loopback.it", "www.devdns.loopback.it"};
    names.push_back(nullptr);
    names.shrink_to_fit();

    g_loglevel = 9;
    a.directory = STAGING_URL;

    char*keyname ;
    asprintf(&keyname, "%s.key", names[0]);

    std::cout << "acme_bootstrap ..." << std::endl;
    if (!acme_bootstrap(&a)) {
        std::cout << "acme_bootstrap fails" << std::endl;
    } else {
        std::cout << "acme_bootstrap ok" << std::endl;
    }

    if (file_exists(keyname)) {
        a.key = key_load(key_type, key_bits, keyname);
    } else {
        a.key = key_load(key_type, key_bits, keyname);
        std::cout << "account_new ..." << std::endl;
        if (!account_new(&a, true)) {
            std::cout << "account_new fails" << std::endl;
        } else {
            std::cout << "account_new ok" << std::endl;
        }
    }

    free(keyname);

    auto csr = csr_gen(const_cast<char *const *>(names.data()), false, a.key);
    if (csr) {
        std::cout << fmt::format("hello {}", csr) << std::endl;
        return 0;
    } else {
        return 1;
    }
}
}