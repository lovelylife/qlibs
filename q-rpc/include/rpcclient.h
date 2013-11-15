
#ifndef rpcclient_h__
#define rpcclient_h__

namespace rpc {

struct ievent : base::RefCountedThreadSafe<ievent>
{
  virtual void notify(int id, const rpc::parameters& evt) = 0;
  virtual ~ievent() {}
};


class queue_map_impl {
public:
  queue_map_impl()  {}
  ~queue_map_impl() {}

public:  
  // add a item to message map
  void add_message(rpc::cookie_t& cookie, rpc::ievent* handler) {
    AutoLock<CriticalSection> lock(msg_critical_);
    message_map_[cookie] = handler;
  }

  // remove listener
  void remove_message(rpc::cookie_t& cookie) {
    AutoLock<CriticalSection> lock(msg_critical_);
    std::map<rpc::cookie_t, RefPtr<rpc::ievent> >::iterator 
      i = message_map_.find(cookie);
    
    if(i != message_map_.end()) {
      message_map_.erase(i);
    }
  }

  // dispatch message to handler
  void on_message_process(rpc::protocol::message& msg) {
    rpc::protocol::event res;
    res.from_string(msg.body);
    rpc::cookie_t cookie = res.body()["cookie"].asInt();
    std::cerr << "new event come: " << cookie << std::endl;
    
    AutoLock<CriticalSection> lock(msg_critical_);
    std::map<rpc::cookie_t, RefPtr<rpc::ievent> >::const_iterator c = message_map_.find(cookie);
    if(c!= message_map_.end()) {
      RefPtr<rpc::ievent> e = c->second;
      if(e) e->notify(res.body()["id"].asInt(), res.body()["parameters"]);
    }
  }

private:
  CriticalSection msg_critical_;
  std::map<rpc::cookie_t, RefPtr<rpc::ievent> > message_map_;
};

	
class client
{
public:
  class iservice_proxy {
  public:
    virtual int channel() = 0;
    virtual const char* name() = 0;
    virtual int func(const std::string& name) = 0;
  };

  class service_proxy : public iservice_proxy
  {
  public:
    service_proxy(int c, const std::string& name, std::map<std::string, int> funcs) 
    : channel_(c)
    , name_(name)
    , functions_(funcs)
    {
    }

    ~service_proxy(){}

    // iservice_proxy
    public:
      virtual int channel() { return channel_; }
      virtual const char* name() { return name_.c_str(); }
      virtual int func(const std::string& name) 
      { 
        std::map<std::string, int>::const_iterator c = functions_.find(name);
        if(c != functions_.end()) 
          return c->second;
        return -1; 
      }

    private:
      int channel_;
      std::string name_;
      std::map<std::string, int> functions_;
    };

// 构造析构
public:
  client() {}
  ~client() {}


public:
  virtual void on_connect() {
  }

  virtual void on_disconnect() 
  {
  }
	  
public:
  void call(const std::string& service_name, 
    const std::string& method,
    const rpc::protocol::request::parameters& params, 
    rpc::protocol::response& res) 
  {
    iservice_proxy* s = get_service(service_name);
    call(s, method, params, res);
  }

  void vcall(const std::string& service_name, 
    const std::string& method, 
    const rpc::parameters& params) 
  {
    rpc::protocol::response res;
    iservice_proxy* s = get_service(service_name);
    call(s, method, params, res, false);
  }
 
  void call(iservice_proxy* s,
    const std::string& method,
    const rpc::protocol::request::parameters& params, 
    rpc::protocol::response& res, bool vcall = false) 
  {
    if(NULL == s) {
      throw rpc::exception("invalid argument iservice_proxy*");	
    }
		
    int func_id = s->func(method);
    if(func_id < 0) {
      std::stringstream oss;
      oss << s->name() << "have no methods: " << method << std::endl;
      throw rpc::exception(oss.str().c_str());
    }
    rpc::protocol::message msg_response;
    rpc::protocol::message msg_request;
    msg_request.type = (!vcall)?rpc::protocol::message_request:rpc::protocol::message_request_void;
    msg_request.channel = s->channel();

    rpc::protocol::request req;
    req.set_action(func_id);
    req.set_params(params);		
    req.to_string(msg_request.body);
    if(!vcall) {
      rpc::protocol::message msg_response;
      // call
      call_proxy(msg_request, msg_response);	
      res.from_string(msg_response.body);
    } else {
      call_proxy_void(msg_request);
    }
    
  }

  void addListener(iservice_proxy* s, ievent* handler, rpc::cookie_t& cookie)
  {
    if(NULL == s) {
      throw rpc::exception("invalid argument iservice_proxy*");	
    }
    rpc::protocol::message msg_response;
    rpc::protocol::message msg_event;
    msg_event.type = rpc::protocol::message_event;
    msg_event.channel = s->channel();

    rpc::protocol::event req;
    req.set_action(rpc::protocol::event_add);
    req.body()["key"] = queue_client_key();
    req.to_string(msg_event.body);

    // call		
    call_proxy(msg_event, msg_response);

    // 实际使用代码
    rpc::protocol::event res;
    res.from_string(msg_response.body);
    cookie = res.body()["cookie"].asInt();
    res.dump();    
  }

  rpc::cookie_t removeListener(iservice_proxy* s, const rpc::cookie_t& cookie) 
  {
    if(NULL == s) {
      throw rpc::exception("invalid argument iservice_proxy*");	
    }
    rpc::protocol::message msg_response;
    rpc::protocol::message msg_event;
    msg_event.type = rpc::protocol::message_event;
    msg_event.channel = s->channel();

    rpc::protocol::event req;
    req.set_action(rpc::protocol::event_remove);
    req.body()["key"] = queue_client_key();
    req.to_string(msg_event.body);

    // call		
    call_proxy(msg_event, msg_response);

    // 实际使用代码
    rpc::protocol::event res;
    res.from_string(msg_response.body);
  }

  iservice_proxy* get_service(const std::string& service_name) 
  {
    std::map<std::string, iservice_proxy*>::const_iterator c = service_proxy_objects_.find(service_name);
    if(c != service_proxy_objects_.end()) {
      return c->second;
    }

    return 0;
  }

// override by network layer
private:
  virtual void send_request_noack(rpc::protocol::message& msg) {
    
  }

  virtual void send_request(rpc::protocol::message& msg, int timeout=-1) 
  {
    //connection_.write();
  }

  virtual void recv_response(rpc::protocol::message& msg, int timeout=-1) 
  {
    //connection_.read();
  }

  virtual std::string queue_client_key() 
  {
    return std::string("");
  }

  void call_proxy(rpc::protocol::message& input_msg, rpc::protocol::message& output_msg) 
  {
    send_request(input_msg);
    // 实际使用代码
    recv_response(output_msg);    
  }

  void call_proxy_void(rpc::protocol::message& input_msg) {
    send_request_noack(input_msg);
  }

public:
  void handler_services_proxy(const std::string& services_info) {
    rpc::protocol::serializer ar;
    ar.from_string(services_info);
	
    // jsoncpp version
    const Json::Value& array = ar.body()["services"];
    for(size_t i=0; i < array.size(); i++) {
      int service_id = array[i]["id"].asInt();
      std::string service_name = array[i]["name"].asString();
      std::map<std::string, int> funcs;
      const Json::Value& func_array = array[i]["funcs"];
      for(size_t j=0; j < func_array.size(); j++) {
        int func_id = func_array[j]["id"].asInt();
        std::string func_name = func_array[j]["name"].asCString();
        funcs[func_name] = func_id;
      }
      
      service_proxy_objects_[service_name] = new service_proxy(service_id, service_name, funcs);
    }
  }

  bool ok() {
    return (service_proxy_objects_.size() > 0);
  }

private:
  std::map<std::string, iservice_proxy*> service_proxy_objects_;
};

} // namespace rpc


#endif // rpcclient_h__

