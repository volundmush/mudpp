//
// Created by volund on 5/30/20.
//

#ifndef MUDPP_TELNET_H
#define MUDPP_TELNET_H

#include "net.h"

namespace mudpp::net::telnet {

    class TelnetProtocol : public Protocol {
    public:
        explicit TelnetProtocol(TcpConnection *connection);
        void Receive(bytes data) override;
    };

    class TelnetProtocolHandler : public ProtocolHandler {
    public:
        virtual Protocol *WrapConnection(TcpConnection *conn) override;

    };

}




#endif //MUDPP_TELNET_H
