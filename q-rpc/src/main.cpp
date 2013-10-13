
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


class io_thread : public q::Thread {
public:
  io_thread(boost::asio::io_service& io) 
  : io_service_(io) 
  , work_(io)
  {

  }

protected:
  bool loop() 
  {
    io_service_.run();
    return false;
  }

private:
  boost::asio::io_service& io_service_;
  boost::asio::io_service::work work_; 
};  // io_thread

class caller {
public:
  caller(rpc::base_stream& s) : stream_(s) {

  }


public:
  void on_call_request() {

  }

public:
  void onstart() {};
  void onread() {}
  void onwrite() {}
  void onstop() {};



private:
  rpc::stream& stream_;

};

class callee {
public:
  callee(rpc::base_stream& s) : stream_(s) {

  }

public:
  void onstart() {};
  void onread() {}
  void onwrite() {}
  void onstop() {};

private:
  rpc::stream& stream_;
};


template<class T>
class acceptor {
public:
  acceptor()
  : io_acceptor_(0)
  , io_thread_(0)
  {
    // Start an accept operation for a new connection.
  }
  
  virtual ~server_base() {
    stop();
  }

  bool listen(unsigned short port) {
    assert(NULL == io_acceptor_);
    if(NULL == io_acceptor_) 
      io_acceptor_ = new boost::asio::ip::tcp::acceptor(io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

    RefPtr<rpc::stream_boost_impl> new_connection(new rpc::stream_boost_impl(acceptor_->get_io_service()));
    io_acceptor_->async_accept(new_connection->socket(),
        boost::bind(&server_base::handle_accept, this,
        boost::asio::placeholders::error, new_connection));
       
    io_thread_ = new rpc::io_thread(io_service_);
    io_thread_->start(); 
    
    return true;
  }

  /// Handle completion of a accept operation.
  void handle_accept(const boost::system::error_code& e, RefPtr<rpc::stream_boost_impl> conn)
  {
    if (!e) {
      try {  
        if (conn != NULL) {  
          boost::format fmt("%1%:%2%");  
          fmt % conn->socket().remote_endpoint().address().to_string();  
          fmt % conn->socket().remote_endpoint().port();  
          //conn->set_remote_addr(fmt.str());  
          OnAccept(conn);	
        }  
      } catch(std::exception& e) {  
        LOG4CXX_ERROR(Wayixia_Logger, "with exception:[" << e.what() << "]");  
      } catch(...)  {  
        LOG4CXX_ERROR(Wayixia_Logger, "unknown exception.");  
      }  
    }   

    // Start an accept operation for a new connection.
    RefPtr<rpc::stream_boost_impl> new_conn(new rpc::stream_boost_impl(acceptor_->get_io_service()));
    io_acceptor_->async_accept(new_conn->socket(),
 	    boost::bind(&acceptor<T>::handle_accept, this,
	    boost::asio::placeholders::error, new_conn));
  }


  void stop() {
    if(acceptor_)
      delete io_acceptor_;
    
    io_acceptor_ = 0;

    if(io_thread_ &&io_thread_->running()) 
      io_thread_->stop();
    
    delete io_thread_;   
    io_thread_ = 0;
  }

  virtual bool OnAccept(RefPtr<rpc::base_stream> conn) {
    conn->handler()->on_connect();
    return true;
  }

private:
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor* io_acceptor_;
  io_thread* io_thread_;	
}; // class acceptor




template<class T>
class connector : public q::Object 
{
public:
  connector(); 
  virtual ~connector() {}

public:
  void connect(const std::string& host,  const std::string& service, int timeout = -1) {
    rpc::stream_boost_impl* p = new rpc::stream_boost_impl(io_service_);

    // Resolve the host name into an IP address.
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::resolver::query query(host, service);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    // Start an asynchronous connect operation.
    boost::asio::async_connect(
	  p->socket(), 
	  endpoint_iterator,
      boost::bind(&connector<T>::handle_connect, this,
      boost::asio::placeholders::error)
	);

    // save
    stream_ = p;
	hanlder_ = new T(*p);
    stream_->handler(handler_);

    std::cerr << "client_base::connect" << std::endl;
    if(!io_thread_.running())
      io_thread_.start();

    std::cerr << "client_base::connect 1" << std::endl;
  
    // wait for connect
    int r = event_timedwait(event_connect_ok_, timeout);
    if(r != 0) {
      throw r;
    }
    std::cerr << "client_base::connect 2" << std::endl;
  }
  
public:
  RefPtr<rpc::base_stream> stream() { return stream_; }

private:
  /// Handle completion of a connect operation.
  void handle_connect(const boost::system::error_code& e){
    if (!e) {
      stream_->handler()->on_connect();
    } else {
      std::cerr << e.message() << std::endl;
      stream_->handler()->on_disconnect();
    }
    event_set(event_connect_ok_);
  }

protected:
  boost::asio::io_service io_service_;
  io_thread io_thread_;
  RefPtr<rpc::base_stream> stream_;
  event_handle event_connect_ok_;
}; // class connector



}; //class connector

} // namespace rpc


int main(int argc, char *argv[])
{
  // run as callee called by caller
  rpc::acceptor< rpc::callee > acceptor_;
  rpc::connector< rpc::caller > connector_;

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

