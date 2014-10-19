
//////////////////////////////////////////////////////////////////////////
// class Thread
// 线程基本操作的封装
//////////////////////////////////////////////////////////////////////////

#ifndef ThreadPool_h__
#define ThreadPool_h__

#include "../base/builder_config.h"
#include "TypeDefs.h"
#include "Object.h"
#include "Thread.h"
#include <vector>
#include "RefCounted.h"
#include "SemaphoreQueue.h"

namespace q {

struct ITask : Object {
	virtual bool Task() = 0;
	virtual void OnFinish() = 0;
};

class ThreadPool;

class WorkThread 
	: public Thread 
{
public:
	WorkThread(ThreadPool* pool);
	~WorkThread();

protected:
	bool loop();

private:
	ThreadPool* pool_;
};

class ThreadPool {
public:
	bool run();
	void stop();
	bool newtask(ITask* task, int time = -1, int times = 0);

public:
	ThreadPool(int num);
	~ThreadPool();
	friend class WorkThread;

private:
	bool work_proc();

private:
	int threads_num;
	std::vector<WorkThread*> threads;
	SemaphoreQueue< RefPtr<ITask> > task_queue_;	
};

class ThreadPoolManager {
public:
	bool run(int thread_num);
	void stop();
	bool newtask(ITask* task, int time = -1, int times = 0);

public:
	ThreadPoolManager();
	ThreadPoolManager(int thread_num);
	~ThreadPoolManager();

private:
	ThreadPool* thread_pool_;
	int number_of_thread;
};

} // namespace q

#endif
