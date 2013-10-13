
#ifndef rpcserver_default_h__
#define rpcserver_default_h__

#include "rpcserver.h"
//#include "message_handler.h"

namespace rpc {

class server_default;

// callee server
class callee_server : public SERVER
{
public:
  callee_server(server_default* s) : rpc_serv_(s) {}
  ~callee_server() {}

public:
  class callee_handler 
  : public rpc::message_handler
  , public rpc::services::session
  {
  public:
    callee_handler(IBaseStream& r)
    : rpc::message_handler(r)
    {
    }

    ~callee_handler() {}

  public:
    virtual void on_connect();
    virtual void on_disconnect();
    virtual bool on_message(const std::string& buf);
  
  // rpc::services::session
  public:
    virtual rpc::idispatch* get_dispatch(rpc::connection_t c); 

  public:
    server_default* this_;
  };

// SERVER interface
public:
  virtual bool OnAccept(RefPtr<STREAM> t) {
    callee_handler* p = StreamDecorator<callee_handler>(t);
    p->this_ = rpc_serv_;
    return SERVER::OnAccept(t);
  }

private:
  server_default* rpc_serv_;  
};



////////////////////////////////////////////////////////////////////////////////
// queue

/*
class queue_server;

typedef struct queue_client_message_ {
  rpc::connection_t connection;
  rpc::protocol::message message;
} queue_client_message;

class queue_send_thread : public CBaseThread {
public:
  queue_send_thread()  {}
  ~queue_send_thread() {}

// CBaseThread
public:
  virtual bool ThreadLoop();

// methods
public:
  void post(const queue_client_message& t) {
    queue_objects_.push(t);
  }
  
private:
  queue_server* this_;
  sem_queue<queue_client_message> queue_objects_;
};
*/

class queue_server : public SERVER
{
public:
  queue_server(server_default* s) : rpc_serv_(s) {}
  ~queue_server() {}

public:
  class queue_handler : public rpc::message_handler, public rpc::idispatch
  {
  public:
    queue_handler(IBaseStream& r) : rpc::message_handler(r) {}
    ~queue_handler() {}
  
  // rpc::message_handler
  public:
    virtual void on_connect()    { 
      std::cerr << "new queue client(key:" << key_ << ") connected!" << std::endl; 
      // send queue client key
      //rpc::protocol::message msg;
      //msg.body = key_;
      //write_message(msg);
    }

    virtual void on_disconnect() {
      // queue client disconnect
      std::cout << "queue client disconnected!" << std::endl; 
    }
    
    // queue_client 服务端不读取客户端的消息   
    virtual bool on_message(const std::string& buf) { return false; }

  // rpc::idispatch
  public:
    virtual void invoke(int id, const rpc::parameters& params, int cookie) {
      // write data to client 
      rpc::protocol::event res;
      res.set_action(rpc::protocol::event_notify);
      res.body()["id"] = id;
      res.body()["key"] = key_;
      res.body()["cookie"] = cookie;
      res.body()["parameters"] = params;

      rpc::protocol::message msg;
      msg.type = rpc::protocol::message_event;
      res.to_string(msg.body);
      write_message(msg);
    }

  public:
    std::string key_;
    rpc::server_default* this_;
  };

  CRefObj<queue_handler> get(rpc::connection_t t) {
    CAutoLock<CCrtSection> lock(critical_section_);
    std::map<rpc::connection_t, CRefObj<queue_handler> >::const_iterator 
    c = queue_clients_.find(t);
    if(c != queue_clients_.end())
      return c->second;

    return 0;  
  }
  
  
  //void post_to_client(const queue_client_message& t) {
  //    CAutoLock<CCrtSection> lock(critical_section_);
  //    std::map<rpc::connection_t, CRefObj<queue_handler> >::const_iterator 
  //    c = queue_clients_.find(t.connection);
  //    if(c != queue_clients_.end()) {
  //      CRefObj<queue_handler> p = c->second;
  //    }
  //}

// SERVER interface
public:
  virtual bool OnAccept(CRefObj<STREAM> t) {
    //std::cerr << "new event client connected" << std::endl;
	queue_handler* p = StreamDecorator<queue_handler>(t);
    
    //生成queue_client_key     
    void* ptr = (void*)p;
    std::stringstream strm;
    strm << ptr;
    std::string key = strm.str(); 

    // 绑定数据
    p->this_ = rpc_serv_;
    p->key_ = key;

    // 保存链接
    { 
    //  CAutoLock<CCrtSection> lock(critical_section_);
    //  queue_clients_[key] = p;
    }

    return SERVER::OnAccept(t);
  }

private:
  server_default* rpc_serv_;  
  std::map<rpc::connection_t, RefPtr<queue_handler> > queue_clients_;
  CCrtSection critical_section_;
};


// this is the default implimention of rpc::server use phstream
class server_default : public rpc::server
{
public:
  server_default()
  : callee_server_(this)
  , queue_server_(this)
  {
  
  }

  ~server_default() {}

public:
  bool start(const std::string& address, short callee_port, short event_port=12345) {
    reactor_.Run(1);
  
    // callee server  // event server
    return callee_server_.StartListen(address.c_str(), callee_port, &reactor_.Tracker())
    &&  queue_server_.StartListen(address.c_str(), event_port, &reactor_.Tracker());
  }

  void stop() {
    reactor_.Stop();
  }

// methods
public:
  rpc::idispatch* get_dispatch(rpc::connection_t c) {
    return queue_server_.get(c);
  }

private:
  // call server
  rpc::callee_server callee_server_;
  // event_server
  rpc::queue_server queue_server_;
  // session manager
  typedef std::map<int, rpc::session_ptr> sessions;
  sessions sessions_;
  REACTOR reactor_;

};

} // namespace rpc

#endif // rpcserver_default_h__

