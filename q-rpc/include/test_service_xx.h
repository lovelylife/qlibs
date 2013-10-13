
#ifndef test_service_h_
#define test_service_h_



namespace rpc {


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

}

namespace rpc { namespace services {


class test_service 
: public rpc::services::iservice
, public rpc::proxy_test_service<test_service> 
{
// connection implimention
connection_impl(test_service)

// ½Ó¿Ú
public:
  int add(int a, int b);


// iservice
public:
  const char* name(); // { return "test_service"; }
  void register_functions(std::map<int, std::string>&);
  void process(rpc::protocol::request& req, rpc::protocol::response& res, rpc::session_ptr context);
}; 


} } // namespace services } namespace rpc

#endif // test_service_h_

