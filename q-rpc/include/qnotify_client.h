
#ifndef qnotify_client_h__
#define qnotify_client_h__

namespace rpc {

struct inotify {
    virtual void on_notify(const rpc::parameters& params) = 0;
    virtual ~inotify() {}
};

class qnotify_client : public connector<callee> {

public:
  class qnotify_service
    : public rpc::services::iservice
  {
  public:
    qnotify_service()
    : notify_(NULL) {

    }

    ~qnotify_service() {
       if(notify_) {
         delete notify_;
       }
    }

    void set_notify(inotify* n) {
      notify_ = n;
    }

  // rpc interfaces
  rpc_methods_begin(qnotify_service)
    rpc_method(1, id)
    rpc_method(2, notify)

  rpc_methods_end()

  public:
    void id(rpc::protocol::request& req, rpc::protocol::response& res, rpc::server_session_ptr) {
       std::string client_key = req.params()["key"].asString();
       std::cerr << "client key: " << client_key << std::endl;
    }

    void notify(rpc::protocol::request& req, rpc::protocol::response& res, rpc::server_session_ptr) {
      if(notify_) {
         notify_->on_notify(req.params());
      }
    }

  private:
    inotify* notify_;
  }; 

public:
  qnotify_client() 
  : service_(new qnotify_service)  
  {
    
    rpcserver_.register_service(service_);
  }

  ~qnotify_client() {
    if(service_)
      delete service_;
  }

public:
  virtual void on_init_handler(rpc::base_stream* p) {
    handler_ = new callee(p, rpcserver_);
    p->handler(handler_);
  }

public:
  void set_notify_handler(rpc::inotify* p) {
    service_->set_notify(p);
  }

private:
  rpc::server rpcserver_;
  qnotify_service* service_;
};

} //namespace rpc

#endif //qnotify_client_h__


