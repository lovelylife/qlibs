#ifndef SemaphoreQueue_h__
#define SemaphoreQueue_h__

#include "semaphore_t.h"
#include "locker.h"
#include <list>

#ifndef WIN32
#include <time.h> 
#include <sys/time.h>
#endif

#ifndef SEM_VALUE_MAX
#define SEM_VALUE_MAX   0x3FFFFFFF
#endif

typedef enum {	OK = 0,	TIMEOUT=1,	FAILED = -1, } ResultCode;

template<class T>
class SemaphoreQueue
{
public:
	typedef typename std::list<T> QUEUE;

// 构造析构
public:
	SemaphoreQueue():m_bOpen(false),m_nSize(0) {
		//open();
	}

	~SemaphoreQueue(){ 
		close(); 
	}

// 方法
public:

	bool push(const T&t,bool push_back = true) {
		if(!m_bOpen) return false;

#ifdef WIN32
		HANDLE hd[] = {m_semin ,m_hExit};
		if(WAIT_OBJECT_0 != WaitForMultipleObjects(2,hd,FALSE,-1)) return false;
#else
		while( -1 == sem_wait(&m_semin) ) {
			if(errno != EINTR && errno != EAGAIN ) {
				return false; 
			}
		}
#endif

		{
			AutoLock<CriticalSection> lock(critical_locker_);
			if(!m_bOpen) 
			{
#ifndef WIN32
				sem_post(&m_semin);
#endif
				return false;
			}
			if(push_back)
				queue_.push_back(t);
			else
				queue_.push_front(t);
			m_nSize++;
		}
#ifdef WIN32
		return TRUE == ReleaseSemaphore(m_semout,1,0);
#else
		return 0 ==sem_post(&m_semout);
#endif
	}

	ResultCode push_timedwait(const T& t,unsigned int ms,bool push_back = true)
	{
		if(!m_bOpen) return FAILED;

#ifdef WIN32
		HANDLE hd[] = {m_semin ,m_hExit};
		DWORD dw = WaitForMultipleObjects(2,hd,FALSE,ms);
		if(WAIT_TIMEOUT == dw) return TIMEOUT;
		if(WAIT_OBJECT_0 !=dw) return FAILED;
#else
		if(ms != -1){
			timeval now;
			timespec timeout_spec;
			gettimeofday(&now, 0);
			timeout_spec.tv_sec = now.tv_sec+ms/1000;
			timeout_spec.tv_nsec = now.tv_usec*1000+ms%1000 * 1000000;
			if(timeout_spec.tv_nsec>= 1000000000){
				timeout_spec.tv_nsec -= 1000000000;
				timeout_spec.tv_sec += 1;
			}
			while( -1 == sem_timedwait(&m_semin,&timeout_spec) )
			{
				if(errno == ETIMEDOUT) return TIMEOUT; 
				if(errno != EINTR) return FAILED; 
			}
		}else{
			while( -1 == sem_wait(&m_semin) )
			{
				if(errno != EINTR) return FAILED; 
			}
		}
#endif

		{
			AutoLock<CriticalSection> lock(critical_locker_);
			if(!m_bOpen) 
			{
#ifndef WIN32
				sem_post(&m_semin);
#endif
				return false;
			}
			if(push_back)
				queue_.push_back(t);
			else
				queue_.push_front(t);
			m_nSize++;
		}
#ifdef WIN32
		return (TRUE == ReleaseSemaphore(m_semout,1,0))?OK:FAILED;
#else
		return (0 ==sem_post(&m_semout))?OK:FAILED;
#endif
	}

	bool peek(T& t)
	{
		if(!m_bOpen) return false;
#ifdef WIN32
		HANDLE hd[] = {m_semout ,m_hExit};
		if(WAIT_OBJECT_0 != WaitForMultipleObjects(2,hd,FALSE,0)) return false;
#else
		if( -1 == sem_trywait(&m_semout) )return false;
#endif
		{
			AutoLock<CriticalSection> lock(critical_locker_);
			if(m_nSize==0) return false;
			t = queue_.front();
			queue_.pop_front();
			m_nSize--;
		}
#ifdef WIN32
		return TRUE == ReleaseSemaphore(m_semin,1,0);
#else
		return 0 ==sem_post(&m_semin);
#endif
		return true;
	}

	ResultCode pop_timedwait(T& t,unsigned int ms) {
		if(!m_bOpen) return FAILED;
#ifdef WIN32
		HANDLE hd[] = {m_semout ,m_hExit};
		DWORD dw= WaitForMultipleObjects(2,hd,FALSE,ms);
		if(WAIT_TIMEOUT == dw) return TIMEOUT;
		if(WAIT_OBJECT_0 !=dw) return FAILED;
#else
		if(ms!=-1){
			timeval now;
			timespec timeout_spec;
			gettimeofday(&now, 0);
			timeout_spec.tv_sec = now.tv_sec+ms/1000;
			timeout_spec.tv_nsec = now.tv_usec*1000+ms%1000 * 1000000;
			if(timeout_spec.tv_nsec>= 1000000000){
				timeout_spec.tv_nsec -= 1000000000;
				timeout_spec.tv_sec += 1;
			}
			while( -1 == sem_timedwait(&m_semout,&timeout_spec) )
			{
				if(errno == ETIMEDOUT) return TIMEOUT; 
				if(errno != EINTR) return FAILED; 
			}
		}else{
			while( -1 == sem_wait(&m_semout) )
			{
				if(errno != EINTR) return FAILED; 
			}
		}
#endif
		{
			AutoLock<CriticalSection> lock(critical_locker_);
			if(!m_bOpen || !m_nSize) 
			{
#ifndef WIN32
				sem_post(&m_semout);
#endif
				return FAILED;
			}
			t = queue_.front();
			queue_.pop_front();
			m_nSize--;
		}
#ifdef WIN32
		return (TRUE == ReleaseSemaphore(m_semin,1,0))?OK:FAILED;
#else
		return (0 ==sem_post(&m_semin))?OK:FAILED;
#endif

	}
	bool pop(T& t)
	{
		if(!m_bOpen) return false;
#ifdef WIN32
		HANDLE hd[] = {m_semout ,m_hExit};
		if(WAIT_OBJECT_0 != WaitForMultipleObjects(2,hd,FALSE,-1)) return false;
#else
		while( -1 == sem_wait(&m_semout) )
		{
			if(errno != EINTR && 
				errno != EAGAIN ) return false; 
		}
#endif
		{
			AutoLock<CriticalSection> lock(critical_locker_);
			if(!m_bOpen || !m_nSize) 
			{
#ifndef WIN32
				sem_post(&m_semout);
#endif
				return false;
			}
			t = queue_.front();
			queue_.pop_front();
			m_nSize--;
		}
#ifdef WIN32
		return TRUE == ReleaseSemaphore(m_semin,1,0);
#else
		return 0 ==sem_post(&m_semin);
#endif

	}

	bool close() {
		AutoLock<CriticalSection> lock(critical_locker_);
		if(!m_bOpen) return false;
		m_bOpen = false;
#ifdef WIN32
		SetEvent(m_hExit);
#else
		if(-1 == sem_post(&m_semout)) return false;
		if(-1 == sem_post(&m_semin)) return false;

#endif
		T t;
		while (peek(t))
		{
		}

#ifdef WIN32
		if(m_semin)CloseHandle(m_semin);
		if(m_semout)CloseHandle(m_semout);
		if(m_hExit)CloseHandle(m_hExit);
#else
		sem_destroy(&m_semin) ;
		sem_destroy(&m_semout) ;
#endif

		return m_bOpen;
	}
	bool open(int iMax=0)
	{
		AutoLock<CriticalSection> lock(critical_locker_);
		if(m_bOpen) return false;
		m_nSize = 0;
		queue_.clear();
#ifdef WIN32
		m_semin = CreateSemaphore(0,iMax?iMax:MAXLONG,MAXLONG,0);
		m_semout = CreateSemaphore(0,0,MAXLONG,0);
		m_hExit = CreateEvent(0,1,0,0);
#else
		sem_init(&m_semin, 0, iMax?iMax:SEM_VALUE_MAX) ;
		sem_init(&m_semout, 0, 0) ;
#endif

#ifdef WIN32
		while(WAIT_OBJECT_0 == WaitForSingleObject( m_semout, 0L))
		{}
		ResetEvent(m_hExit);
#else
		while(sem_trywait(&m_semout) == 0){};
#endif
		m_bOpen = true;
		return m_bOpen;
	}
	size_t size() const
	{
		/*AutoLock<CriticalSection> lock(critical_locker_);*/
		return m_nSize;
	}
	size_t empty()const 
	{
		return m_nSize==0;
	}
private:
	CriticalSection critical_locker_;
	size_t m_nSize;
	QUEUE queue_;
#ifdef WIN32
	HANDLE m_semin;
	HANDLE m_semout;
	HANDLE m_hExit;
#else
	sem_t m_semin;
	sem_t m_semout;
#endif
	//int m_iMax;
	bool m_bOpen;
};

#endif //__SQMQUEUE_H__
