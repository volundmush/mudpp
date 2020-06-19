//
// Created by volund on 5/17/20.
//

#ifndef MUDPP_NET_H
#define MUDPP_NET_H

#include <iostream>
#include <unordered_map>
#include <string>
#include <sys/epoll.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <boost/asio.hpp>
#include <utility>

namespace mudpp::net {

    typedef std::vector<uint8_t> bytes;

    class ProtocolFactory;
    class TransportFactory;
    class HandlerFactory;
    class ServerManager;
    class Server;
    class Protocol;
    class Transport;
    class Handler;
    class NetworkManager;
    class Connection;


    class Transport {
    public:
        Transport(Connection& conn);
        virtual void Send(bytes data);
        virtual void Receive(bytes data);
        virtual void OnConnect();
    private:
        void ReadFromSocket();
        void SendToSocket();
        Connection& _conn;
    };

    class TransportFactory {
    public:
        void SetManager(NetworkManager *manager);
        virtual Transport* CreateTransport(Connection &conn) = 0;
    private:
        NetworkManager* _manager;
    };

    class TcpTransport : public Transport {
    public:
        TcpTransport(Connection& conn);
    };

    class TlsTransport : public TcpTransport {
    public:
        TlsTransport(Connection& conn);
        void Send(bytes data) override;
        void Receive(bytes data) override;
    };

    class TcpTransportFactory : public TransportFactory {
    public:
        Transport* CreateTransport(Connection& conn) override;
    };

    class TlsTransportFactory : public TransportFactory {
    public:
        Transport* CreateTransport(Connection& conn) override;
    };

    class Protocol {
    public:
        Protocol(Connection &conn);
        virtual void Receive(bytes data) = 0;
        virtual void OnConnect() = 0;
        virtual void OnDisconnect() = 0;
        virtual void Send(bytes data) = 0;
    private:
        Connection &_conn;
    };

    class ProtocolFactory {
    public:
        virtual Protocol* CreateProtocol(Connection& conn);
    };

    class Handler {
    public:
        Handler(Connection &conn);
        virtual void OnConnect();
    private:
        Connection &_conn;
    };

    class HandlerFactory {
    public:
        virtual Handler* CreateHandler(Connection& conn);
    };

    class Connection {
        public:
            Connection(boost::asio::ip::tcp::socket socket, Server &server, int conn_id);
            void OnConnect();
            void OnTransportReady();
            void OnProtocolReady();
            void OnHandlerReady();
            void OnDisconnect();
        private:
            int connection_id;
            bool is_initialized = false;
            bool is_tearing_down = false;
            boost::asio::ip::tcp::socket _socket;
            Server& _server;
            std::shared_ptr<Transport> _transport;
            std::shared_ptr<Handler> _handler;
            std::shared_ptr<Protocol> _protocol;
            friend class Protocol;
            friend class Transport;
            friend class Handler;
    };

    class Server {
        public:
            Server(std::string name, boost::asio::ip::tcp::endpoint endpoint, std::shared_ptr<TransportFactory> trans_fac,
                    std::shared_ptr<ProtocolFactory> prot_fac, std::shared_ptr<HandlerFactory> handler_fac, NetworkManager &manager);
            void Start();
            void Stop();
            void Listen();
            void Accept(boost::system::error_code& ec, boost::asio::ip::tcp::socket sock);
        private:
            bool is_listening = false;
            std::shared_ptr<HandlerFactory> _handler_factory;
            std::shared_ptr<ProtocolFactory> _protocol_factory;
            std::shared_ptr<TransportFactory> _transport_factory;
            boost::asio::ip::tcp::acceptor _acceptor;
            std::string _name;
            boost::asio::ip::tcp::endpoint _endpoint;
            NetworkManager &_manager;
            std::unordered_map<int, std::shared_ptr<Connection>> connection_map;
            friend class Connection;
    };

    class NetworkManager {
        public:
            NetworkManager(boost::asio::io_context &io);
            void RegisterProtocolFactory(std::string name, ProtocolFactory* factory);
            void RegisterHandlerFactory(std::string name, HandlerFactory* factory);
            void RegisterTransportFactory(std::string name, TransportFactory* factory);
            void CreateServer(std::string name, std::string addr, int port, std::string transport_name,
                    std::string protocol_name, std::string handler_name);
            void DeleteServer(std::string name);
            void StartServer(std::string name);
            void StopServer(std::string name);
            void RegisterConnection();
            int NextId();
        private:
            int next_conn_id = 0;
            std::unordered_map<std::string, std::shared_ptr<ProtocolFactory>> protocol_map;
            std::unordered_map<std::string, std::shared_ptr<HandlerFactory>> handler_map;
            std::unordered_map<std::string, std::shared_ptr<TransportFactory>> transport_map;
            std::unordered_map<std::string, std::shared_ptr<Server>> server_map;
            std::unordered_map<int, std::shared_ptr<Connection>> connection_map;
            boost::asio::io_context& _io_con;
            friend class Server;
            friend class Connection;
    };
}
#endif //MUDPP_NET_H
