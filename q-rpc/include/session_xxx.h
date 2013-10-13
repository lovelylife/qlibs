

#ifndef session_default_h__
#define session_default_h__

#include "session.hpp"

namespace rpc {

typedef STREAM transport;

class session_default : public rpc::services::session
{
public:
  session_default();
  ~session_default();

public:
  virtual void write(rpc::protocol::message& msg) {

  }

  virtual void read(rpc::protocol::message& msg) {

  }

  virtual void notify(rpc::protocol::message& msg) {

  }


private:
  CRefObj<rpc::transport> rpc_call_;
  CRefObj<rpc::transport> rpc_event_;
};


} // namespace rpc

#endif // session_default_h__

