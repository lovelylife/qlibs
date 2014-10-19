
#ifndef rpcclient_default_h__
#define rpcclient_default_h__

namespace rpc {

class rpcclient_default : public connector
{
public:
  virtual rpc::message_handler* on_init_handler(rpc::base_stream* p)
  {
    rpc::caller* c = (new rpc::caller(p));
    c_ = c;
    return c;
  }

public:
  void call(const std::string& service, 
            const std::string& method, 
            const rpc::parameters& params,
            rpc::protocol::response& res)
  {
    if(NULL == handler_) {
      throw rpc::exception("invalid rpc caller.");
    }

    rpc::client::iservice_proxy* s = c_->get_service(service);
    if(NULL == s) 
      throw rpc::exception("have no service");

    c_->call(s, method, params, res); 
  }
private: 
  RefPtr<caller> c_;
};

} //namespace rpc

#endif  // rpcclient_default_h__

