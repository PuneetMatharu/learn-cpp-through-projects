// Boost-specific libraries
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

// OpenSSL libraries to use SSL/TLS protocols
#include <openssl/ssl.h>

// Regular libraries
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

// Headers we've defined
#include "network-monitor/websocket_client.h"

namespace NetworkMonitor
{
  // Static functions

  static void log(const std::string& where,
                  const boost::system::error_code& ec)
  {
    std::cerr << "[" << std::setw(20) << where << "] "
              << (ec ? "Error: " : "OK")
              << (ec ? ec.message() : "")
              << std::endl;
  } // End of log

  // Public methods

  //=========================================================================
  /*! \brief Construct a WebSocket client.

      \note This constructor does not initiate a connection.

      \param url  The URL of the server.
      \param port The port on the server.
      \param ioc  The io_context object. The user takes care of calling
                  ioc.run().
  */
  //=========================================================================
  WebSocketClient::WebSocketClient(const std::string& url,
                                   const std::string& port,
                                   boost::asio::io_context& ioc) :
    Url(url),
    Port(port),
    Resolver(boost::asio::make_strand(ioc)),
    Web_socket(boost::asio::make_strand(ioc))
  {} // End of WebSocketClient

  //=========================================================================
  /*! \brief Destructor
  */
  //=========================================================================
  WebSocketClient::~WebSocketClient() = default;

  //=========================================================================
  /*! \brief Connect to the server.

      \param on_connect     Called when the connection fails or succeeds.
      \param on_message     Called only when a message is successfully
                            received. The message is an rvalue reference;
                            ownership is passed to the receiver.
      \param on_disconnect  Called when the connection is closed by the server
                            or due to a connection error.
  */
  //=========================================================================
  void WebSocketClient::connect(
    std::function<void (boost::system::error_code)> on_connect,
    std::function<void (boost::system::error_code,
                        std::string&&)> on_message,
    std::function<void (boost::system::error_code)> on_disconnect)
  {
    // Save the callbacks for later use
    OnConnectPt=on_connect;
    OnMessagePt=on_message;
    OnDisconnectPt=on_disconnect;

    // Start the chain of asynchronous callbacks
    Resolver.async_resolve(
      Url,Port,
      [this](auto err_code, auto endpoint)
    {
      on_resolve(err_code,endpoint);
    });
  } // End of connect

  //=========================================================================
  /*! \brief Send a text message to the WebSocket server.

      \param message  The message to send.
      \param on_send  Called when a message is sent successfully or if it
                      failed to send.
  */
  //=========================================================================
  void WebSocketClient::send(
    const std::string& message,
    std::function<void (boost::system::error_code)> on_send)
  {
    Web_socket.async_write(
      boost::asio::buffer(std::move(message)),
      [this,on_send](auto err_code, auto n_byte_written)
    {
      if (on_send)
      {
        on_send(err_code);
      }
    });
  } // End of send

  //=========================================================================
  /*! \brief Close the WebSocket connection.

      \param on_close Called when the connection is closed, successfully or
                      not.
  */
  //=========================================================================
  void WebSocketClient::close(
    std::function<void (boost::system::error_code)> on_close)
  {
    Web_socket.async_close(
      boost::beast::websocket::close_code::none,
      [this,on_close](auto err_code)
    {
      if (on_close)
      {
        on_close(err_code);
      }
    });
  } // End of close

  // Private methods

  //=========================================================================
  //
  //=========================================================================
  void WebSocketClient::on_resolve(
    const boost::system::error_code& err_code,
    boost::asio::ip::tcp::resolver::iterator endpoint)
  {
    if (err_code)
    {
      log("on_resolve",err_code);
      if (OnConnectPt)
      {
        OnConnectPt(err_code);
      }
      return;
    } // if (err_code)

    // The following timeout only matters for the purpose of connecting to the
    // TCP socket. We will reset the timeout to a sensible default after we are
    // connected.
    Web_socket.next_layer().expires_after(std::chrono::seconds(5));

    // Connect to the TCP socket.
    // Instead of constructing the socket and the ws objects separately, the
    // socket is now embedded in ws_, and we access it through next_layer().
    Web_socket.next_layer().async_connect(
      *endpoint,
      [this](auto err_code)
    {
      on_connect(err_code);
    });
  } // End of on_resolve


  //=========================================================================
  //
  //=========================================================================
  void WebSocketClient::on_connect(const boost::system::error_code& err_code)
  {
    if (err_code)
    {
      log("on_connect",err_code);
      if (OnConnectPt)
      {
        OnConnectPt(err_code);
      }
      return;
    }

    // Now that the TCP socket is connected, we can reset the timeout to
    // whatever Boost.Beast recommends.
    Web_socket.next_layer().expires_never();
    Web_socket.set_option(
      boost::beast::websocket::stream_base::timeout::suggested(
        boost::beast::role_type::client));

    // Attempt a WebSocket handshake.
    Web_socket.async_handshake(
      Url,"/",
      [this](auto err_code)
    {
      on_handshake(err_code);
    });
  } // End of on_connect


  //=========================================================================
  //
  //=========================================================================
  void WebSocketClient::on_handshake(const boost::system::error_code& err_code)
  {
    if (err_code)
    {
      log("on_handshake",err_code);
      if (OnConnectPt)
      {
        OnConnectPt(err_code);
      }
      return;
    }

    // Tell the WebSocket object to exchange messages in text format.
    Web_socket.text(true);

    // Now that we are connected, set up a recursive asynchronous listener to
    // receive messages.
    listen_to_incoming_message(err_code);

    // Dispatch the user callback.
    // Note: This call is synchronous and will block the WebSocket strand.
    if (OnConnectPt)
    {
      OnConnectPt(err_code);
    }
  } // End of on_handshake


  //=========================================================================
  //
  //=========================================================================
  void WebSocketClient::listen_to_incoming_message(
    const boost::system::error_code& err_code)
  {
    // Stop processing messages if the connection has been aborted.
    if (err_code==boost::asio::error::operation_aborted)
    {
      if (OnDisconnectPt)
      {
        OnDisconnectPt(err_code);
      }
      return;
    }

    // Read a message asynchronously. On a successful read, process the message
    // and recursively call this function again to process the next message.
    Web_socket.async_read(Read_buffer,
                          [this](auto err_code, auto n_bytes)
    {
      on_read(err_code,n_bytes);
      listen_to_incoming_message(err_code);
    });
  } // End of listen_to_incoming_message


  //=========================================================================
  //
  //=========================================================================
  void WebSocketClient::on_read(const boost::system::error_code& err_code,
                                size_t n_bytes)
  {
    // We just ignore messages that failed to read.
    if (err_code)
    {
      return;
    }

    // Parse the message and forward it to the user callback.
    // Note: This call is synchronous and will block the WebSocket strand.
    std::string message{boost::beast::buffers_to_string(Read_buffer.data())};
    Read_buffer.consume(n_bytes);
    if (OnMessagePt)
    {
      OnMessagePt(err_code,std::move(message));
    }
  } // End of on_read






} // namespace NetworkMonitor