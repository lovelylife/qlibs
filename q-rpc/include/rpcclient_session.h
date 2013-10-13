
#ifndef rpcclient_session_h__
#define rpcclient_session_h__


namespace rpc { 

class base_stream : public q::Object
{
  /// Constructor.
public:
  client_session(){}
  virtual ~client_session() {}

public:
  virtual void write(rpc::protocol::message& msg, int timeout=-1) = 0;
  virtual void read(rpc::protocol::message& msg,  int timeout=-1) = 0;
  virtual void close() = 0;

  virtual rpc::message_handler* handler() = 0;
};

} // namespace rpc

namespace rpc {
  typedef rpc::client_session* client_session_ptr;
}

#endif // rpcclient_session_h__

