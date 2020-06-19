//
// Created by volund on 5/17/20.
//

#include "net.h"

// NETWORK MANAGER SECTION
mudpp::net::NetworkManager::NetworkManager(boost::asio::io_context &io) : _io_con(io) {

}

void mudpp::net::NetworkManager::RegisterProtocolFactory(std::string name, ProtocolFactory* factory) {
    this->protocol_map[name] = std::shared_ptr<ProtocolFactory>(factory);
}

void mudpp::net::NetworkManager::RegisterHandlerFactory(std::string name, HandlerFactory* factory) {
    this->handler_map[name] = std::shared_ptr<HandlerFactory>(factory);
}

void mudpp::net::NetworkManager::RegisterTransportFactory(std::string name, TransportFactory* factory) {
    this->transport_map[name] = std::shared_ptr<TransportFactory>(factory);
    factory->SetManager(this);
}

void mudpp::net::NetworkManager::CreateServer(std::string name, std::string addr, int port,
        std::string transport_name, std::string protocol_name, std::string handler_name) {

    try {
        if(server_map.contains(name)) {
            throw "Server Name already in use!";
        }
        auto ip = boost::asio::ip::address::from_string(addr);
        auto transport_fac = transport_map[transport_name];
        auto protocol_fac = protocol_map[protocol_name];
        auto handler_fac = handler_map[handler_name];
        auto endp = boost::asio::ip::tcp::endpoint(ip, port);
        auto *server = new Server(name, endp, transport_fac, protocol_fac, handler_fac, *this);
        server_map[name] = std::shared_ptr<Server>(server);
    }
    catch(boost::system::error_code &ec) {
        std::cout << "Something done goofed with creating a server!" << std::endl;
        return;
    }
    catch(const char *err) {
        std::cout << err << std::endl;
        return;
    }
}

void mudpp::net::NetworkManager::StartServer(std::string name) {

}

void mudpp::net::NetworkManager::StopServer(std::string name) {

}

void mudpp::net::NetworkManager::DeleteServer(std::string name) {

}

int mudpp::net::NetworkManager::NextId() {
    next_conn_id++;
    return next_conn_id;
}

mudpp::net::Server::Server(std::string name, boost::asio::ip::tcp::endpoint endpoint,
        std::shared_ptr<TransportFactory> trans_fac, std::shared_ptr<ProtocolFactory> prot_fac,
        std::shared_ptr<HandlerFactory> handler_fac, NetworkManager &manager) : _manager(manager), _endpoint(std::move(endpoint)), _acceptor(manager._io_con) {
    _name = name;
    _transport_factory = trans_fac;
    _protocol_factory = prot_fac;
    _handler_factory = handler_fac;
    _acceptor.open(_endpoint.protocol());
    _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    _acceptor.bind(_endpoint);

}

void mudpp::net::Server::Start() {
    if(!is_listening) {
        is_listening = true;
        Listen();
    }
}

void mudpp::net::Server::Stop() {
    if(is_listening) {
        is_listening = false;
        _acceptor.cancel();
        _acceptor.close();
    }
}

void mudpp::net::Server::Listen() {
    _acceptor.async_accept([&] (const boost::system::error_code& ec, boost::asio::ip::tcp::socket s) {
        auto conn_id = _manager.NextId();
        auto conn = new Connection(std::move(s), *this, conn_id);
        connection_map[conn_id] = std::shared_ptr<Connection>(conn);
        _manager.connection_map[conn_id] = connection_map[conn_id];
        conn->OnConnect();
        Listen();
    });
}

mudpp::net::Connection::Connection(boost::asio::ip::tcp::socket socket, Server &server, int conn_id) :
_socket(std::move(socket)), _server(server) {
    connection_id = conn_id;
    _transport = std::shared_ptr<Transport>(_server._transport_factory->CreateTransport(*this));
    _protocol = std::shared_ptr<Protocol>(_server._protocol_factory->CreateProtocol(*this));
    _handler = std::shared_ptr<Handler>(_server._handler_factory->CreateHandler(*this));

}

void mudpp::net::Connection::OnConnect() {
    // call the transport. It will call OnTransportReady() once it's ready.
    _transport->OnConnect();
}

void mudpp::net::Connection::OnTransportReady() {
    _protocol->OnConnect();
}

void mudpp::net::Connection::OnProtocolReady() {
    _handler->OnConnect();
}

void mudpp::net::Connection::OnHandlerReady() {
    is_initialized = true;
}