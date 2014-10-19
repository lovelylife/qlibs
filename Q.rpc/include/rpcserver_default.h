
#ifndef rpcserver_default_h__
#define rpcserver_default_h__


namespace rpc {


class rpcserver_default : public acceptor 
{
public:
  // register rpc service
  void register_service(rpc::services::iservice* s) {
    rpcserver_.register_service(s);
  }

// acceptor overrides
public:
  virtual bool OnAccept(RefPtr<rpc::base_stream> conn) {
    rpc::callee* handler = new rpc::callee(conn, rpcserver_);
    conn->handler(handler);
    static int i= 0;
    clients_[++i] = handler;
    return acceptor::OnAccept(conn);
  }

private:
  std::map<int, RefPtr<rpc::callee> > clients_;
  rpc::server rpcserver_;
};


} // namespace rpc

#endif // rpcserver_default_h__

