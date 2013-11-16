
#ifndef protocol_h_
#define protocol_h_

/*
Message {
　　type: request|response|event,
	channel: 1|2|...		// 对象通道
　　body : RequestMessage|ResponseMessage|EventMessage
　}

RequestMessage结构，客户端发送到服务端,
{
　　action: action_id            // 操作id
　　parameters: list             // 参数列表
}
　　
ResponseMessage结构,  response 为服务端应答给客户端
{
　　action: action_id,           // 操作id
　　result: resultdata           // 应答数据
}

EventMessage结构，   事件操作
{
　　action: add|remove|notify,   // 添加消息监听，删除消息监听, 事件通知
　　id:    event_id,             // 事件id
　　parameters: list,            // 参数列表
}


*/
//#include "stdint.h"
#include "text_archive.h"
#include <json/json.h>


namespace rpc {
  typedef Json::Value parameters;
  typedef unsigned int cookie_t;
  typedef std::string connection_t;
}


namespace rpc { namespace protocol {


static const int message_request  = 0;
static const int message_response = 1;
static const int message_request_void = 2;
static const int message_event    = 3;


static const int event_add	  = 0;
static const int event_remove	  = 1;
static const int event_notify	  = 2;


struct package_header
{
  unsigned int size;			// 数据包大小
};

typedef struct message__ {
  int type;
  int channel;
  std::string body;
  template <typename Archive>
  void serialize(Archive& ar)
  {
    ar & type;
    ar & channel;
    ar & body;
  }
} message;



class serializer
{
public:
  typedef Json::Value body_type;

public:
  serializer() : body_() {}
  ~serializer() {}

public:
  body_type& body() { return body_; }

public:
  bool from_string(const std::string& in_string) {
    Json::Reader reader;
    if(!reader.parse(in_string, body_)) {
      return false;
    }

    return true;
  }

  void to_string(std::string& out_string) {
    Json::StyledWriter writer;
    out_string = writer.write(body_);
  }

protected:
  body_type body_;
};

class request : public serializer {
public:
  typedef Json::Value parameters;

public:
  request() {}
  ~request() {}

  int action() { return body_["action"].asInt(); }

  parameters& params() { return body_["parameters"]; }

  void set_action(int func_id) {
    body_["action"] = func_id;
  }

  void set_params(const parameters& p) {
    body_["parameters"] = p;
  }

public:
  void dump() {
    std::string s;
    to_string(s);
    std::cout << "request:\n" << s << std::endl;
  }
};

class response  : public serializer 
{
public:
  response() {}
  ~response(){}
  void put_result_code(int code) {
    body_["result_code"] = code;
  }

  int  get_result_code() {
    return body_["result_code"].asInt();
  }

  void dump() {
    std::string s;
    to_string(s);
    std::cout << "response:\n" << s << std::endl;
  }
};

class event : public serializer
{
public:
  typedef Json::Value parameters;

public:
  event() {}
  ~event() {}
  void set_action(int func_id) {body_["action"] = func_id;}
  int  action() { return body_["action"].asInt(); }
  void dump() {
    std::string s;
    to_string(s);
    std::cout << "event:\n" << s << std::endl;
  }
};


} } // namespace protocol } namespace rpc


#endif // protocol_h_

