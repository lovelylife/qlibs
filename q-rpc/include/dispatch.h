

#ifndef dispatch_h__
#define dispatch_h__


namespace rpc {

// 分发接口
struct idispatch : q::Object {
  virtual void invoke(int id, const rpc::parameters& params, int cookie ) = 0;
};

}

#endif

