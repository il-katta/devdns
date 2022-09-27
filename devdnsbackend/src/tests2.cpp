#include "engine.cpp"
#include <iostream>
extern "C" {
#include "uacme/uacme.c"
#include <sys/stat.h>
}

int main() {
    std::cout << "hello" << std::endl;
    check_or_mkdir(true, "/tmp/devdns-test", S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    return 0;
}