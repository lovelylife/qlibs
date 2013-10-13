

env = Environment(TARGET_ARCH = 'x86')

common_dir = '../../../../common'

libpath = [
'/usr/code/jsoncpp-src-0.5.0/libs/linux-gcc-4.1.2',
]

includes = [
'-I../../../../PhRemote/ext/jsoncpp/include',
'-I/usr/code/jsoncpp-src-0.5.0/include',
'-I'+common_dir,
'-D_DISABLE_OBJECT_MONITORING',
]


libs_ = ['libjson','pthread','rt']

#sources
sources = Glob('*.cpp')
sources.append(common_dir+'/phstream/BaseStream.cpp')
sources.append(common_dir+'/phstream/SockAcceptor.cpp')
sources.append(common_dir+'/phstream/SockStream.cpp')
sources.append(common_dir+'/phstream/InitSock.cpp')
sources.append(common_dir+'/thread/BaseThread.cpp')
sources.append(common_dir+'/memalloc/mallocins.cpp')



env.Program('new-simple-rpc', 
  source=sources, 
  LINKFLAGS=[],
  LIBS=libs_,
  LIBPATH=libpath,
  CCFLAGS=includes)
