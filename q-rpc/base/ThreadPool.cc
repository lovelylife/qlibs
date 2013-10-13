

#include "ThreadPool.h"

namespace q {

WorkThread::WorkThread( ThreadPool* pool )
: pool_(pool)
{

}

WorkThread::~WorkThread() {}

bool WorkThread::loop() { 
	if(NULL != pool_) {
		return pool_->work_proc();
	}

	return false;
}


////////////////////////////////////////////////////////////
// class ThreadPool
////////////////////////////////////////////////////////////

ThreadPool::ThreadPool( int num ) : threads_num(num)
{

}

ThreadPool::~ThreadPool()
{

}

bool ThreadPool::run()
{
	task_queue_.open();
	for(int i=0; i < threads_num; i++) {
		WorkThread* p = new WorkThread(this);
		threads.push_back(p);
		p->start();
	}
	return true;
}

void ThreadPool::stop()
{
	task_queue_.close();
	for(size_t i=0; i < threads.size(); i++) {
		WorkThread* p = threads[i];
		if(p) {
			p->stop();
		}
	}
	
}

bool ThreadPool::newtask( ITask* task, int time /*= -1*/, int times /*= 0*/ )
{
	return task_queue_.push(task);
}

bool ThreadPool::work_proc()
{
	RefPtr<ITask> pTask;
	while(task_queue_.pop(pTask)) {
		pTask->Task();
		pTask->OnFinish();
	}
	return true;
}


////////////////////////////////////////////////////////////
// class ThreadPoolManager
////////////////////////////////////////////////////////////





ThreadPoolManager::ThreadPoolManager() {
}

ThreadPoolManager::~ThreadPoolManager()
{
	stop();
	if(NULL != thread_pool_) {
		delete thread_pool_;
	}
}

bool ThreadPoolManager::run(int thread_num)
{
	if(thread_num <= 0) {
		thread_num = 5;
	}

	thread_pool_ = new ThreadPool(thread_num);

	return thread_pool_->run();
}

bool ThreadPoolManager::newtask( ITask* task, int time /*= -1*/, int times /*= 0*/ )
{
	//assert(NULL != thread_pool_);
	return thread_pool_->newtask(task, time, times);
}

void ThreadPoolManager::stop()
{
	thread_pool_->stop();
}



} // namespace q