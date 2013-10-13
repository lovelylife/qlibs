

env = Environment(TARGET_ARCH = 'x86')

#common directory
base_dir = '../base'

#jsoncpp src path
jsoncpp_path = '../thirdparty/jsoncpp'

#macro and include paths
includes = [
'-I../../../../PhRemote/ext/jsoncpp/include',
'-I../../../../common',
'-I'+jsoncpp_path+'/include',
'-I../base',
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
libs_ = ['libjson']

#source files
sources = Glob('*.cpp')
sources.append(base_dir+'/RefCounted.cc')
#sources.append(common_dir+'/phstream/BaseStream.cpp')
#sources.append(common_dir+'/phstream/LFTracker.cpp')
#sources.append(common_dir+'/phstream/IOCPSockStream.cpp')
#sources.append(common_dir+'/phstream/InitSock.cpp')
#sources.append(common_dir+'/thread/BaseThread.cpp')
#sources.append(common_dir+'/memalloc/mallocins.cpp')

#build
env.Program('new-simple-rpc', 
  source=sources, 
  LINKFLAGS=['/PDB:new-simple-rpc.pdb'],
  LIBS=libs_,
  LIBPATH=[],
  CCFLAGS=includes)
