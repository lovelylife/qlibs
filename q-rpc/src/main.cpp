
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
//#include <reference/IReference.h>
//#include <phstream/NetLibDef.h>
//#include <phstream/StreamDecorator.h>
//#include <memalloc/mallocins.h>
#include <object.h>
#include <locker.h>
#include <thread.h>


#include "rpcexception.h"
#include "rpcprotocol.h"
#include "event.h"
#include "message_handler.h"
#include "stream.h"
#include "stream_boost_impl.h"
#include "dispatch.h"
#include "connection_point.h"
#include "services.h"
#include "test_service.h"


#include "rpcclient.h"
#include "rpcserver.h"

#include "text_archive.h"


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

class caller : public rpc::message_handler, public rpc::client  {
public:
  caller(rpc::base_stream* s) 
  : stream_(s) 
  , init_rpc_interface_(false) 
  , read_ok_(0)
  , write_ok_(0)
  {
      read_ok_ = event_create(false, false);
      write_ok_ = event_create(false, false);
  }

// interface rpc::message_handler
public:
  virtual void on_connect() {
      std::cerr << "callee_handler connected" << std::endl;
      stream_->async_read();
  }

  // Á´½Ó¶Ï¿ª
  virtual void on_disconnect() { 
    std::cerr << "callee_handler disconnected not impliement" << std::endl;
  }

  virtual void on_read_completed(const std::string& buffer) {
    //std::cerr << "callee_handler read_completed." << std::endl;
    
    if(!init_rpc_interface_) {
      // intialize interface
      init_rpc_interface_ = true;      
      std::cerr << "intialize rpc interface " << buffer << std::endl;
      handler_services_proxy(buffer);        
    } else {
      text_iarchiver ar(buffer);
      ar >> message_;
      event_set(read_ok_);
    }
  }

  virtual void on_write_completed(const std::string& buffer) {  
    //std::cerr << "callee_handler write_completed." << std::endl;  
    event_set(write_ok_);
  }

// rpc::client interface
public:
  virtual void send_request(rpc::protocol::message& msg, int timeout=-1) 
  {
    event_reset(write_ok_);
    std::string o;
    text_oarchiver archive(o);
    archive << msg;

    stream_->async_write(o);
    int result = event_timedwait(write_ok_, timeout);
    if(result)
      throw rpc::exception("write timeout");
  }

  virtual void recv_response(rpc::protocol::message& msg, int timeout=-1) 
  {
    event_reset(read_ok_);
    stream_->async_read();
    int result = event_timedwait(read_ok_, timeout);
    if(result)
      throw rpc::exception("read timeout");

    msg = message_;
  }

private:
  RefPtr<rpc::base_stream> stream_;
  bool init_rpc_interface_;
  event_handle read_ok_;
  event_handle write_ok_;
  rpc::protocol::message message_;
};

class callee : public rpc::message_handler {
public:
  callee(rpc::base_stream* s, rpc::server& r) 
  : stream_(s) 
  , rpcserver_(r)
  {
    
  }

// interface rpc::message_handler
public:
  void on_connect() {
    std::cerr << "callee_server:: callee_handler::on_connect()" << std::endl;
    std::string s;
    //@todo
    rpcserver_.dump_services_info(s);
    std::cerr << "export interface to caller:" << s << std::endl;
    stream_->async_write(s);
  }
    
  void on_disconnect() {
    std::cerr << "callee_server:: callee_handler::on_disconnect()" << std::endl;
  }

  void on_read_completed(const std::string& buf) {
    //std::cerr << "callee_server:: callee_handler::on_read_completed()" << std::endl;
    rpc::protocol::message msg;
    text_iarchiver ar(buf);
    // std::cerr << buf << std::endl;
    ar >> msg;
    rpc::protocol::message res;
    //@todo
    rpcserver_.handle_message(msg, res, NULL);

    std::string o;
    text_oarchiver oar(o);
    oar << res;
    stream_->async_write(o);
  }

  void on_write_completed(const std::string& buf) {
    //std::cerr << "callee_server:: callee_handler::on_write_completed()" << std::endl;
    stream_->async_read();
  }

private:
  RefPtr<rpc::base_stream> stream_;
  rpc::server& rpcserver_;
};


class acceptor {
public:
  acceptor()
  : io_acceptor_(0)
  , io_thread_(0)
  {
    // Start an accept operation for a new connection.
  }
  
  virtual ~acceptor() {
    stop();
  }

  bool listen(unsigned short port) {
    assert(NULL == io_acceptor_);
    if(NULL == io_acceptor_) 
      io_acceptor_ = new boost::asio::ip::tcp::acceptor(io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

    RefPtr<rpc::stream_boost_impl> new_connection(new rpc::stream_boost_impl(io_acceptor_->get_io_service()));
    io_acceptor_->async_accept(new_connection->socket(),
        boost::bind(&acceptor::handle_accept, this,
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
        // create handler
        if (conn != NULL) {  
          boost::format fmt("%1%:%2%");  
          fmt % conn->socket().remote_endpoint().address().to_string();  
          fmt % conn->socket().remote_endpoint().port();  
          //conn->set_remote_addr(fmt.str());  
          OnAccept(conn);	
        }  
      } catch(std::exception& e) {  
        std::cerr << "with exception:[" << e.what() << "]" << std::endl;  
      } catch(...)  {  
        std::cerr <<  "unknown exception." << std::endl;  
      }  
    }   

    // Start an accept operation for a new connection.
    RefPtr<rpc::stream_boost_impl> new_conn(new rpc::stream_boost_impl(io_acceptor_->get_io_service()));
    io_acceptor_->async_accept(new_conn->socket(),
 	    boost::bind(&acceptor::handle_accept, this,
	    boost::asio::placeholders::error, new_conn));
  }


  void stop() {
    if(io_acceptor_)
      delete io_acceptor_;
    
    io_acceptor_ = 0;

    if(io_thread_ &&io_thread_->running()) 
      io_thread_->stop();
    
    delete io_thread_;   
    io_thread_ = 0;
  }

  virtual bool OnAccept(RefPtr<rpc::base_stream> conn) {
    if(conn->handler())
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
  connector() 
  : io_service_()
  , io_thread_(io_service_)
  {
    event_connect_ok_ = event_create(false, false);
  };
 
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
    handler_ = new T(p);
    p->handler(handler_);

    std::cerr << "client_base::connect" << std::endl;
    if(!io_thread_.running())
      io_thread_.start();

    std::cerr << "client_base::connect 1" << std::endl;
  
    // wait for connect
    int r = event_timedwait(event_connect_ok_, timeout);
    if(1 == r) {
      std::cerr << "connect timout" << std::endl;
      throw r;
    } else if(-1 == r){
      std::cerr << "connect error" << std::endl;
    }
    std::cerr << "client_base::connect 2" << std::endl;
  }

private:
  /// Handle completion of a connect operation.
  void handle_connect(const boost::system::error_code& e){
    if (!e) {
      if(handler_)
        handler_->on_connect();
    } else {
      std::cerr << e.message() << std::endl;
      if(handler_)
         handler_->on_disconnect();
    }
    event_set(event_connect_ok_);
  }

protected:
  boost::asio::io_service io_service_;
  io_thread io_thread_;
  event_handle event_connect_ok_;
  RefPtr<T> handler_;
}; //class connector



class rpcserver_default : public acceptor 
{
public:
  // register rpc service
  void register_service(rpc::services::iservice* s) {
    rpcserver_.register_service(s);
  }

// acceptor overrides
public:
  virtual bool OnAccept(RefPtr<rpc::base_stream> conn) {
    rpc::callee* handler = new rpc::callee(conn, rpcserver_);
    conn->handler(handler);
    static int i= 0;
    clients_[++i] = handler;
    return acceptor::OnAccept(conn);
  }

private:
  std::map<int, RefPtr<rpc::callee> > clients_;
  rpc::server rpcserver_;
};


class rpcclient_default : public connector<caller> 
{
public:
  void call(const std::string& service, 
            const std::string& method, 
            const rpc::parameters& params,
            rpc::protocol::response& res)
  {
    if(NULL == handler_) {
      throw rpc::exception("invalid rpc caller.");
    }

    rpc::client::iservice_proxy* s = handler_->get_service(service);
    if(NULL == s) 
      throw rpc::exception("have no service");

    handler_->call(s, method, params, res); 
  }
};

class rpcnotify_client_default : public connector<callee> {


};


} // namespace rpc



int main(int argc, char *argv[])
{
  // run as callee called by caller
  //rpc::acceptor< rpc::callee > acceptor_;
  //rpc::connector< rpc::caller > connector_;

  // run as caller which call the callee
  //rpc::acceptor<rpc::caller> acceptor2_;
  //rpc::connector<rpc::callee> connector2_;


  try
  {
    if(argc > 1) {
      rpc::rpcserver_default server;
      server.register_service(new rpc::services::test_service);
      if(!server.listen(5555)) {
        std::cerr << "start server error" << std::endl;
      } else {
        std::cerr << "start server ok" << std::endl;
        // register service
	// server.register_service(new rpc::services::test_service);
        std::string action;
        while(std::getline(std::cin, action)) {
          if(action == "exit") break;
	}
      }
    } else {
      // client
      rpc::rpcclient_default rpc_client;
      //rpc_client.initialize("127.0.0.1", 10000);      
      rpc_client.connect("127.0.0.1", "5555");
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

