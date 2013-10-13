
#ifndef rpcserver_session_hpp__
#define rpcserver_session_hpp__


namespace rpc { 

class server_session : public q::Object
{
  /// Constructor.
public:
  server_session(){}
  virtual ~server_session() {}

public:
  virtual rpc::idispatch* get_dispatch(connection_t t) = 0;
  //virtual connection_t connection_id() = 0;
  //virtual void write(rpc::protocol::message& msg) = 0;
  //virtual void read(rpc::protocol::message& msg) = 0;
  //virtual void notify(rpc::protocol::message& msg) = 0;
};

} // namespace rpc

namespace rpc {
  typedef rpc::server_session* server_session_ptr;
}

#endif // session_hpp__

