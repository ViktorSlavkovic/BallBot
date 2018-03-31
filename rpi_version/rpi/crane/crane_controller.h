#ifndef CRANE_CRANE_CONTROLLER_H_
#define CRANE_CRANE_CONTROLLER_H_

#include <array>
#include <boost/asio.hpp>
#include <chrono>
#include <cstdio>
#include <iterator>
#include <sstream>
#include <thread>

namespace crane {

using boost::asio::ip::udp;

class CraneController {
 public:
  static CraneController& Instance() {
    static CraneController crane_controller;
    return crane_controller;
  }

  void Rise() { Rise(3000); }

  void Rise(uint32_t max_millis) {
    std::ostringstream sout;
    sout << "UP@" << max_millis << "@\n";
    auto s = sout.str();
    std::copy(s.begin(), s.end(), send_buffer_.begin());
    
    for (int i = 0; i < kNumSendRepeats; i++) {
      socket_.send_to(boost::asio::buffer(send_buffer_), receiver_endpoint_);
    }
  }

  void Drop(uint32_t millis) {
    std::ostringstream sout;
    sout << "DOWN@" << millis << "@\n";
    auto s = sout.str();
    std::copy(s.begin(), s.end(), send_buffer_.begin());

    for (int i = 0; i < kNumSendRepeats; i++) {
      socket_.send_to(boost::asio::buffer(send_buffer_), receiver_endpoint_);
    }
  }

  void ControlTreadmill(bool run) {
    std::ostringstream sout;
    sout << "TREADMILL@" << (run ? 1 : 0) << "@\n";
    auto s = sout.str();
    std::copy(s.begin(), s.end(), send_buffer_.begin());

    for (int i = 0; i < kNumSendRepeats; i++) {
      socket_.send_to(boost::asio::buffer(send_buffer_), receiver_endpoint_);
    }
  }

 private:
  static const constexpr int kNumSendRepeats = 3;
  
  boost::asio::io_service io_service_;
  udp::resolver resolver_;
  udp::resolver::query query_;
  udp::endpoint receiver_endpoint_;
  udp::socket socket_;
  std::array<char, 32> send_buffer_;

  CraneController()
    : io_service_(),
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