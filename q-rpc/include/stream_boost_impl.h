
#ifndef stream_boost_impl_h__
#define stream_boost_impl_h__


namespace rpc {


class stream_boost_impl : public rpc::base_stream 
{
public:

  ~stream_boost_impl() 
  {
     std::cerr << "~stream_boost_imp()" << std::endl;
     socket_.close();
  }

  stream_boost_impl(boost::asio::io_service& io_service)
  : socket_(io_service)
  , handler_(0)
  {
    //read_data_ok_  = event_create(false, false);
    //write_data_ok_ = event_create(false, false);
    std::cerr << "stream_boost_imp()" << std::endl;
  }

// base_stream 
public:
  // read data
  virtual void read(const std::string& buffer, int timeout=-1) {
  
  }

  // write data
  virtual void write(const std::string& buffer, int timeout=-1) {
  
  }

  // get or set handler
  virtual rpc::message_handler* handler(rpc::message_handler* new_handler = 0) {
    if(new_handler == 0)
      return handler_;

    rpc::message_handler* old = handler_;
    handler_ = new_handler;

    return old;
  }

public:
  /// Get the underlying socket. Used for making a connection or for accepting
  /// an incoming connection.
  boost::asio::ip::tcp::socket& socket()
  {
    return socket_;
  }

  /// Asynchronously write a data structure to the socket.
  void async_write(const std::string& ostr)
  {
    // Serialize the data first so we know how large it is.
    //std::string o = ostr;
    //text_oarchiver archive(o);
    //archive << t;
    outbound_data_ = ostr;
    char* c = &(outbound_data_[0]); // copy on write
    assert(outbound_data_.data() != ostr.data());

    // Format the header.
    out_pkg_header_.size = outbound_data_.size();		
		
    // Write the serialized data to the socket. We use "gather-write" to send
    // both the header and the data in a single write operation.
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer((void*)&out_pkg_header_, sizeof(protocol::package_header)));
    buffers.push_back(boost::asio::buffer(outbound_data_));

    boost::asio::async_write(socket_, 
    	buffers, 
    	boost::bind(&stream_boost_impl::handle_write_completed, this,
    	boost::asio::placeholders::error));
  }

  void handle_write_completed(const boost::system::error_code& e) {
    if(!e) {
      if(handler_)
        handler_->on_write_completed(outbound_data_);
      //event_set(write_data_ok_);
    } else {
      if(handler_)
        handler_->on_disconnect();
    }
  }

  /// Asynchronously read a data structure from the socket.
  void async_read()
  {
    // Issue a read operation to read exactly the number of bytes in a header.
    boost::asio::async_read(socket_, 
      boost::asio::buffer((void*)&in_pkg_header_, sizeof(protocol::package_header)),
      boost::bind(&stream_boost_impl::handle_read_header,  this, 
      boost::asio::placeholders::error));
  }

  void handle_read_header(const boost::system::error_code& e)
  {
    if (e) {
      if(handler_)
        handler_->on_disconnect();
      //throw rpc::exception("network error");
    } else {
      // Start an asynchronous call to receive the data.
      inbound_data_.resize(in_pkg_header_.size);

      boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
         boost::bind(&stream_boost_impl::handle_read_data, this,
         boost::asio::placeholders::error 
      ));
    }
  }

  /// Handle a completed read of message data.
  void handle_read_data(const boost::system::error_code& e)
  {
    if (e) {
      if(handler_)
        handler_->on_disconnect();
    } else {
      std::string archive_data(&inbound_data_[0], inbound_data_.size());
      if(handler_) 
        handler_->on_read_completed(archive_data);
    }
  }

private:
  rpc::protocol::package_header out_pkg_header_;
  rpc::protocol::package_header in_pkg_header_;
  rpc::protocol::message message_;
  std::string       outbound_data_;
  std::vector<char> inbound_data_;
  
  boost::asio::ip::tcp::socket socket_;
  rpc::message_handler* handler_;
  
//  event_handle read_data_ok_;
//  event_handle write_data_ok_;
  
}; // stream_boost_impl


} // namespace rpc

#endif // stream_boost_impl_h__

