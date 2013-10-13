
//////////////////////////////////////////////////////////////////////////
// class Thread
// 线程基本操作的封装
//////////////////////////////////////////////////////////////////////////

#ifndef Thread_h__
#define Thread_h__

#include "../base/builder_config.h"
#include "TypeDefs.h"

# ifndef OS_WIN
#	include <pthread.h>
#	include <time.h>
# else
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <process.h>
# endif

namespace q {

class Thread {
// 共用方法
public:
	virtual bool start();
	virtual bool stop();

// 属性
public:
	bool running() const;

// 标准构造和析构函数
public:
	Thread();
	~Thread();

// 线程句柄
public:
#  ifdef OS_WIN
	typedef HANDLE pthread_t;
	DWORD thread_id_;
#  endif
	pthread_t thread_;

// 保护方法
protected:
#  ifdef OS_WIN
	static unsigned __stdcall threadfunc( void* pArguments );
#  else
	static void* threadfunc( void* pArguments );
#  endif

	virtual bool loop() = 0;

	// 等待线程停止
	// 调用后将设置为等待停止状态,并阻塞调用线程,直到线程停止
	virtual bool timedwait_stop(unsigned long timeout = -1); 

	// 线程开始时被调用
	// 在线程空间内执行,线程开始时被调用
	virtual void OnBegin();

	// 线程停止时被调用
	// 在线程空间内执行,线程停止时被调用
	virtual void OnEnd();

	// 等待停止
	volatile bool bWaitStop_;
	// 是否在运行
	volatile bool bRunning_;

private:
	DISALLOW_COPY_AND_ASSIGN(Thread);

}; // class Thread

} // namespace base

#endif
