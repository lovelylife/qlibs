

env = Environment(TARGET_ARCH = 'x86')

base_dir = '../base'
jsoncpp_dir = '../thanks/jsoncpp'
libpath = [
'/usr/code/jsoncpp-src-0.5.0/libs/linux-gcc-4.1.2',
]

includes = [
'-I../include',
'-I'+jsoncpp_dir+'/include',
'-I'+base_dir,
'-D_DISABLE_OBJECT_MONITORING',
]


libs_ = ['pthread','rt']

#sources
sources = Glob('../src/*.cpp')
sources.append(jsoncpp_dir+'/src/json/json_value.cpp')
sources.append(jsoncpp_dir+'/src/json/json_reader.cpp')
sources.append(jsoncpp_dir+'/src/json/json_writer.cpp')



env.Program('qrpc', 
  source=sources, 
  LINKFLAGS=[],
  LIBS=libs_,
  LIBPATH=libpath,
  CCFLAGS=includes)
