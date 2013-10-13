

#ifndef connection_point_h__
#define connection_point_h__

#include "dispatch.h"

namespace rpc {

typedef struct c_ {
    cookie_t     cookie;    // cookie
    RefPtr<rpc::idispatch> dispatch;  // 事件分发id
} connection;

static cookie_t new_cookie();

template<class T>
class connection_point {
// constructor/destructor
public:
  connection_point()  {}
  ~connection_point() {}

public:
  cookie_t append(rpc::idispatch* dispatch) {
    //CAutoLock<CCrtSection> lock(critical_section_);
    connection c;
    c.cookie = new_cookie();
    c.dispatch = dispatch;
    points_.push_back(c);

    return c.cookie;
  }
  
  void remove(rpc::idispatch* dispatch, cookie_t c) {
    //CAutoLock<CCrtSection> lock(critical_section_);
    std::list<connection>::iterator i = points_.begin();
    for(; i != points_.end(); i++) {
      if(i->cookie == c) {
        points_.erase(i);
        break;	
      }
    }
  }

protected:
  std::list<connection> points_;
};


cookie_t new_cookie() {
  static cookie_t cookie_count = 1;
  return cookie_count++;
}







// test service proxy
/*
template<class T>
struct proxy_test_service : public connection_point<T>
{
public:
  void notify_test(int a, int b) {
    T* this_ = static_cast<T *>(this);
    std::list<rpc::connection>::const_iterator 
    c = points_.begin();
    while(c != points_.end()) {
      RefPtr<rpc::idispatch> pConnection = c->dispatch;
      if (pConnection)
      {
        rpc::protocol::event::parameters params;  
        params["a"] = a;
        params["b"] = b;
        // send to queue
        pConnection->invoke(1, params, c->cookie);
      }

      c++;
    }
  }
};
*/

} // namespace rpc

#endif // connection_point_h__

