// Boost-specific libraries
#include <boost/asio.hpp>

// Regular libraries
#include <iostream>
#include <string>

// Headers we've defined
#include "network-monitor/websocket_client.h"

int main()
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
  bool message_matches{false};
  bool disconnected{false};

  // Our own callbacks
  auto on_send{[&message_sent](auto ec)
  {
    message_sent=!ec;
  }};
  auto on_connect{[&client, &connected, &on_send, &message](auto ec)
  {
    connected=!ec;
    if (!ec)
    {
      client.send(message, on_send);
    }
  }};
  auto on_close{[&disconnected](auto ec)
  {
    disconnected = !ec;
  }};
  auto on_receive{[&client,
                   &on_close,
                   &message_received,
                   &message_matches,
                   &message](auto ec, auto received)
  {
    message_received=!ec;
    message_matches=(message==received);
    client.close(on_close);
  }};

  // We must call io_context::run for asynchronous callbacks to run.
  client.connect(on_connect,on_receive);
  ioc.run();

  // When we get here, the io_context::run function has run out of work to do.
  bool ok
  {
    connected &&
    message_sent &&
    message_received &&
    message_matches &&
    disconnected
  };
  if (ok)
  {
    std::cout << "OK" << std::endl;
    return 0;
  }
  else
  {
    std::cerr << "Test failed" << std::endl;
    return 1;
  }
} // End of main


