
#include "thread.h"

#ifndef WIN32
#include <pthread.h>
#include <time.h>
#else
#include <process.h>
#endif

namespace q {

Thread::Thread() 
: thread_(0)
, bRunning_(false)
, bWaitStop_(false)
{

}

Thread::~Thread(){}

bool Thread::start(){
	if(!bRunning_) {
#ifdef OS_WIN
		thread_ = (HANDLE)_beginthreadex( 0, 0, &threadfunc, this, 0, (unsigned*)&thread_id_ );
		if (thread_ == 0) return false;
#else
		if (0 != pthread_create(&thread_, NULL, &threadfunc, this)) {
			thread_ = 0;
			return false;
		}
#endif
	}	
	return bRunning_;
}

bool Thread::timedwait_stop(unsigned long timeout) {
	if(bRunning_) {
		bWaitStop_ = true;
#ifdef OS_WIN
		return WaitForSingleObject(thread_,timeout) == WAIT_OBJECT_0;
#else
		pthread_join(thread_, NULL);
#endif
	}
	return true;
}

bool Thread::stop() {
	if(bRunning_) {
		timedwait_stop();
#ifdef OS_WIN
		CloseHandle(thread_);
#endif
		thread_ = 0;
		bRunning_ = false;
	}
	return true;
}

#ifdef OS_WIN
unsigned __stdcall 
#else
void* 
#endif

Thread::threadfunc( void* pArguments ) {
	Thread* this_ = (Thread*)pArguments;
	this_->bRunning_ = true;
	this_->bWaitStop_ = false;
	unsigned exitCode = 0;
	this_->OnBegin();
	while(!this_->bWaitStop_){
		if(!this_->loop()){
			exitCode = 0;
			break;
		}
	}
	this_->OnEnd();
#ifdef OS_WIN
	return exitCode;
#else
	return 0;
#endif

} 

void Thread::OnBegin()
{

}

void Thread::OnEnd()
{

}

bool Thread::running() const
{
	return bRunning_;
}
} // namespace q