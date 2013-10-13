
#ifndef rpcclient_default_h__
#define rpcclient_default_h__

#include "rpcclient.h"
#include "message_handler.h"
#include "event.h"

namespace rpc {

class queue_map_impl {
public:
  queue_map_impl()  {}
  ~queue_map_impl() {}

public:  
  // add a item to message map
  void add_message(rpc::cookie_t& cookie, rpc::client::ievent* handler) {
    CAutoLock<CCrtSection> lock(msg_critical_);
    message_map_[cookie] = handler;
  }

  // remove listener
  void remove_message(rpc::cookie_t& cookie) {
    CAutoLock<CCrtSection> lock(msg_critical_);
    std::map<rpc::cookie_t, CRefObj<rpc::client::ievent> >::iterator 
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
    
    CAutoLock<CCrtSection> lock(msg_critical_);
    std::map<rpc::cookie_t, CRefObj<rpc::client::ievent> >::const_iterator c = message_map_.find(cookie);
    if(c!= message_map_.end()) {
      CRefObj<rpc::client::ievent> e = c->second;
      if(e) e->notify(res.body()["id"].asInt(), res.body()["parameters"]);
    }
  }

private:
  CCrtSection msg_critical_;
  std::map<rpc::cookie_t, CRefObj<rpc::client::ievent> > message_map_;
};

class queue_client : public queue_map_impl
{
public:
  queue_client()  {
    // create read and write event handler
    event_connect_ok_ = event_create(false, false);
  }

  ~queue_client() {
    std::cerr << "queue_client::~queue_client()" << std::endl;
    if(stream_)
      stream_->Disconnect();
  }

public:
  class queue_handler : public rpc::message_handler
  {
  public:
    queue_handler(IBaseStream& r) : rpc::message_handler(r), have_handler_key_(false) {}
    virtual ~queue_handler() {

      std::cerr << "queue_handler::~queue_handler()" << std::endl;
      //transport_.Disconnect();
    }

  public:
    virtual void on_connect() {
      //read_header_ok_ = false;
      //transport_.Read(0, sizeof(rpc::protocol::package_header), -1);
      //this_->on_connect(); 
      this_->on_init_ok("test");
    }

    virtual void on_disconnect() {
      // this_->on_disconnect(); 
      std::cerr<< "queue client service unavaible" << std::endl;
    }

    virtual bool on_message(const std::string& buf) { 
      rpc::protocol::message msg; 
      text_iarchiver ar(buf);
      ar >> msg;

      if(!have_handler_key_) {
        // 读取queue_client_key
        have_handler_key_ = true;
	this_->on_init_ok(msg.body);
      } else {
	// 将消息丢给queue_client处理
        this_->on_message_process(msg);
      }

      // read next message
      return true;
    }
    
  
  public:
    queue_client* this_;
  private:
    bool have_handler_key_;
  };

public:
  void connect(const std::string& address, unsigned short port, int timeout) 
  {
    event_reset(event_connect_ok_);
    stream_  = new STREAM(IBaseStream::eClient);
    handler_  =  StreamDecorator<queue_handler>(stream_);
    handler_->this_ = this;
    connector_.Connect(stream_, address.c_str(), port, &reactor_.Tracker()); 
     
    int r = event_timedwait(event_connect_ok_, timeout);
    if(r !=0)
      throw rpc::exception("message queue server is invalid");
    event_reset(event_connect_ok_);
  }

  REACTOR& reactor() { return reactor_; }
  std::string key() const { return key_; }

public:
  void on_init_ok(const std::string& key) { 
    key_ = key;
    //std::cerr << "queue client key:" << key << std::endl;
    event_set(event_connect_ok_); 
  }

//  void on_connect() { event_set(event_connect_ok_); }
//  void on_disconnect() {}
  
private:
  // 消息队列客户端id
  CRefObj<queue_handler> handler_;
  CRefObj<STREAM> stream_;
  CONNECTOR connector_;
  REACTOR reactor_;
  event_handle event_connect_ok_;
  std::string key_;
};

class rpcclient_default
{
public:
  rpcclient_default() {
    event_connect_ok_ = event_create(false, false);
  }

  ~rpcclient_default() {}


public:
  class caller_handler 
  : public rpc::message_handler
  , public rpc::client
  {
  public:
    caller_handler(IBaseStream& r) 
    : rpc::message_handler(r)
    , have_handler_services_(false)
    {
      // create read and write event handler
      read_data_event_ = event_create(false, false);
      write_data_event_ = event_create(false, false);
    }
    ~caller_handler() {}

  public:
    // 与服务器建立链接
    virtual void on_connect() { 
      std::cerr << "caller server avaiable" << std::endl;
      read_header_ok_ = false;
      int ret_code = transport_.Read(0, sizeof(rpc::protocol::package_header), -1);
    }
    // 与服务器断开链接
    virtual void on_disconnect() { std::cerr << "caller server unavaiable" << std::endl; }

    // 收到一条消息
    virtual bool on_message(const std::string& buf) {
      text_iarchiver ar(buf);
      ar >> message_;

      if(!have_handler_services_) {
        have_handler_services_ = true;
        handler_services_proxy(message_.body);
        this_->on_init_ok();
      }

      // 一个消息接收完成
      event_set(read_data_event_);

      // not read next message
      return false;
    }

  // IBaseStream::IBaseHandler
  public:
    virtual bool Handle(IBASESTREAM_PARAM pStr,IBaseStream::NotifyType notify, IBUFFER_PARAM pBuf,unsigned long transf);

  // rpc::rpcclient 发送和接收消息接口, 事件客户端key
  public:
    virtual void send_request(rpc::protocol::message& msg)  { send_message(msg); }
    virtual void recv_response(rpc::protocol::message& msg) { recv_message(msg); }
    virtual std::string queue_client_key() {  return this_->queue_client_key(); }
  
  // 发送和接收消息
  public:
    void recv_message(rpc::protocol::message& msg, int timeout = -1);
    void send_message(rpc::protocol::message& msg, long timeout = -1);
  
  public:
    rpc::rpcclient_default* this_;
  
  private:
    event_handle read_data_event_;
    event_handle write_data_event_;
    rpc::protocol::message message_;
    bool have_handler_services_;
  };

// static initialize
public:
  void initialize(const std::string& address, int timeout) {
    queue_client_.reactor().Run(1);
    queue_client_.connect(address, 12345, timeout);
    std::cerr << "init event queue(key:"<< queue_client_.key() <<") ok!" << std::endl;
  }


public:
  void connect(const std::string& address, short port, int timeout=-1); 

  rpc::client::iservice_proxy* get_service(const std::string& service_name)
  {
      return handler_->get_service(service_name);
  }

  void call(rpc::client::iservice_proxy* s, 
    const std::string& methods, 
    rpc::protocol::request::parameters params, 
    rpc::protocol::response& res) 
  {
    handler_->call(s, methods, params, res);
  } 

  void addListener(rpc::client::iservice_proxy* s, rpc::client::ievent* handler, rpc::cookie_t& cookie) {
    handler_->addListener(s, handler, cookie);
    // add to queue client message map
    queue_client_.add_message(cookie, handler);
  }
public:
  void on_init_ok() { 
    std::cerr << "init ok" << std::endl;
    event_set(event_connect_ok_);
  }

  std::string queue_client_key() { return queue_client_.key(); }

private:
  REACTOR reactor_;
  CONNECTOR connector_;
  CRefObj<STREAM> stream_;
  CRefObj<caller_handler> handler_;
  event_handle event_connect_ok_;

  // 共享唯一消息队列客户端
  rpc::queue_client queue_client_;
};


} //namespace rpc

#endif  // rpcclient_default_h__

