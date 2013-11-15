
#ifndef rpcserver_h_
#define rpcserver_h_

namespace rpc {

class server { 
public:
  server() {}
  ~server(){}

// initialize service
public:
  void register_service(rpc::services::iservice* s) {
    int channel_id = services_.size();
    services_[channel_id] = s;
  }

  // 处理客户端消息请求(input_msg)，将返回处理结果(output_msg)
  void handle_message(rpc::protocol::message& input_msg, rpc::protocol::message& output_msg, rpc::server_session_ptr s)
  {
    output_msg.channel = input_msg.channel;
    int message_type = input_msg.type;
    switch(input_msg.type) {
    case rpc::protocol::message_request:
      handle_request_message(input_msg, output_msg, s);
      break;		

    case rpc::protocol::message_event:
      handle_event_message(input_msg, output_msg, s);
      break;
    
    default: 
      {
        rpc::protocol::response res;
        res.put_result_code(-2);
        res.body()["message"] = "un acceptable message";
        res.to_string(output_msg.body);
      }
    }
  }


private:
  // process call request 
  void handle_request_message(rpc::protocol::message& input_msg, rpc::protocol::message& output_msg, rpc::server_session_ptr s) 
  {
    output_msg.channel = input_msg.channel;
    output_msg.type = rpc::protocol::message_response;
    rpc::protocol::request req;
    rpc::protocol::response res;
    // serialize from request
    req.from_string(input_msg.body);
    //req.dump();
    try {
      if(input_msg.channel < 0) {
        // server message
      } else {
        // get service object
        rpc::services::iservice* is = get_service(input_msg.channel);
        if(!is) { // error occurred
          std::ostringstream oss;
          oss << "unknow service id:" << input_msg.channel;
          throw rpc::exception(oss.str().c_str());						
        }
			
        // service process 
        is->process(req, res, s);	
      }
    }
    catch (const std::exception& e) {
      res.put_result_code(-1);
      res.body()["message"] = e.what();
    }

    // serialize to response		
    res.to_string(output_msg.body);
  }

  // process addEventListener or removeEventListener request
  void handle_event_message(rpc::protocol::message& input, rpc::protocol::message& output, rpc::server_session_ptr s)
  {

  }

public:
  rpc::services::iservice* get_service(int channel) {
    std::map<int, rpc::services::iservice*>::const_iterator c = services_.find(channel);
    if(c != services_.end())
      return c->second;

    return 0;
  }


  void dump_services_info(std::string& json_data) {
    // jsoncpp version
    Json::Value tree;
    tree["action"] = "serviceinfo";
    Json::Value array(Json::arrayValue);
    std::map<int, rpc::services::iservice*>::const_iterator c = services_.begin();
    while(c != services_.end()) {
      Json::Value s(Json::objectValue);
      s["id"] = c->first;
      s["name"] = c->second->name();

      Json::Value func_array(Json::arrayValue);
      std::map<int, std::string> funcs_map;
      const rpc_method_entry* entry = c->second->methods();
      while(entry->id > 0) {
        Json::Value func_item(Json::objectValue);
        func_item["id"] = entry->id;
        func_item["name"] = entry->name;
        func_array.append(func_item);
        (entry++);
      }
      s["funcs"] =  func_array;
      array.append(s); 
      c++;
    }

    tree["services"] = array;
    Json::StyledWriter writer;
    json_data = writer.write(tree);
  }

private: 
  std::map<int, rpc::services::iservice*> services_;
};

class queue_server {
public:
  queue_server() {}
  virtual ~queue_server() {}

// accept queue_client
  void handle_accept(rpc::server_session_ptr session) {
    // serialize ptr to string
    void* ptr = (void*)session;
    std::stringstream strm;
    strm << ptr;
    std::string key = strm.str(); 

    // 绑定数据
    //p->this_ = rpc_serv_;
    //p->key_ = key;

    // 保存链接
    { 
    //  CAutoLock<CCrtSection> lock(critical_section_);
      queue_clients_[key] = session;
    }

  }


  rpc::server_session_ptr get(rpc::connection_t t) {
    //CAutoLock<CCrtSection> lock(critical_section_);
    std::map<rpc::connection_t, rpc::server_session_ptr>::const_iterator 
    c = queue_clients_.find(t);
    if(c != queue_clients_.end())
      return c->second;

    return 0;  
  }
  
private:
  std::map<std::string, rpc::server_session_ptr> queue_clients_;  
};

} // namespace rpc


#endif // protocol_h_

