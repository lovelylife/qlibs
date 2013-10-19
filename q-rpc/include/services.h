
#ifndef services_h_
#define services_h_

#include <map>
#include "rpcprotocol.h"
#include "rpcserver_session.h"


#ifndef SIMPLE_RPC_CALL
#define SIMPLE_RPC_CALL
#endif 

namespace rpc { namespace services {

struct iservice;

} }

typedef void (SIMPLE_RPC_CALL rpc::services::iservice::*rpc_func)(rpc::protocol::request&, rpc::protocol::response&, rpc::server_session_ptr);

struct rpc_method_entry {
  int id;
  const char* name;
  rpc_func func; 
};



namespace rpc { namespace services {

// 纯事件接口
struct ievent_listener {
  virtual unsigned int addListener(rpc::idispatch* d) = 0;
  virtual void removeListener(rpc::idispatch* d, unsigned int cookie) = 0;
}; 


struct iservice {
  virtual const char* name() = 0;
  virtual const rpc_method_entry* methods() = 0;
  virtual void process(rpc::protocol::request& req, rpc::protocol::response& res, rpc::server_session_ptr session) = 0 ;
  virtual ~iservice() {}
};



} } // namespace services //  } namespace rpc

//(this->*(rpc_call_func)entry->func)(req, res, session); \

#define rpc_methods_begin(className) \
public: \
  virtual const char* name() { return #className; } \
  virtual void process(rpc::protocol::request& req, rpc::protocol::response& res, rpc::server_session_ptr session) { \
    int fid = req.action(); \
    res.put_result_code(0); \
    const rpc_method_entry* entry = methods(); \
    typedef void (SIMPLE_RPC_CALL className::*rpc_call_func)(rpc::protocol::request&, rpc::protocol::response&, rpc::server_session_ptr); \
    while(entry->id > 0) { \
      if(fid == entry->id) { \
        (this->*(const rpc_call_func)(*entry).func)(req, res, session); \
        return; \
      } \
      (entry++); \
    } \
  } \
  virtual const rpc_method_entry* methods() { \
    typedef className this_class; \
    static const rpc_method_entry __methods_map[] = { 

// id 必须从大于0（!=0）开始
#define rpc_method(id, func) \
      {id, #func, (rpc_func)&this_class::func},

#define rpc_methods_end() \
      {0, "", (rpc_func)0} \
    }; \
    return &__methods_map[0]; \
  }
 

#define connection_impl(className) \
public: \
  virtual unsigned int addListener(rpc::idispatch* d) {\
    return connection_point<className>::append(d); \
  } \
  \
  virtual void removeListener(rpc::idispatch* d, unsigned int cookie) { \
    connection_point<className>::remove(d, cookie);  \
  }



#endif // services_h_


