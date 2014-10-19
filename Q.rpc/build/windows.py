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
'-D_WIN32_WINNT=0x0501', 
'-DWIN32',
'-D_WIN32',
'/MTd',
'/EHa',
'/Gd',
'/Od',
'/DEBUG',
]

#libraries
libs_ = []

#source files
sources = Glob('../src/*.cpp')
sources.append(jsoncpp_dir+'/src/json/json_value.cpp')
sources.append(jsoncpp_dir+'/src/json/json_reader.cpp')
sources.append(jsoncpp_dir+'/src/json/json_writer.cpp')
sources.append(base_dir+'/thread.cc')
sources.append(base_dir+'/ref_counted.cc')
sources.append(base_dir+'/atomicops_internals_x86_gcc.cc')


#build
env.Program('q-rpc', 
  source=sources, 
  LINKFLAGS=['/PDB:q-rpc.pdb'],
  LIBS=libs_,
  LIBPATH=[boost_dir+'/stage/lib'],
  CCFLAGS=includes)
