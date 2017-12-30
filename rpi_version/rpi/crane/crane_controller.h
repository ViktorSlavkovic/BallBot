#ifndef CRANE_CRANE_CONTROLLER_H_
#define CRANE_CRANE_CONTROLLER_H_

#include <array>
#include <cstdio>
#include <iterator>
#include <boost/asio.hpp>

namespace crane {

using boost::asio::ip::udp;

class CraneController {
 public:
  static CraneController& Instance() {
    static CraneController crane_controller;
    return crane_controller;
  }

  void Rise() {
    std::copy(kCommandUp2, kCommandUp2 + 4, send_buffer_.begin());
    socket_.send_to(boost::asio::buffer(send_buffer_), receiver_endpoint_);
  }

  void Drop(int num_steps) {
    std::copy(kCommandDown, kCommandDown + 5, send_buffer_.begin());
    for  (int i = 0; i < num_steps; i++) {
      socket_.send_to(boost::asio::buffer(send_buffer_), receiver_endpoint_);
    }
  }

 private:
  const char* kCommandUp;
  const char* kCommandUp2;
  const char* kCommandDown;
  boost::asio::io_service io_service_;
  udp::resolver resolver_;
  udp::resolver::query query_;
  udp::endpoint receiver_endpoint_;
  udp::socket socket_;
  std::array<char, 16> send_buffer_;

  CraneController()
    : kCommandUp("UP\n"),
      kCommandUp2("UP2\n"),
      kCommandDown("DOWN\n"),
      io_service_(),
      resolver_(io_service_),
      query_(udp::v4(), "192.168.2.253", "12345"),
      receiver_endpoint_(*resolver_.resolve(query_)),
      socket_(io_service_),
      send_buffer_() {
    socket_.open(udp::v4());
  }

  ~CraneController() {
    socket_.close();
  }

  void operator=(const CraneController&) = delete;
  void operator=(const CraneController&&) = delete;

};

};  // crane

#endif  // CRANE_CRANE_CONTROL_H_