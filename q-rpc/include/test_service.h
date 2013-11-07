
#ifndef test_service_h_
#define test_service_h_


namespace rpc { namespace services {

class test_service 
: public rpc::services::iservice
{
public:
  test_service(rpc::qnotify_server* ns) : ns_(ns) {}
  ~test_service() {}


// rpc interfaces
rpc_methods_begin(test_service)
rpc_method(1, add)
rpc_method(2, test)
rpc_methods_end()

// ½Ó¿Ú
public:
  void add(rpc::protocol::request& req, rpc::protocol::response& res, rpc::server_session_ptr) {
    res.body()["data"] = req.params()["p0"].asInt() + req.params()["p1"].asInt();
  }

  void test(rpc::protocol::request&, rpc::protocol::response&, rpc::server_session_ptr) {
    
  }
private:
  rpc::qnotify_server* ns_;
}; 


} } // namespace services } namespace rpc

#endif // test_service_h_


