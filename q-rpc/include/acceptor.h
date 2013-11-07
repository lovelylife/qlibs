
#ifndef acceptor_h__
#define acceptor_h__

namespace rpc {


class io_thread : public q::Thread {
public:
  io_thread(boost::asio::io_service& io) 
  : io_service_(io) 
  , work_(io)
  {

  }

protected:
  bool loop() 
  {
    io_service_.run();
    return false;
  }

private:
  boost::asio::io_service& io_service_;
  boost::asio::io_service::work work_; 
};  // io_thread


class acceptor {
public:
  acceptor()
  : io_acceptor_(0)
  , io_thread_(0)
  {
    // Start an accept operation for a new connection.
  }
  
  virtual ~acceptor() {
    stop();
  }

  bool listen(unsigned short port) {
    assert(NULL == io_acceptor_);
    if(NULL == io_acceptor_) { 
      io_acceptor_ = new boost::asio::ip::tcp::acceptor(io_service_, 
                         boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
    }
    RefPtr<rpc::stream_boost_impl> new_connection(new rpc::stream_boost_impl(io_acceptor_->get_io_service()));
    io_acceptor_->async_accept(new_connection->socket(),
        boost::bind(&acceptor::handle_accept, this,
        boost::asio::placeholders::error, new_connection));
       
    io_thread_ = new rpc::io_thread(io_service_);
    io_thread_->start(); 
    
    return true;
  }

  /// Handle completion of a accept operation.
  void handle_accept(const boost::system::error_code& e, RefPtr<rpc::stream_boost_impl> conn)
  {
    if (!e) {
      
      try {  
        // create handler
        if (conn != NULL) {  
          //boost::format fmt("%1%:%2%");  
          //fmt % conn->socket().remote_endpoint().address().to_string();  
          //fmt % conn->socket().remote_endpoint().port();  
          //conn->set_remote_addr(fmt.str());  
          OnAccept(conn);	
        }  
      } catch(std::exception& e) {  
        std::cerr << "with exception:[" << e.what() << "]" << std::endl;  
      } catch(...)  {  
        std::cerr <<  "unknown exception." << std::endl;  
      }  
       // Start an accept operation for a new connection.
      RefPtr<rpc::stream_boost_impl> new_conn(new rpc::stream_boost_impl(io_acceptor_->get_io_service()));
      io_acceptor_->async_accept(new_conn->socket(),
 	    boost::bind(&acceptor::handle_accept, this,
	    boost::asio::placeholders::error, new_conn));
    }   
  }


  void stop() {
    if(io_acceptor_)
      delete io_acceptor_;
    
    io_acceptor_ = 0;

    if(io_thread_ &&io_thread_->running()) 
      io_thread_->stop();
    
    delete io_thread_;   
    io_thread_ = 0;
  }

  virtual bool OnAccept(RefPtr<rpc::base_stream> conn) {
    if(conn->handler())
      conn->handler()->on_connect();
    return true;
  }

private:
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor* io_acceptor_;
  io_thread* io_thread_;	
}; // class acceptor


} //namespace rpc

#endif // acceptor_h__


