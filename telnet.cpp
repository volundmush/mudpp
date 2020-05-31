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

    for (auto &byte : data)
    {
        switch(this->state)
        {
            case DATA:
                switch(byte)
                {
                    case telnet_codes::IAC:
                        this->state = ESCAPED;
                        break;
                    case telnet_codes::CR:
                        this->state = ENDLINE;
                        break;
                    default:
                        this->user_command_buffer.push_back(byte);
                        break;
                }
                break;
            case ESCAPED:
                switch(byte)
                {
                    case telnet_codes::WILL:
                        this->state = COMMAND;
                        this->command_mode = telnet_codes::WILL;
                        break;
                    case telnet_codes::WONT:
                        this->state = COMMAND;
                        this->command_mode = telnet_codes::WONT;
                        break;
                    case telnet_codes::DO:
                        this->state = COMMAND;
                        this->command_mode = telnet_codes::DO;
                        break;
                    case telnet_codes::DONT:
                        this->state = COMMAND;
                        this->command_mode = telnet_codes::DONT;
                        break;
                    case telnet_codes::SB:
                        this->state = SUBNEGOTIATION;
                        break;
                    case telnet_codes::IAC:
                        this->state = DATA;
                        this->user_command_buffer.push_back(byte);
                        break;
                    default:
                        break;
                }
                break;
            case COMMAND:
                this->ProcessCommand(this->command_mode, byte);
                this->state = DATA;
                break;
            case SUBNEGOTIATION:
                this->subcommand = byte;
                this->state = IN_SUBNEGOTIATION;
                break;
            case IN_SUBNEGOTIATION:
                switch(byte)
                {
                    case telnet_codes::IAC:
                        this->state = SUB_ESCAPED;
                        break;
                    default:
                        this->sub_command_buffer.push_back(byte);
                        break;
                }
                break;
            case SUB_ESCAPED:
                switch(byte)
                {
                    case telnet_codes::IAC:
                        this->state = IN_SUBNEGOTIATION;
                        this->sub_command_buffer.push_back(byte);
                        break;
                    case telnet_codes::SE:
                        this->state = DATA;
                        this->ProcessSubCommand(this->subcommand, this->sub_command_buffer);
                        this->sub_command_buffer.clear();
                        this->subcommand = 0;
                    default:
                        break;
                }
                break;

            case ENDLINE:
                switch(byte)
                {
                    case telnet_codes::LF:
                        this->state = DATA;
                        this->ProcessUserCommand(this->user_command_buffer);
                        this->user_command_buffer.clear();
                        break;
                    case telnet_codes::IAC:
                        this->state = DATA;
                        this->ProcessUserCommand(this->user_command_buffer);
                        this->user_command_buffer.clear();
                        break;
                    default:
                        this->state = DATA;
                        this->user_command_buffer.push_back(telnet_codes::CR);
                        break;
                }
                break;
        }
    }

}

void mudpp::net::telnet::TelnetProtocol::ProcessCommand(uint8_t command, uint8_t arg) {
    Send(bytes{'I', 'A', 'C', ' ', command, ' ', arg, CR, LF});
}

void mudpp::net::telnet::TelnetProtocol::ProcessSubCommand(uint8_t arg, mudpp::net::bytes data) {
    Send(bytes{'I', 'A', 'C', ' ', 'S', 'B', ' ', arg});
    Send(data);
    Send(bytes{'I', 'A', 'C', ' ', 'S', 'E', CR, LF});
}

void mudpp::net::telnet::TelnetProtocol::ProcessUserCommand(mudpp::net::bytes data) {
    Send(bytes{'R', 'E', 'C', 'V', ' '});
    Send(data);
    Send(bytes{CR, LF});
}


