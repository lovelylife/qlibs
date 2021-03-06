
import os

#env = Environment(TARGET_ARCH = 'x86')
env = Environment()
 
base_dir = '../base'
jsoncpp_dir = '../thanks/jsoncpp'
boost_dir = os.environ.get('BOOST_HOME')

libpath = [
  boost_dir + '/stage/lib'
]

includes = [
'-I../include',
'-I'+jsoncpp_dir+'/include',
'-I'+base_dir,
'-I'+boost_dir,
#'-g',
'-DNDEBUG',
]


libs_ = ['rt', 'boost_thread-gcc43-mt-1_49','boost_system-gcc43-mt-1_49','pthread']
#libs_ = ['rt', 'boost_thread','boost_system','pthread']

#sources
sources = Glob('../src/*.cpp')
sources.append(jsoncpp_dir+'/src/json/json_value.cpp')
sources.append(jsoncpp_dir+'/src/json/json_reader.cpp')
sources.append(jsoncpp_dir+'/src/json/json_writer.cpp')

#base dir cpp
sources.append(base_dir+'/thread.cc')
sources.append(base_dir+'/ref_counted.cc')
sources.append(base_dir+'/atomicops_internals_x86_gcc.cc')

env.Program('qrpc', 
  source=sources, 
  LINKFLAGS=['-static'],
  LIBS=libs_,
  LIBPATH=libpath,
  CCFLAGS=includes)
