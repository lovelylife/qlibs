
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
#include <object.h>
#include <locker.h>

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

namespace rpc {

class caller {
public:
  rpc::stream stream_;

public:
  void on_call_request() {

  }

public:
  void onstart() {};
  void onread() {}
  void onwrite() {}
  void onstop() {};
};

class callee {
  void onstart() {};
  void onread() {}
  void onwrite() {}
  void onstop() {};

};

template<class T>
class transport : public q::Object
{
public:
  transport() : object_(new T) {

  }

public:
  void onconnect() {
    object_->onstart();
  };
  void ondisconnect() {
    object_->onstop();
  };
  void onread() {
    object_->onread();
  }
  void onwrite() {
    object_->onwrite();
  }

private:
  RefPtr<T> object_;
};


template<class T>
class acceptor
{

}; // class acceptor

template<class T>




}; //class connector

} // namespace rpc


int main(int argc, char *argv[])
{
  // run as callee called by caller
  rpc::acceptor< rpc::transport <rpc::callee> > acceptor_;
  rpc::connector< rpc::transport <rpc::caller> > connector_;

  // run as caller which call the callee
  rpc::acceptor<rpc::caller> acceptor2_;
  rpc::connector<rpc::callee> connector2_;


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

