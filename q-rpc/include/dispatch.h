

#ifndef dispatch_h__
#define dispatch_h__


namespace rpc {

// �ַ��ӿ�
struct idispatch : q::Object {
  virtual void invoke(int id, const rpc::parameters& params, int cookie ) = 0;
};

}

#endif

