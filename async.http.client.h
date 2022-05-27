#ifndef ALIAS__ASYNC_HTTP_CLIENT_H_
#define ALIAS__ASYNC_HTTP_CLIENT_H_
class Client
    : private boost::noncopyable {
 public:
  explicit server(const std::string &address, const std::string &port,
                  std::size_t thread_pool_size);

  /// Run the server's io_context loop.
  void run();

 private:
  /// Initiate an asynchronous accept operation.
  void start_accept();

  /// Handle completion of an asynchronous accept operation.
  void handle_accept(const boost::system::error_code &e);

  /// Handle a request to stop the server.
  void handle_stop();

 private:

  /// The io_context used to perform asynchronous operations.
  boost::asio::io_context io_context_;

  /// The signal_set is used to register for process termination notifications.
  boost::asio::signal_set signals_;

  /// Acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  /// The next connection to be accepted.
  boost::shared_ptr<ClientConnection> new_connection_;

  /// The handler for all incoming requests.
//  Router<std::string (*)(const Request &request)> request_router;

};
#endif //ALIAS__ASYNC_HTTP_CLIENT_H_
