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
namespace websocket=boost::beast::websocket;

//===========================================================================
// Logs the status of a Boost I/O context usage
//===========================================================================
void log(const std::string& where,boost::system::error_code ec)
{
  std::cerr << "[" << std::left << std::setw(20) << where << "] "
            << (ec ? "Error: " : "OK")
            << (ec ? ec.message() : "")
            << std::endl;
} // End of Log


//===========================================================================
// Callback handler for an asynchronous write. Stop if an error has been
// encountered. Otherwise, output the endpoint response.
//===========================================================================
void on_receive(
  // --> Start of shared data
  boost::beast::flat_buffer& read_buffer,
  // <-- End of shared data
  const boost::system::error_code& err_code)
{
  if (err_code)
  {
    log("on_receive",err_code);
    return;
  }

  // Log the message that was received
  std::cerr << "Message received: "
            << boost::beast::make_printable(read_buffer.data())
            << std::endl;
} // End of on_receive


//===========================================================================
// Callback handler for an asynchronous write. Stop if an error has been
// encountered. Otherwise, attempt to read the endpoint response.
//===========================================================================
void on_send(
  // --> Start of shared data
  websocket::stream<boost::beast::tcp_stream>& web_socket,
  boost::beast::flat_buffer& read_buffer,
  // <-- End of shared data
  const boost::system::error_code& err_code)
{
  if (err_code)
  {
    log("on_send",err_code);
    return;
  }

  // Read the echoed message back.
  web_socket.async_read(
    read_buffer,
    [&read_buffer](auto err_code, auto n_bytes_read)
  {
    on_receive(read_buffer,err_code);
  });
} // End of on_send

//===========================================================================
// Callback handler for an asynchronous handshake call. Stop if an error has
// been encountered. Otherwise, attempt to send a message to the endpoint.
//===========================================================================
void on_handshake(
  // --> Start of shared data
  websocket::stream<boost::beast::tcp_stream>& web_socket,
  const boost::asio::const_buffer& write_buffer,
  boost::beast::flat_buffer& read_buffer,
  // <-- End of shared data
  const boost::system::error_code& err_code)
{
  if (err_code)
  {
    log("on_handshake",err_code);
    return;
  }

  // Tell the WebSocket object to exchange messages in text format.
  web_socket.text(true);

  // Send a message to the connected WebSocket server.
  web_socket.async_write(
    write_buffer,
    [&web_socket,&read_buffer](auto err_code, auto n_byte_written)
  {
    on_send(web_socket,read_buffer,err_code);
  });
} // End of on_handshake


//===========================================================================
// Callback handler for an asynchronous connect call. Stops if an error was
// encountered. If an error was not encountered, it tests the connection with
// an asynchronous handshake call.
//===========================================================================
void on_connect(
  // --> Start of shared data
  boost::beast::websocket::stream<tcp_stream>& web_socket,
  const std::string& url,
  boost::asio::const_buffer& write_buffer,
  boost::beast::flat_buffer& read_buffer,
  // <-- End of shared data
  const boost::system::error_code& err_code)
{
  if (err_code)
  {
    log("on_connect",err_code);
    return;
  }

  web_socket.async_handshake(
    url,"/",
    [&web_socket,&write_buffer,&read_buffer](auto err_code)
  {
    on_handshake(web_socket,write_buffer,read_buffer,err_code);
  });
} // End of on_connect



//===========================================================================
// Callback that handles the outcome of a resolve call. If an error occurs,
// we stop, otherwise we move onto the next stage; trying to asynchronously
// connect to the endpoint.
//===========================================================================
void on_resolve(
  // --> Start of shared data
  boost::beast::websocket::stream<tcp_stream>& web_socket,
  const std::string& url,
  boost::asio::const_buffer& write_buffer,
  boost::beast::flat_buffer& read_buffer,
  // <-- End of shared data
  const boost::system::error_code& err_code,
  tcp::resolver::iterator endpoint)
{
  // If an error occurred, log it then exit
  if (err_code)
  {
    log("on_resolve",err_code);
    return;
  }

  // Connect to the TCP socket. Instead of constructing the socket and the
  // web_socket objects separately, we pass the io_context object to the
  // websocket and access the TCP socket through next_layer()
  web_socket.next_layer().async_connect(
    *endpoint,
    [&web_socket,&url,&write_buffer,&read_buffer](auto err_code)
  {
    on_connect(web_socket,url,write_buffer,read_buffer,err_code);
  });
} // End of on_resolve

//===========================================================================
// Set up the necessary data required to resolve the specified URL, create a
// TCP socket and connect to the endpoint, perform a WebSocket handshake,
// send a message, wait for a reply, then output the returned message.
//===========================================================================
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

  // The objects that will be shared by the connection callbacks
  websocket::stream<tcp_stream> web_socket{ioc};
  boost::asio::const_buffer write_buffer(message.c_str(),message.size());
  boost::beast::flat_buffer read_buffer{};

  // Attempt to resolve the address
  tcp::resolver resolver{ioc};
  resolver.async_resolve(
    url,port,
    [&web_socket,&url,&write_buffer,&read_buffer](auto err_code, auto endpoint)
  {
    on_resolve(web_socket,url,write_buffer,read_buffer,err_code,endpoint);
  });

  // Run the chain of callbacks
  ioc.run();
} // End of main







