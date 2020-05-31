//
// Created by volund on 5/17/20.
//

#include "net.h"


void mudpp::net::ProtocolHandler::SetNetworkManager(NetworkManager *manager) {
    this->manager = std::shared_ptr<NetworkManager>(manager);
}

// NETWORK MANAGER SECTION
mudpp::net::NetworkManager::NetworkManager() {
    acceptor_epollfd = epoll_create(20);
    connections_epollfd = epoll_create(20);
}

mudpp::net::NetworkManager::~NetworkManager() {
    close(this->acceptor_epollfd);
    close(this->connections_epollfd);

}

void mudpp::net::NetworkManager::RegisterProtocolHandler(std::string addr, ProtocolHandler *handler) {
    this->handler_map[addr] = std::shared_ptr<ProtocolHandler>(handler);
    handler->SetNetworkManager(this);
}

void mudpp::net::NetworkManager::AddListeningServer(std::string name, std::string addr, int port, bool enable_tls, std::string handler_name) {

    auto *server = new ListeningServer(name, addr, port, enable_tls, this->handler_map[handler_name], this);
    int acceptor = 0;
    if((acceptor = server->Start())<1) {
        // Something done goofed here.
    }

    // Add a slot to Epoll and alert epoll_ctl about the new acceptor.
    epoll_event ev_in{};
    ev_in.events = EPOLLIN;
    ev_in.data.fd = acceptor;
    int epcheck = epoll_ctl(this->acceptor_epollfd, EPOLL_CTL_ADD, acceptor, &ev_in);
    int the_error = errno;
    this->acceptor_events.resize(this->acceptor_events.size() + 1);

    // Add server to the maps.
    this->acceptor_map[acceptor] = std::shared_ptr<ListeningServer>(server);
    this->server_map[name] = std::shared_ptr<ListeningServer>(server);

}

void mudpp::net::NetworkManager::RemoveListeningServer(std::string name) {
    this->acceptor_events.resize(this->acceptor_events.size() - 1);
}

void mudpp::net::NetworkManager::AcceptNewConnections() {
    int size = this->acceptor_events.size();
    int ready = epoll_wait(this->acceptor_epollfd, this->acceptor_events.data(), this->acceptor_events.size(), 0);
    if(ready < 1)
    {
        // Nothing to do - return early.
        return;
    }

    sockaddr_storage new_addr{};
    int addrlen = sizeof(new_addr);
    uint16_t port;
    char addrstr[INET6_ADDRSTRLEN];

    // Loop over all sockets epoll says have connections waiting.
    for(int i = 0; i < ready; i++)
    {
        auto ev = this->acceptor_events[i];
        int fd = ev.data.fd;
        auto server = this->acceptor_map[fd];

        int new_connection = -1;

        // Continue attempting to accept() until it EWOULDBLOCK. This will empty out all queued connections.
        // I hope.
        while(true)
        {
            if((new_connection = accept(fd, (sockaddr *)&new_addr, (socklen_t*)&addrlen)) == -1)
            {
                // Eventually we will run out of pending connections. At which point? break the loop.
                // The socket MUST be set NON BLOCKING or this will block the thread...
                break;
            }

            if(new_addr.ss_family == AF_INET)
            {
                auto *s = (sockaddr_in*)&new_addr;
                port = ntohs(s->sin_port);
                inet_ntop(AF_INET, &s->sin_addr, addrstr, sizeof(addrstr));
            } else
            {
                auto *s = (sockaddr_in6*)&new_addr;
                port = ntohs(s->sin6_port);
                inet_ntop(AF_INET6, &s->sin6_addr, addrstr, sizeof(addrstr));
            }
            std::string addr(addrstr);
            server->Accept(new_connection, addr);
        }
    }

}

void mudpp::net::NetworkManager::RegisterConnection(int socket, mudpp::net::TcpConnection *conn, GameConnection *gconn) {
    this->connection_events.resize(this->connection_events.size() + 1);
    epoll_event ev_in{};
    ev_in.events = EPOLLIN;
    ev_in.data.fd = socket;
    epoll_ctl(this->connections_epollfd, EPOLL_CTL_ADD, socket, &ev_in);
    this->game_conns[socket] = std::shared_ptr<GameConnection>(gconn);
    this->connection_map[socket] = std::shared_ptr<TcpConnection>(conn);
}

void mudpp::net::NetworkManager::ProcessNewInput() {
    int ready = 0;
    if(!(ready = epoll_wait(this->connections_epollfd, this->connection_events.data(), this->connection_events.size(), 0)>0))
    {
        // Nothing to do - return early.
        return;
    }

    for(int i = 0; i < ready; i++)
    {
        auto ev = this->connection_events[i];
        int fd = ev.data.fd;
        auto conn = this->connection_map[fd];
        conn->ReadFromSocket();
    }

}


// LISTENING SERVER SECTION
mudpp::net::ListeningServer::ListeningServer(std::string name, std::string addr, int port, bool tls,
                                             std::shared_ptr<ProtocolHandler> handler, NetworkManager *manager) {

    this->port = port;
    this->name = name;
    this->addr = addr;
    this->tls = tls;
    this->handler = handler;
    this->manager = manager;

}

mudpp::net::ListeningServer::~ListeningServer() {

}

int mudpp::net::ListeningServer::Start() {
    int new_socket = 0;
    // socket() will return -1 if there's an error.
    if((new_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) != -1)
    {
        int option = 1;

        if(setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT | SO_KEEPALIVE, &option,
                       sizeof(option)) == 0)
        {
            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = inet_addr(this->addr.c_str());
            address.sin_port = htons(this->port);

            if(bind(new_socket, (sockaddr *)&address, sizeof(address)) == 0)
            {
                if(listen(new_socket, 40) == 0)
                {
                    this->acceptor_socket = new_socket;
                    return new_socket;
                }
            }
        }

    }
    return -1;
}

bool mudpp::net::ListeningServer::Stop() {
    return false;
}

// Accepts a connection. sets up the socket
void mudpp::net::ListeningServer::Accept(int socket, std::string addr) {
    TcpConnection *conn;
    if(this->tls)
    {
        conn = new TlsConnection(socket, addr);
    } else {
        conn = new TcpConnection(socket, addr);
    }

    Protocol *prot = this->handler->WrapConnection(conn);
    conn->SetProtocol(prot);
    auto *gconn = new GameConnection(prot);
    prot->SetGameConnection(gconn);
    this->manager->RegisterConnection(socket, conn, gconn);
    prot->OnConnect();
}

mudpp::net::TcpConnection::TcpConnection(int socket, std::string addr) {
    this->socket = socket;
    this->addr = addr;
}

mudpp::net::TcpConnection::~TcpConnection() {

}


void mudpp::net::TcpConnection::Send(bytes data) {
    write(this->socket, data.data(), data.size());
}

void mudpp::net::TcpConnection::Receive(bytes data) {
    this->protocol->Receive(data);
}

void mudpp::net::TcpConnection::ReadFromSocket() {
    // First, let's get how many bytes should be processed.
    int count;
    ioctl(this->socket, FIONREAD, &count);
    bytes in_buffer;
    in_buffer.resize(count);
    read(this->socket, in_buffer.data(), count);
    this->Receive(in_buffer);
}

void mudpp::net::TcpConnection::SendToSocket() {

}

void mudpp::net::TcpConnection::SetProtocol(mudpp::net::Protocol *prot) {
    this->protocol = std::shared_ptr<Protocol>(prot);
}

mudpp::net::TlsConnection::TlsConnection(int socket, std::string addr) : TcpConnection(socket, addr) {

}

mudpp::net::TlsConnection::~TlsConnection() {

}

void mudpp::net::TlsConnection::Send(bytes data) {
    TcpConnection::Send(data);
}

void mudpp::net::TlsConnection::Receive(bytes data) {
    TcpConnection::Receive(data);
}

mudpp::net::Protocol::Protocol(mudpp::net::TcpConnection *connection) {
    this->conn = std::shared_ptr<TcpConnection>(connection);
}

mudpp::net::Protocol::~Protocol() {

}

void mudpp::net::Protocol::Receive(mudpp::net::bytes data) {
    // Does nothing by default.
}

void mudpp::net::Protocol::OnConnect() {
    // Does nothing by default.
}

void mudpp::net::Protocol::SetGameConnection(mudpp::net::GameConnection *gconn) {
    this->gconn = std::shared_ptr<GameConnection>(gconn);
}

void mudpp::net::Protocol::Send(mudpp::net::bytes data) {
    this->conn->Send(data);
}

mudpp::net::GameConnection::GameConnection(mudpp::net::Protocol *prot) {
    this->prot = std::shared_ptr<Protocol>(prot);
}

mudpp::net::GameConnection::~GameConnection() {

}
