#ifndef BB_COMMAND_RECEIVER_H
#define BB_COMMAND_RECEIVER_H

#include <iostream>
#include <string>
#include "util/shared_buffer.h"
#include <boost/asio.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <unordered_map>

namespace bb {

using util::SharedBuffer;
using boost::asio::ip::udp;

enum DirectionCommand {
    FORWARD = 0,
    ROTATE_LEFT = 1,
    ROTATE_RIGHT = 2,
    STOP = 3,
    UNKNOWN = 4
};

class CommandReceiver {
public:

    static void Receive(util::SharedBuffer<DirectionCommand> &buffer,
                        int port);

private:

    static const constexpr int kMaxLength = 1024;
    static const std::unordered_map<std::string, DirectionCommand> kCommandMap;

    boost::asio::ip::udp::socket socket;
    char data[kMaxLength];
    boost::asio::ip::udp::endpoint senderEndpoint;
    util::SharedBuffer<DirectionCommand> &buffer;

    CommandReceiver(util::SharedBuffer<DirectionCommand> &buffer,
                    boost::asio::io_service &ioService,
                    int port);

    void CommandHandler(const boost::system::error_code& error,
                 size_t bytesReceived);

};

}; // namespace bb

#endif // BB_COMMAND_RECEIVER_H