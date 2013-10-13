
#ifndef message_handler_h__
#define message_handler_h__

namespace rpc {

class message_handler : public q::Object
{
public:
  message_handler() 
  {
 
  }
 
  virtual ~message_handler() {}

public:
  virtual void on_read_completed(const std::string& buffer) = 0;
  virtual void on_write_completed(const std::string& buffer) = 0;
  // ���ӽ���
  virtual void on_connect() = 0; // {}
  // ���ӶϿ�
  virtual void on_disconnect() = 0;  // { std::cerr << "message_handler disconnected not impliement" << std::endl;}
  // ���ݷ���ֵȷ���Ƿ������ȡ��һ����Ϣ
  //virtual bool on_message(const std::string& buf) = 0; //{ return false; } 

};



} // namespace rpc

#endif // message_handler_h__

