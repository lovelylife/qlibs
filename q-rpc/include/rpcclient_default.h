
#ifndef rpcclient_default_h__
#define rpcclient_default_h__

namespace rpc {

class rpcclient_default : public connector<caller> 
{
public:
  virtual void on_init_handler(rpc::base_stream* p) {
    handler_ = new caller(p);
    p->handler(handler_);
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

    rpc::client::iservice_proxy* s = handler_->get_service(service);
    if(NULL == s) 
      throw rpc::exception("have no service");

    handler_->call(s, method, params, res); 
  }
};

} //namespace rpc

#endif  // rpcclient_default_h__

