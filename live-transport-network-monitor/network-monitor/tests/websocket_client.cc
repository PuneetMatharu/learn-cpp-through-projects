// Headers we've defined
#include "network-monitor/websocket_client.h"

// Boost-specific libraries
#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

// Regular libraries
#include <iostream>
#include <string>
#include <filesystem>

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_CASE(cacert_pem)
{
  // Make sure we were able to find the secure certificates
  BOOST_CHECK(std::filesystem::exists(TESTS_CACERT_PEM));
}

BOOST_AUTO_TEST_CASE(class_WebSocketClient)
{
  // Connection targets
  const std::string url {"echo.websocket.org"};
  const std::string port{"80"};
  const std::string message{"Hello WebSocket"};

  // Always start with an I/O context object.
  boost::asio::io_context ioc{};

  // The class under test
  NetworkMonitor::WebSocketClient client{url,port,ioc};

  // We use these flags to check that the connection, send, receive functions
  // work as expected.
  bool connected{false};
  bool message_sent{false};
  bool message_received{false};
  bool disconnected{false};
  std::string echoed_message{};

  // Our own callbacks
  auto on_send{[&message_sent](auto err_code)
  {
    message_sent=!err_code;
  }};
  auto on_connect{[&client,&connected,&on_send,&message](auto err_code)
  {
    connected=!err_code;
    if (connected)
    {
      client.send(message,on_send);
    }
  }};
  auto on_close{[&disconnected](auto err_code)
  {
    disconnected=!err_code;
  }};
  auto on_receive{[&client,
                   &on_close,
                   &message_received,
                   &echoed_message](auto err_code,auto received)
  {
    message_received=!err_code;
    echoed_message=std::move(received);
    client.close(on_close);
  }};

  // We must call io_context::run for asynchronous callbacks to run.
  client.connect(on_connect,on_receive);
  ioc.run();

  // When we get here, the io_context::run function has run out of work to do.
  BOOST_CHECK(connected);
  BOOST_CHECK(message_sent);
  BOOST_CHECK(message_received);
  BOOST_CHECK(disconnected);
  BOOST_CHECK_EQUAL(message,echoed_message);
} // BOOST_AUTO_TEST_CASE(class_WebSocketClient)

BOOST_AUTO_TEST_SUITE_END();

