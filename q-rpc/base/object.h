
#ifndef object_h__
#define object_h__

#include <ref_counted.h>

namespace q {

class Object : public base::RefCountedThreadSafe<Object> {
public:
  Object() {}
  virtual ~Object() {}
};

} // namespace q

#endif // object_h__

