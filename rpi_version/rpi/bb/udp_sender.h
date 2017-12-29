#ifndef BB_UDP_SENDER_H
#define BB_UDP_SENDER_H

#include "util/shared_buffer.h"
#include <boost/asio.hpp>
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace bb {

using util::SharedBuffer;

class UDPSender {
public:
    static void Send(SharedBuffer<std::string> &buffer, 
                     const std::string &receiverIp) {
        try {
            boost::asio::io_service ioService;
            UDPSender sender(buffer, 
                             ioService, 
                             boost::asio::ip::address::from_string(receiverIp));
            ioService.run();
        } catch (std::exception &e) {
            // Report error.
        }
    }
private:
    SharedBuffer<std::string> &buffer;

    boost::asio::ip::udp::endpoint endpoint;
    boost::asio::ip::udp::socket socket;
    boost::asio::deadline_timer timer;
    int messageCount;
    std::string message;

    const int port = 30001;

    UDPSender(SharedBuffer<std::string> &buffer,
              boost::asio::io_service& ioService,
              const boost::asio::ip::address& ipAddress) 
            : buffer(buffer),
              endpoint(ipAddress, port),
              socket(ioService, endpoint.protocol()),
              timer(ioService),
              messageCount(0) {

        std::ostringstream os;
        os << "UDP Sender attached. Sending data to: " << ipAddress << "/" << port << std::endl; 
        message = os.str();

        std::cout << message;
        
        socket.async_send_to(
            boost::asio::buffer(message), endpoint,
            boost::bind(&UDPSender::handleSender, 
                        this,
                        boost::asio::placeholders::error));
        
    }

    void handleSender(const boost::system::error_code& error) {
        if (!error) {

            message = buffer.Pop();

            socket.async_send_to(
            boost::asio::buffer(message), endpoint,
            boost::bind(&UDPSender::handleSender, 
                        this,
                        boost::asio::placeholders::error));
        }
    }
  

};

}; // namespace bb

#endif // BB_UDP_SENDER_H