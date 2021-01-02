// Boost-specific libraries
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

// Regular libraries
#include <iomanip>
#include <iostream>
#include <thread>

// Type alias for the TCP socket
using tcp=boost::asio::ip::tcp;
using tcp_stream=boost::beast::tcp_stream;

// Logs the status of a Boost I/O context usage
void log(const std::string& where,boost::system::error_code ec)
{
  std::cerr << "[" << std::left << std::setw(20) << where << "] "
            << (ec ? "Error: " : "OK")
            << (ec ? ec.message() : "")
            << std::endl;
} // End of Log

int main()
{
  // Connection targets
  const std::string url {"echo.websocket.org"};
  const std::string port{"80"};
  const std::string message{"Hello-ooo! Can anybody hear me?..."};

  // Always start with an I/O context object.
  boost::asio::io_context ioc{};

  // An error code to store the status of the program
  boost::system::error_code err_code{};

  // Attempt to resolve the address
  tcp::resolver resolver{ioc};
  auto endpoint{resolver.resolve(url,port,err_code)};
  if (err_code)
  {
    log("resolver.resolve",err_code);
    return -1;
  }

  // Create a TCP connection to the resolved host.
  tcp::socket socket{ioc};
  socket.connect(*endpoint,err_code);
  if (err_code)
  {
    log("socket.connect",err_code);
    return -2;
  }

  // Tie the socket object to the WebSocket stream and attempt an handshake.
  boost::beast::websocket::stream<tcp_stream> web_socket(std::move(socket));
  web_socket.handshake(url,"/",err_code);
  if (err_code)
  {
    log("web_socket.handshake",err_code);
    return -3;
  }

  // Tell the WebSocket object to exchange messages in text format.
  web_socket.text(true);

  // Send a message to the connected WebSocket server.
  boost::asio::const_buffer write_buffer(message.c_str(),message.size());
  web_socket.write(write_buffer,err_code);
  if (err_code)
  {
    log("web_socket.write",err_code);
    return -4;
  }

  // Read the echoed message back
  boost::beast::flat_buffer read_buffer{};
  web_socket.read(read_buffer,err_code);
  if (err_code)
  {
    log("web_socket.read",err_code);
    return -5;
  }

  // Log the message that was received
  std::cerr << "Message sent: "
            << boost::beast::make_printable(write_buffer)
            << "\nMessage received: "
            << boost::beast::make_printable(read_buffer.data())
            << std::endl;

  // We're done
  log("Returning...",err_code);
} // End of main







