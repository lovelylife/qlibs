
#ifndef caller_h__
#define caller_h__


namespace rpc {

class caller : public rpc::message_handler, public rpc::client  {
public:
  caller(rpc::base_stream* s) 
  : stream_(s) 
  , init_rpc_interface_(false) 
  , read_ok_(0)
  , write_ok_(0)
  {
      read_ok_ = event_create(false, false);
      write_ok_ = event_create(false, false);
  }

  ~caller() {
    std::cerr << "caller::~caller()" << std::endl;
  }

// interface rpc::message_handler
public:
  virtual void on_connect() {
      std::cerr << "callee_handler connected" << std::endl;
      stream_->async_read();
  }

  // Á´½Ó¶Ï¿ª
  virtual void on_disconnect() { 
    std::cerr << "callee_handler disconnected not impliement" << std::endl;
  }

  virtual void on_read_completed(const std::string& buffer) {
    std::cerr << "caller_handler read_completed." << std::endl;
    
    if(!init_rpc_interface_) {
      // intialize interface
      init_rpc_interface_ = true;      
      std::cerr << "intialize rpc interface " << buffer << std::endl;
      handler_services_proxy(buffer);        
    } else {
      text_iarchiver ar(buffer);
      ar >> message_;
      event_set(read_ok_);
    }
    stream_->async_read();
     
  }

  virtual void on_write_completed(const std::string& buffer) {  
    std::cerr << "caller_handler write_completed." << std::endl;  
    event_set(write_ok_);
  }

// rpc::client interface
public:
  virtual void send_request_noack(rpc::protocol::message& msg) 
  {
    event_reset(write_ok_);
    std::string o;
    text_oarchiver archive(o);
    archive << msg;

    stream_->async_write(o);
  }

  virtual void send_request(rpc::protocol::message& msg, int timeout=-1) 
  {
    event_reset(write_ok_);
    std::string o;
    text_oarchiver archive(o);
    archive << msg;

    stream_->async_write(o);
    int result = event_timedwait(write_ok_, 60000);
    if(1 == result) {
      throw rpc::exception("write timeout");
    } else if(-1 == result) {
      throw rpc::exception("write error");
    } else if(0 == result ){
      std::cerr << "write ok" << std::endl;
    } else {
      throw rpc::exception("unknow write error");
    }
  }

  virtual void recv_response(rpc::protocol::message& msg, int timeout=-1) 
  {
    event_reset(read_ok_);
    //stream_->async_read();
    int result = event_timedwait(read_ok_, 60000);
    if(1 == result) {
      throw rpc::exception("read timeout");
    } else if(-1 == result) {
      throw rpc::exception("read error");
    } else if(0 == result ){
      std::cerr << "read ok" << std::endl;
    } else {
      throw rpc::exception("unknow read  error");
    }

    msg = message_;
  }

private:
  RefPtr<rpc::base_stream> stream_;
  bool init_rpc_interface_;
  event_handle read_ok_;
  event_handle write_ok_;
  rpc::protocol::message message_;
};

} // namespace rpc

#endif // caller_h__


