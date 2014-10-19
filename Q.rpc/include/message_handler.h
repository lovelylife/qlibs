
#ifndef message_handler_h__
#define message_handler_h__

namespace rpc {

class message_handler : public q::Object
{
public:
  typedef enum { connect = 0, write   = 1,  read    = 2,  byebye  = 3, } stream_step;

  message_handler() {} 
  virtual ~message_handler() {}

public:
  virtual void on_read_completed(const std::string& buffer) = 0;
  virtual void on_write_completed(const std::string& buffer) = 0;  
  virtual void on_connect() = 0; // ���ӽ���
  virtual void on_disconnect(stream_step step, const std::string& err_msg) = 0;  // ���ӶϿ�
};



} // namespace rpc

#endif // message_handler_h__

