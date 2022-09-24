#include <iostream>
#include "devdnsbackend.hpp"
#include "devdnsbackend.cpp"

int main() {
    std::cout << "running test ..." << std::endl;
    DevDnsBackend devdns = DevDnsBackend();
    devdns.abortTransaction();
    return 0;
}