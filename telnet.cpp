//
// Created by volund on 5/30/20.
//

#include "telnet.h"

mudpp::net::telnet::TelnetProtocol::TelnetProtocol(mudpp::net::TcpConnection *connection) : Protocol(connection) {
    // Not sure what else this'll do just yet.
}

mudpp::net::Protocol *mudpp::net::telnet::TelnetProtocolHandler::WrapConnection(mudpp::net::TcpConnection *conn) {
    return new TelnetProtocol(conn);
}

void mudpp::net::telnet::TelnetProtocol::Receive(mudpp::net::bytes data) {
    // Just gonna echo for the moment.
    Send(data);
}


