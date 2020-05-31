#include <iostream>
#include "net.h"
#include "telnet.h"


int main(int argc, char** argv) {

    auto nman = new mudpp::net::NetworkManager();
    nman->RegisterProtocolHandler("telnet", new mudpp::net::telnet::TelnetProtocolHandler());
    nman->AddListeningServer("telnet", "10.0.0.226", 4200, false, "telnet");

    while(true)
    {
        nman->AcceptNewConnections();
        nman->ProcessNewInput();
    }

    std::cout << *argv << std::endl;
    return 0;
}
