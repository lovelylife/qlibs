
#ifndef connector_h__
#define connector_h__


namespace rpc {

class connector : public q::Object 
{
public:
  connector() 
  : io_service_()
  , io_thread_(io_service_)
  {
    event_connect_ok_ = event_create(false, false);
  };
 
  virtual ~connector() {
    io_service_.stop();
    io_thread_.stop();
    if(event_connect_ok_) {
      event_destroy(event_connect_ok_);
    }
  }
 
public:
  virtual rpc::message_handler* on_init_handler(rpc::base_stream*) = 0;

public:
  void connect(const std::string& host,  const std::string& service, int timeout = -1) {
    rpc::stream_boost_impl* p = new rpc::stream_boost_impl(io_service_);
    handler_ = on_init_handler(p);
    p->handler(handler_);

    // Resolve the host name into an IP address.
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::resolver::query query(host, service);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    // Start an asynchronous connect operation.
    boost::asio::async_connect(
      p->socket(), 
      endpoint_iterator,
      boost::bind(&connector::handle_connect, this,
      boost::asio::placeholders::error)
    );

    std::cerr << "client_base::connect" << std::endl;
    if(!io_thread_.running())
      io_thread_.start();

    std::cerr << "client_base::connect 1" << std::endl;
  
    // wait for connect
    int r = event_timedwait(event_connect_ok_, timeout);
    if(event_timeout == r) {
      std::cerr << "connect timout" << std::endl;
      throw r;
    } else if(event_error == r){
      std::cerr << "connect error" << std::endl;
    }
    std::cerr << "client_base::connect 2" << std::endl;
  }

private:
  /// Handle completion of a connect operation.
  void handle_connect(const boost::system::error_code& e){
    if (!e) {
      if(handler_)
        handler_->on_connect();
    } else {
      std::cerr << e.message() << std::endl;
      if(handler_)
         handler_->on_disconnect();
    }
    event_set(event_connect_ok_);
  }

protected:
  boost::asio::io_service io_service_;
  io_thread io_thread_;
  event_handle event_connect_ok_;
  RefPtr<rpc::message_handler> handler_;
}; //class connector

} // namespace rpc

#endif // connector_h__

