#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

// Boost-specific libraries
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>
#include <boost/beast/ssl.hpp>

// Regular libraries
#include <functional>
#include <string>

namespace NetworkMonitor
{
  //===========================================================================
  /*! \brief Client to connect to a WebSocket server over TLS
  */
  //===========================================================================
  class WebSocketClient
  {
  public:
    //=========================================================================
    /*! \brief Construct a WebSocket client.

        \note This constructor does not initiate a connection.

        \param url      The URL of the server.
        \param endpoint The endpoint on the server to connect to.
                        Example: echo.websocket.org/<endpoint>
        \param port     The port on the server.
        \param ioc      The io_context object. The user takes care of calling
                        ioc.run().
        \param ctx      The TLS context to setup a TLS socket stream.
    */
    //=========================================================================
    WebSocketClient(const std::string& url,
                    const std::string& endpoint,
                    const std::string& port,
                    boost::asio::io_context& ioc,
                    boost::asio::ssl::context& ctx);

    //=========================================================================
    /*! \brief Destructor
    */
    //=========================================================================
    ~WebSocketClient();

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
    void connect(
      std::function<void (boost::system::error_code)> on_connect = nullptr,
      std::function<void (boost::system::error_code,
                          std::string&&)> on_message = nullptr,
      std::function<void (boost::system::error_code)> on_disconnect = nullptr);

    //=========================================================================
    /*! \brief Send a text message to the WebSocket server.

        \param message  The message to send.
        \param on_send  Called when a message is sent successfully or if it
                        failed to send.
    */
    //=========================================================================
    void send(
      const std::string& message,
      std::function<void (boost::system::error_code)> on_send = nullptr);

    //=========================================================================
    /*! \brief Close the WebSocket connection.

        \param on_close Called when the connection is closed, successfully or
                        not.
    */
    //=========================================================================
    void close(
      std::function<void (boost::system::error_code)> on_close = nullptr);

  private:
    const std::string& Url;
    const std::string& Endpoint;
    const std::string& Port;

    // Leave uninitialized because they do not support a default constructor.
    boost::asio::ip::tcp::resolver Resolver;
    boost::beast::websocket::stream<
    boost::beast::ssl_stream<boost::beast::tcp_stream>
    > Web_socket;
    boost::beast::flat_buffer Read_buffer;

    // Functions
    std::function<void (boost::system::error_code)> OnConnectPt{nullptr};
    std::function<void (boost::system::error_code,
                        std::string&&)> OnMessagePt{nullptr};
    std::function<void (boost::system::error_code)> OnDisconnectPt{nullptr};

    // Callback handlers to manage the outcome of this class' public members
    void on_resolve(const boost::system::error_code& ec,
                    boost::asio::ip::tcp::resolver::iterator endpoint);
    void on_connect(const boost::system::error_code& ec);
    void on_tls_handshake(const boost::system::error_code& err_code);
    void on_handshake(const boost::system::error_code& ec);
    void listen_to_incoming_message(const boost::system::error_code& ec);
    void on_read(const boost::system::error_code& ec, size_t nBytes);
  };
} // namespace NetworkMonitor

#endif