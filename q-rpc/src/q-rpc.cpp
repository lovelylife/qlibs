
#include <iostream>
#include <exception>
#include <list>
#include <map>
#include <string>
#include <json/json.h>
#include <sstream>
#include <assert.h>

// common libraries
//#include <reference/IReference.h>
//#include <phstream/NetLibDef.h>
//#include <phstream/StreamDecorator.h>
//#include <memalloc/mallocins.h>
#include <Object.h>
#include <Locker.h>

#include "rpcexception.h"
#include "rpcprotocol.h"
#include "dispatch.h"
#include "connection_point.h"
#include "services.h"
//#include "test_service.h"


#include "rpcclient.h"
#include "rpcserver.h"

#include "text_archive.h"


struct test_struct {
  int a;
  long b;

  std::string s;

  template<typename Archive>
  void serialize(Archive& ar) {
    ar & a;
    ar & b;
    ar & s;
  }
};


//#include "rpcserver_default.h"
//#include "rpcclient_default.h"

class test_event : public rpc::ievent {
public:
  virtual void notify(int id, const rpc::parameters& params)
  {
    if(id == 1) {
      std::cerr << "a : " << params["a"].asInt() << "; b : " << params["b"].asInt() << std::endl;   
    }
  } 
};

int main(int argc, char *argv[])
{
  
  std::string s = "abcde";
  s.append(1, '\0');
  std::cout << s.size() << std::endl;   
  

  struct test_struct t;
  t.a = 3;
  t.b = 5;
  t.s = "agcdefg";

  std::string o;
  text_oarchiver oar(o);
  oar << t;

  struct test_struct t2;
  text_iarchiver iar(o);
  iar >> t2;

  std::cout << "t2.a=" << t2.a
	    << "\nt2.b=" << t2.b
	    << "\nt2.s=" << t2.s << std::endl;


  
  //int a;
  //void* ptr = (void*)&a;
  //std::stringstream strm;
  //strm << ptr;
  //std::string str = strm.str(); 
  //std::cout << "ptr string: " << str << std::endl;
  try
  {
    if(argc > 1) {
      //rpc::server_default> server;
      //if(!server.start("127.0.0.1", 55555)) {
        std::cerr << "start server error" << std::endl;
      //} else {
      //  std::cerr << "start server ok" << std::endl;
	   // register service
	//    server.register_service(new rpc::services::test_service);
        std::string action;
        while(std::getline(std::cin, action)) {
          if(action == "exit") break;
	}
      //}
    } else {
      // client
      rpc::client rpc_client;
      //rpc_client.initialize("127.0.0.1", 10000);      
      //rpc_client.connect("127.0.0.1", 55555);
      rpc::client::iservice_proxy* test_service = rpc_client.get_service("test_service");

      rpc::cookie_t cookie;
      //rpc_client.addListener(test_service, new CReference_T<test_event>, cookie);
      std::cerr << "get cookie : " << cookie << std::endl;	
	
      // call test_service add method
      // 
      rpc::protocol::response res;
      rpc::protocol::request::parameters params;
      params["p0"] = 20;
      params["p1"] = 35;	
      // rpc_client.call(test_service, "add", params, res);

      // debug
      std::string action;
      while(std::getline(std::cin, action)) {
        if(action == "exit") {
          break;
        }

        rpc_client.call(test_service, action, params, res);
        res.dump();
      }
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

