
#ifndef rpcexception_h__
#define rpcexception_h__

#include <exception>

namespace rpc {

class exception : public std::exception
{
public:
  exception(const std::string& msg) 
  : message_(msg)
  {

  }
  
  virtual ~exception() throw() {}

// std::exception
public:
  const char* what() const throw() {
    return message_.c_str();
  }

private:
  std::string message_;


};

} // namespace rpc


#endif //rpcexception_h__

