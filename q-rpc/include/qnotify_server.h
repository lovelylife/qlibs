
#ifndef qnotify_server_h__
#define qnotify_server_h__


namespace rpc {


class qnotify_server : public acceptor 
{
public:
  qnotify_server() {
    std::cerr << "qnotify_server::qnotify_server()" << std::endl;
  }

  ~qnotify_server() {
    std::cerr << "qnotify_server::~qnotify_server()" << std::endl;
  }

public:
  void notify(const std::string& id, const rpc::parameters& params) {
    RefPtr<rpc::caller> p;	 
    {
      AutoLock<CriticalSection> lock(critical_section_);
      std::map<std::string, RefPtr<rpc::caller> >::iterator c 
	      =  clients_.find(id);
      if(c != clients_.end()) p = c->second;
    }    
    if(p) {
      rpc::protocol::response res;
      p->call("qnotify_service", "notify", params, res);
    }
  }
  
  void dump() {
    std::map<std::string, RefPtr<rpc::caller> >::const_iterator c
	    =  clients_.begin();
    while(c != clients_.end())
    {
       std::cerr << "id " <<  c->first << std::endl;
       c++;
    }
  }

// acceptor overrides
public:
  virtual bool OnAccept(RefPtr<rpc::base_stream> conn) {
    std::cerr << "qnotify accept a new connection" << std::endl;
    rpc::caller* handler = new rpc::caller(conn);
    conn->handler(handler);
    std::ostringstream os;
    void* p = (void*)handler;
    os << p;   
    AutoLock<CriticalSection> lock(critical_section_);
    clients_[os.str()] = handler;
    // write id to the notify client
    write_notify_client_id(conn, os.str());
 
    return acceptor::OnAccept(conn);
  }

  void write_notify_client_id(RefPtr<rpc::base_stream> conn, const std::string& id) {
   // write id to the notify client
    rpc::protocol::message msg;
    msg.type = rpc::protocol::message_request;
    msg.channel = 0; // notify service
    rpc::protocol::request req;
    rpc::parameters params;
    params["key"] = id;
    req.set_action(1); // id     
    req.set_params(params);
    req.to_string(msg.body);
    std::string o;
    text_oarchiver archive(o);
    archive << msg;
    conn->async_write(o);
  }

private:
  CriticalSection critical_section_;
  std::map<std::string, RefPtr<rpc::caller> > clients_;
};

} //namespace rpc

#endif // qnotify_server_h__

