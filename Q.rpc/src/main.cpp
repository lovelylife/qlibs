
#include <iostream>
#include <exception>
#include <list>
#include <map>
#include <string>
#include <json/json.h>
#include <sstream>
#include <assert.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>


// common libraries
#include <object.h>
#include <locker.h>
#include <thread.h>
#include <semaphore_queue.h>

#include "rpcexception.h"
#include "rpcprotocol.h"
#include "event.h"
#include "message_handler.h"
#include "stream.h"
#include "stream_boost_impl.h"
#include "text_archive.h"
#include "services.h"

#include "io_thread.h"
#include "rpcclient.h"
#include "rpcserver.h"
#include "caller.h"
#include "callee.h"
#include "acceptor.h"
#include "connector.h"

#include "rpcserver_default.h"
#include "rpcclient_default.h"

#include "qnotify_server.h"
#include "qnotify_client.h"

#include "test_service.h"




namespace rpc { namespace services {

class notify_dispatcher : public q::Thread {
public:
  notify_dispatcher(rpc::qnotify_server* ns):ns_(ns) {
//    queue_.open(10000);
  }

  ~notify_dispatcher() {
//    queue_.close();
  }  

public:
    void add(const rpc::protocol::request& req) {
      AutoLock<CriticalSection> lock(critical_);
      queue_.push_back(req);
      std::cerr << "[notify_dispatcher] add after size: " << queue_.size() << std::endl;
    }

public:  
    bool loop() {
      rpc::protocol::request req;
      bool is_empty = false;
      {
        AutoLock<CriticalSection> lock(critical_);
        is_empty = (queue_.size() <= 0);
        if(!is_empty) {
	  req = queue_.front();
	  queue_.pop_front();
	}
      }

      if(is_empty) {
        sleep(10);
	return true;
      }
      std::cerr << "[rpc::dispatcher]send a notify" << std::endl;
      try {
         ns_->notify(req.params()["nid"].asString(), req.params()); 
      } catch( const std::exception& e) {
         std::cerr << "notify task error " << e.what() << " \n" << req.body() << std::endl;
      }

      return true;
    }

private:
  //SemaphoreQueue<rpc::protocol::request> queue_;
  CriticalSection critical_;
  std::list<rpc::protocol::request> queue_;  
  rpc::qnotify_server* ns_;
};



class qnotify_service 
: public rpc::services::iservice
{
public:
  qnotify_service(rpc::qnotify_server* ns) : dispatcher_(ns) {
    dispatcher_.start();
  }

  ~qnotify_service() {
    dispatcher_.stop();
  }


// rpc interfaces
rpc_methods_begin(qnotify_service)
rpc_method(1, add)
rpc_method(2, call)
rpc_methods_end()

// ½Ó¿Ú
public:
  void add(rpc::protocol::request& req, rpc::protocol::response& res, rpc::server_session_ptr) {
    std::cerr << "[qnotify_service] add method called" << std::endl;
    res.body()["data"] = req.params()["p0"].asInt() + req.params()["p1"].asInt();
  }

  void call(rpc::protocol::request& req, rpc::protocol::response&, rpc::server_session_ptr) {
    //std::string nid = req.params()["nid"].asString();
    //ns_->call(nid, req.params());
    std::cerr << "[qnotify_service] recv a notify command" << std::endl;
    dispatcher_.add(req);
  }

private:
  rpc::services::notify_dispatcher dispatcher_;
}; 


} } // namespace services } namespace rpc



class test_event : public rpc::ievent {
public:
  virtual void notify(int id, const rpc::parameters& params)
  {
    if(id == 1) {
      std::cerr << "a : " << params["a"].asInt() << "; b : " << params["b"].asInt() << std::endl;   
    }
  }
};


class qnotify_event : public rpc::inotify 
{
public:
  virtual void on_initialize(const std::string& nid) {
    std::cerr << "get notify id: " << nid << std::endl;
  }

  virtual void on_notify(const rpc::parameters& params) {
    std::cerr << "qnotify_event: " << params << std::endl;
  }

  virtual void on_uninitialize(int step, const std::string& err_msg)
  {
    std::cerr << "error: " << step << " msg: " << err_msg << std::endl;
  }
};


void split(const std::string& input, const char* split_str, std::vector<std::string>& ls) {
  ls.clear();
  std::string s = input;
  char* p = (char*)s.c_str();
  p = strtok(p, split_str);
  while(p) {
    ls.push_back(p);
    p = strtok(NULL, split_str);
  }
}

void dump_vector(const std::vector<std::string>& ls) {
  std::vector<std::string>::const_iterator c = ls.begin();
  int i = 0;
  while(c!=ls.end()) {
    
    std::cerr << "[" << i << "]" << " " << *c << std::endl;
    i++;c++;
  }
}

/*
bool runas_deamon(int argc, char* argv) {
  std::string cmd = "-d";
  for(int i=1; i < argc; i++) {
    if(cmd == argv[i])
      return true;
  }

  return false;
}
*/

int main(int argc, char *argv[])
{
  try
  {
    if(argc > 1) {
      rpc::qnotify_server notify_server;
      if(!notify_server.listen(5556)) {
        std::cerr << "start notify server" << std::endl;
        return -2;
      }            

      rpc::rpcserver_default server;
      server.register_service(new rpc::services::qnotify_service(&notify_server));

      if(!server.listen(5555)) {
       std::cerr << "start rpc server error" << std::endl;
      } else {
        std::cerr << "start rpc server ok" << std::endl;
        std::string action;
        while(std::getline(std::cin, action)) {
          try {
/////////////////////////////
          //std::cerr << "input: " << action << std::endl;
          std::vector<std::string> ls;
          split(action, " ", ls);
          //dump_vector(ls); 
          if(ls.size() <= 0) continue;
          std::string f = ls[0];
          if(f == "exit") break;
          if(f == "dump") {
            notify_server.dump();
          } else if(f=="notify"){
            if(ls.size() <= 1) continue;
            std::string id = ls[1];
            rpc::parameters params;
            params["a"] = "test";
            notify_server.notify(id, params);
	  }
////////////////////////////
          } catch(const rpc::exception& e) {
            std::cerr << "error " << e.what() << std::endl;
          }
        }
        std::cerr << "exit..." << std::endl;
      }
    } else {
      // client
      rpc::rpcclient_default rpc_client;
      rpc::qnotify_client nc;
      
      nc.set_notify_handler(new qnotify_event);
      //rpc_client.initialize("127.0.0.1", 10000);      
      //rpc_client.connect("wayixia.com", "5555");
      nc.connect("wayixia.com", "5556", 10000);
      //nc.connect("127.0.0.1", "5556", 10000);
      //rpc::client::iservice_proxy* test_service = rpc_client.get_service("test_service");

      //rpc::cookie_t cookie;
      //rpc_client.addListener(test_service, new CReference_T<test_event>, cookie);
      //std::cerr << "get cookie : " << cookie << std::endl;	
	
      // call test_service add method
      // 
      rpc::protocol::response res;
      rpc::parameters params;
      params["p0"] = 20;
      params["p1"] = 35;	
      // rpc_client.call(test_service, "add", params, res);

      // debug
      std::string action;
      while(std::getline(std::cin, action)) {
        if(action == "exit") {
          break;
        }
        try {
          rpc_client.call("test_service", action, params, res);
          res.dump();
        } catch(const std::exception& e) {
          std::cerr << e.what() << std::endl;
        }
      }
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

