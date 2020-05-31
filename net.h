//
// Created by volund on 5/17/20.
//

#ifndef MUDPP_NET_H
#define MUDPP_NET_H

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

namespace mudpp::net {

        typedef std::vector<uint8_t> bytes;

        class ProtocolHandler;
        class NetworkManager;
        class Protocol;
        class GameConnection;

        class TcpConnection {
        public:
            TcpConnection(int socket, std::string addr);
            ~TcpConnection();
            void ReadFromSocket();
            void SendToSocket();
            void SetProtocol(Protocol *prot);
            virtual void Send(bytes data);
            virtual void Receive(bytes data);
        private:
            int socket;
            std::string addr;
            std::shared_ptr<Protocol> protocol;
        };

        class TlsConnection : public TcpConnection {
        public:
            TlsConnection(int socket, std::string addr);
            ~TlsConnection();
            void Send(bytes data) override;
            void Receive(bytes data) override;
        };

        class Protocol {
        public:
            explicit Protocol(TcpConnection *connection);
            ~Protocol();
            virtual void Receive(bytes data);
            virtual void OnConnect();
            void SetGameConnection(GameConnection *gconn);
            void Send(bytes data);
        private:
            std::shared_ptr<TcpConnection> conn;
            std::shared_ptr<GameConnection> gconn;
        };

        class GameConnection {
        public:
            explicit GameConnection(Protocol *prot);
            ~GameConnection();
        private:
            std::string address;
            std::shared_ptr<Protocol> prot;
        };

        class ProtocolHandler {
        public:
            void SetNetworkManager(NetworkManager *manager);
            virtual Protocol *WrapConnection(TcpConnection *conn) = 0;
        private:
            std::shared_ptr<NetworkManager> manager;
        };

        class ListeningServer {
        public:
            ListeningServer(std::string name, std::string addr, int port, bool tls, std::shared_ptr<ProtocolHandler> handler, NetworkManager *manager);
            ~ListeningServer();
            int Start();
            bool Stop();
            void Accept(int socket, std::string addr);

        private:
            std::shared_ptr<ProtocolHandler> handler;
            int acceptor_socket;
            bool tls;
            std::string name;
            std::string addr;
            int port;
            NetworkManager *manager;
        };

        class NetworkManager {
        public:
            NetworkManager();
            ~NetworkManager();

            void RegisterProtocolHandler(std::string addr, ProtocolHandler *handler);
            void AddListeningServer(std::string name, std::string addr, int port, bool enable_tls,
                                    std::string handler_name);
            void RemoveListeningServer(std::string name);
            void RegisterConnection(int socket, TcpConnection *conn, GameConnection *gconn);
            void AcceptNewConnections();
            void ProcessNewInput();

        private:
            std::unordered_map<std::string, std::shared_ptr<ProtocolHandler>> handler_map;
            std::unordered_map<int, std::shared_ptr<GameConnection>> game_conns;
            std::unordered_map<int, std::shared_ptr<ListeningServer>> acceptor_map;
            std::unordered_map<int, std::shared_ptr<TcpConnection>> connection_map;
            std::unordered_map<std::string, std::shared_ptr<ListeningServer>> server_map;
            int acceptor_epollfd;
            int connections_epollfd;
            std::vector<epoll_event> acceptor_events;
            std::vector<epoll_event> connection_events;
        };
    }
#endif //MUDPP_NET_H
