#include "bb/command_receiver.h"

namespace bb {

const std::unordered_map<std::string, DirectionCommand> CommandReceiver::kCommandMap = {
    {"FORWARD\n", DirectionCommand::FORWARD},
    {"ROTATE_RIGHT\n", DirectionCommand::ROTATE_RIGHT},
    {"ROTATE_LEFT\n", DirectionCommand::ROTATE_LEFT},
    {"STOP\n", DirectionCommand::STOP},
};

void CommandReceiver::Receive(util::SharedBuffer<DirectionCommand> &buffer,
                              int port) {
    try {    
        boost::asio::io_service ioService(1);
        CommandReceiver commandReceiver(buffer, ioService, port);        
        ioService.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
};


CommandReceiver::CommandReceiver(util::SharedBuffer<DirectionCommand> &buffer,
                                 boost::asio::io_service &ioService,
                                 int port) : buffer(buffer), socket(ioService) {
        
    boost::asio::ip::udp::endpoint listenEndpoint(boost::asio::ip::udp::v4(), port);

    socket.open(listenEndpoint.protocol());
    socket.set_option(
            boost::asio::ip::udp::socket::reuse_address(true));
        
    socket.bind(listenEndpoint);

    socket.async_receive_from(boost::asio::buffer(data, kMaxLength), 
                               senderEndpoint,
                               boost::bind(&CommandReceiver::CommandHandler, 
                                           this,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
};

void CommandReceiver::CommandHandler(const boost::system::error_code& error,
                              size_t bytesReceived) {
    if (!error) {
        
        auto search = CommandReceiver::kCommandMap.find(std::string(data, 0, bytesReceived));
        if (search != CommandReceiver::kCommandMap.end()) {
            std::cout << search->first << std::endl;
            buffer.Push(search->second);
        }

        socket.async_receive_from(boost::asio::buffer(data, kMaxLength), 
                                   senderEndpoint,
                                   boost::bind(&CommandReceiver::CommandHandler, 
                                               this,
                                               boost::asio::placeholders::error,
                                               boost::asio::placeholders::bytes_transferred));
    }
};

}; // namespace bb
