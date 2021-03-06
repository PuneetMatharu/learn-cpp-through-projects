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

//-------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(cacert_pem)
{
  // Make sure we were able to find the secure certificates
  BOOST_CHECK(std::filesystem::exists(TESTS_CACERT_PEM));
}

//-------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(class_WebSocketClient)
{
  // Connection targets
  const std::string url {"echo.websocket.org"};
  const std::string endpoint {"/"};
  const std::string port{"443"};
  const std::string message{"Hello WebSocket"};

  // Always start with an I/O context object.
  boost::asio::io_context ioc{};

  // TLS context for a secure WebSocket connection
  boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};

  // Load the Certificate Authority (CA) entities
  ctx.load_verify_file(TESTS_CACERT_PEM);

  // The class under test
  NetworkMonitor::WebSocketClient client{url,endpoint,port,ioc,ctx};

  // We use these flags to check that the connection, send, receive functions
  // work as expected.
  bool connected{false};
  bool message_sent{false};
  bool message_received{false};
  bool disconnected{false};
  std::string response{};

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
                   &response](auto err_code,auto received)
  {
    message_received=!err_code;
    response=std::move(received);
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
  BOOST_CHECK_EQUAL(message,response);
} // BOOST_AUTO_TEST_CASE(class_WebSocketClient)

//-------------------------------------------------------------------------

bool check_response(const std::string& response)
{
  // We do not parse the whole message. We only check that it contains some
  // expected items.
  bool ok {true};
  ok&=response.find("ERROR")!=std::string::npos;
  ok&=response.find("ValidationInvalidAuth")!=std::string::npos;
  return ok;
} // End of check_response

BOOST_AUTO_TEST_CASE(test_stomp_protocol)
{
  // Connection targets
  const std::string url {"ltnm.learncppthroughprojects.com"};
  const std::string endpoint {"/network-events"};
  const std::string port{"443"};

  // STOMP frame
  const std::string username {"fake_username"};
  const std::string password {"fake_password"};
  std::stringstream ss{};
  ss << "STOMP" << std::endl
     << "accept-version:1.2" << std::endl
     << "host:transportforlondon.com" << std::endl
     << "login:" << username << std::endl
     << "passcode:" << password << std::endl
     << std::endl // Headers need to be followed by a blank line.
     << '\0'; // The body (even if absent) must be followed by a NULL octet.
  const std::string message {ss.str()};

  // Always start with an I/O context object.
  boost::asio::io_context ioc{};

  // TLS context for a secure WebSocket connection
  boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};

  // Load the Certificate Authority (CA) entities
  ctx.load_verify_file(TESTS_CACERT_PEM);

  // The class under test
  NetworkMonitor::WebSocketClient client{url,endpoint,port,ioc,ctx};

  // We use these flags to check that the connection, send, receive functions
  // work as expected.
  bool connected{false};
  bool message_sent{false};
  bool message_received{false};
  bool disconnected{false};
  std::string response{};

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
                   &response](auto err_code,auto received)
  {
    message_received=!err_code;
    response=std::move(received);
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
  BOOST_CHECK(check_response(response));
} // BOOST_AUTO_TEST_CASE(class_WebSocketClient)

//-------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END();

