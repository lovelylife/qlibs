
#ifndef stream_h__
#define stream_h__

#include <object.h>

namespace rpc {

	
struct base_stream : virtual public q::Object 
{
  // read data
  virtual void async_read()  = 0;
  // write data
  virtual void async_write(const std::string& buffer) = 0;
  // get or set handler
  virtual rpc::message_handler* handler(rpc::message_handler* p=0) = 0;

}; // class base_stream


} // namespace rpc


#endif // stream_h__

