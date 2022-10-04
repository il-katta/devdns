#include <iostream>
#include <fmt/core.h>
#include "engine.cpp"

int main() {

    DevDsnEngine engine{};

    std::string email = "acme@devdns.sh";
    std::string domain = "example.devdns.sh";
    std::vector<string> names = {domain};
    std::optional<acme_lw::Certificate> certificate = engine.generateCertificate(
            email,
            names,
            [](
                    const std::string &type,
                    const std::string &domainName,
                    const std::string &token,
                    const std::string &keyAuthorization
            ) {
                return false;
            },
            "/tmp",
            true
    );

    std::string response;
    if (!engine.check_request("1.1.1.1.a.devdns.sh.", response)) {
        std::cout << "check fail [1]" << std::endl;
        return 1;
    }
    if (response != "1.2.3.4") {
        std::cout << fmt::format("'{}' != '{}'", response, "1.2.3.4") << std::endl;
        return 1;
    }
    if (engine.check_request("1.2.3.a.example.test.", response)) {
        std::cout << "check fail [2]" << std::endl;
        return 1;
    }

    if (!engine.check_request("abc.1.2.3.4.a.example.test.", response)) {
        std::cout << "check fail [3]" << std::endl;
        return 1;
    }
    return 0;
}