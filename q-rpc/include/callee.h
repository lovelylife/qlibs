#ifndef callee_h__
#define callee_h__


namespace rpc {

class callee : public rpc::message_handler {
public:
  callee(rpc::base_stream* s, rpc::server& r) 
  : stream_(s) 
  , rpcserver_(r)
  {
    
  }

// interface rpc::message_handler
public:
  void on_connect() {
    std::cerr << "callee_server:: callee_handler::on_connect()" << std::endl;
    std::string s;
    //@todo
    rpcserver_.dump_services_info(s);
    std::cerr << "export interface to caller." << std::endl;
    stream_->async_write(s);
  }
    
  void on_disconnect() {
    std::cerr << "callee_server:: callee_handler::on_disconnect()" << std::endl;
  }

  void on_read_completed(const std::string& buf) {
    //std::cerr << "callee_server:: callee_handler::on_read_completed()" << std::endl;
    rpc::protocol::message msg;
    text_iarchiver ar(buf);
    ar >> msg;
    rpc::protocol::message res;
    //@todo
    rpcserver_.handle_message(msg, res, NULL);
    if(msg.type == rpc::protocol::message_request_void) {
      stream_->async_read();
      return;
    }

    std::string o;
    text_oarchiver oar(o);
    oar << res;
    stream_->async_write(o);
  }

  void on_write_completed(const std::string& buf) {
    //std::cerr << "callee_server:: callee_handler::on_write_completed()" << std::endl;
    stream_->async_read();
  }

private:
  RefPtr<rpc::base_stream> stream_;
  rpc::server& rpcserver_;
}; // class callee

} // namespace rpc
#endif //callee_h__

