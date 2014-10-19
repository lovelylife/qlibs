
#ifndef qnotify_client_h__
#define qnotify_client_h__

namespace rpc {

struct inotify {
    virtual void on_initialize(const std::string& key) = 0;
    virtual void on_notify(const rpc::parameters& params) = 0;
    virtual void on_uninitialize(int step, const std::string& err_msg) = 0;
    virtual ~inotify() {}
};
/**/
class qnotify_client;

class notify_callee : public callee {
public:
  notify_callee(rpc::base_stream* s, rpc::server& r, qnotify_client* t) 
  : rpc::callee(s, r)
  , this_(t)
  {

  }

// rpc::callee overrides
public:
  virtual void on_disconnect(message_handler::stream_step step, const std::string& err_msg);

private:
  qnotify_client* this_;
};


class qnotify_client : public connector {
public:
  class qnotify_service
    : public rpc::services::iservice
  {
  public:
    qnotify_service(qnotify_client* t) : this_(t) {

    }

    ~qnotify_service() {

    }

  // rpc interfaces
  rpc_methods_begin(qnotify_service)
    rpc_method(1, id)
    rpc_method(2, notify)
  rpc_methods_end()

  public:
    void id(rpc::protocol::request& req, rpc::protocol::response& res, rpc::server_session_ptr) {
      std::string client_key = req.params()["key"].asString();
      //std::cerr << "client key: " << client_key << std::endl;
      if(this_)
        this_->on_init_key(client_key);
    }

    void notify(rpc::protocol::request& req, rpc::protocol::response& res, rpc::server_session_ptr) {
      if(this_) 
         this_->on_notify(req.params());
    }

  private:
    qnotify_client* this_;
  }; 

public:
  qnotify_client() 
  : service_(new qnotify_service(this))
  , notify_(NULL)
  {    
    rpcserver_.register_service(service_);
  }

  ~qnotify_client() 
  {
    if(service_)
      delete service_;
    if(notify_)
      delete notify_;
  }

// connector overrides
public:
  virtual rpc::message_handler* on_init_handler(rpc::base_stream* p) {
    return (new notify_callee(p, rpcserver_, this));
  }

public:
  void on_init_key(const std::string& key) {
    if(notify_)
      notify_->on_initialize(key);
  }

  void on_notify(const Json::Value& notify_data) {
    if(notify_)
      notify_->on_notify(notify_data);
  }

  void on_disconnect(message_handler::stream_step step, const std::string& err_msg) {
    if(notify_)
      notify_->on_uninitialize(step, err_msg);
  }

public:
  void set_notify_handler(rpc::inotify* p) {
    notify_ = p;
  }

private:
  rpc::server rpcserver_;
  qnotify_service* service_;
  inotify* notify_;
};

void notify_callee::on_disconnect(stream_step step, const std::string& err_msg) {
    rpc::callee::on_disconnect(step, err_msg);
    if(this_)
      this_->on_disconnect(step, err_msg);
}

} //namespace rpc

#endif //qnotify_client_h__


