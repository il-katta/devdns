#include "engine.cpp"

using namespace std;

int main() {
    DevDsnEngine engine{
        "devdns",
        "localhost",
        "5432",
        "devdns",
        "devdns",
        ""
    };


    Storage storage {
            "devdns",
            "localhost",
            "5432",
            "devdns",
            "devdns",
            ""
    };

}
