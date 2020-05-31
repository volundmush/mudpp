//
// Created by volund on 5/30/20.
//

#ifndef MUDPP_TELNET_H
#define MUDPP_TELNET_H

#include "net.h"

namespace mudpp::net::telnet {

    enum telnet_codes : uint8_t
    {
        NUL = 0,
        BEL = 7,
        CR = 13,
        LF = 10,
        SGA = 3,
        NAWS = 31,
        SE = 240,
        NOP = 241,
        GA = 249,
        SB = 250,
        WILL = 251,
        WONT = 252,
        DO = 253,
        DONT = 254,
        IAC = 255,

        // The following are special MUD specific protocols.

        // MUD eXtension Protocol
        MXP = 91,

        // Mud Server Status Protocol
        MSSP = 70,

        // Compression
        // MCCP1 = 85 - this is deprecrated
        MCCP2 = 86,
        MCCP3 = 87,

        // GMCP - Generic Mud Communication Protocol
        GMCP = 201,

        // MSDP - Mud Server Data Protocol
        MSDP = 69,

        // TTYPE - Terminal Type
        TTYPE = 24
    };

    enum telnet_state : uint8_t {
        DATA = 0,
        ESCAPED = 1,
        SUBNEGOTIATION = 2,
        IN_SUBNEGOTIATION = 3,
        SUB_ESCAPED = 4,
        COMMAND = 5,
        ENDLINE = 6
    };

    class TelnetOptionHandler {

    };



    class TelnetProtocol : public Protocol {
    public:
        explicit TelnetProtocol(TcpConnection *connection);
        void Receive(bytes data) override;
    private:
        void ProcessCommand(uint8_t command, uint8_t arg);
        void ProcessSubCommand(uint8_t arg, bytes data);
        void ProcessUserCommand(bytes data);
        std::unordered_map<telnet_codes, std::shared_ptr<TelnetOptionHandler>> op_handlers;
        uint8_t subcommand = 0;
        telnet_codes command_mode = NUL;
        telnet_state state = DATA;
        bytes user_command_buffer;
        bytes sub_command_buffer;
        bool mccp2 = false;
        bool mccp3 = false;
    };

    class TelnetProtocolHandler : public ProtocolHandler {
    public:
        virtual Protocol *WrapConnection(TcpConnection *conn) override;

    };

}




#endif //MUDPP_TELNET_H
