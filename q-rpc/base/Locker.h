
#ifndef __LOCKER_H__
#define __LOCKER_H__

#ifndef WIN32
	#include <pthread.h>
	#include <sys/types.h>
	#include <errno.h>
	#if defined( __GNUC__ ) && ( defined( __i386__ ) || defined( __x86_64__ ) )
		#include "InterLock.h"
	#endif
#else
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#ifdef WIN32
	typedef HANDLE pthread_t;
	typedef HANDLE pthread_mutex_t;
	typedef HANDLE pthread_cond_t;
	typedef DWORD pthread_key_t;
#endif

class ILock {
public:
	virtual long lock() = 0;
	virtual long unlock() = 0;
	virtual ~ILock(){}
	ILock(){};

private:	
	ILock(const ILock&rhs){}
	ILock& operator=(const ILock& rhs) {}
};

//////////////////////////////////////////////////////////////////////////
// Template class AutoLock

template <class T> 
class AutoLock {
public:
	AutoLock(T& locker) : m_locker(locker) {
		m_locker.lock();
	}

	~AutoLock() {
		m_locker.unlock();
	}

private:
	T& m_locker;
};

//////////////////////////////////////////////////////////////////////////
// Template class AutoLockEx

template <class T> 
class AutoLockEx {
public:
	AutoLockEx(T& locker,bool block = true,bool bManu = false) 
	  : m_locker(locker)
	  , m_bLocked(false)
	{
	  if(!bManu) {
	    if (block == true) {
		  m_locker.lock();
		  m_bLocked = true;
		} else {
		  m_bLocked = m_locker.try_lock();
		}
	  }
	}

	~AutoLockEx(){
		if(m_bLocked)
			m_locker.Unlock();
	}

	bool locked(){ 
		return m_bLocked; 
	}

	bool try_lock(){
		m_bLocked = m_locker.tryLock();
		return m_bLocked;
	}
	void unlock(){
		if(m_bLocked){ 
			m_bLocked=false; 
			m_locker.unlock();
		}
	}
private:
	T& m_locker;
	bool m_bLocked;
};

//////////////////////////////////////////////////////////////////////////
// class MutexLock

class MutexLock : public ILock {
public:
	MutexLock(const char* name = 0) {
#ifdef WIN32
		m_hMutex  = CreateMutexA(NULL, FALSE, name); 
#else
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&m_hMutex, &attr);
#endif
	}

	~MutexLock() {
#ifdef WIN32
		CloseHandle(m_hMutex);
#else
		pthread_mutex_destroy(&m_hMutex);
#endif		
	}

	bool try_lock() {
#ifdef WIN32
		return WaitForSingleObject(m_hMutex,0) == WAIT_OBJECT_0 ;
#else
		return pthread_mutex_trylock(&m_hMutex) != EBUSY;
#endif
	}

	long lock()	{
#ifdef WIN32
		if(WaitForSingleObject(m_hMutex,INFINITE) == WAIT_OBJECT_0)  {
			return 1;
		}

		return 0;
#else
		pthread_mutex_lock(&m_hMutex);

		return 1;
#endif
}

	long unlock() {
#ifdef WIN32
		ReleaseMutex(m_hMutex);
#else
		pthread_mutex_unlock(&m_hMutex);
#endif
		return 1;
	}

private:
	pthread_mutex_t  m_hMutex;            // Alias name of the mutex to be protected
};

typedef MutexLock MutexLockEx;


#ifdef WIN32
class CriticalSection : public ILock {
public:
	CriticalSection() {
		InitializeCriticalSection(&m_csLock);
	}

	~CriticalSection(){
		DeleteCriticalSection(&m_csLock);
	}

	long lock() {
		EnterCriticalSection(&m_csLock);
		return m_csLock.LockCount;
	}

	long unlock(){
		LeaveCriticalSection(&m_csLock);
		return m_csLock.LockCount;
	}

	bool try_lock()	{
		return TryEnterCriticalSection(&m_csLock) != 0;
	}

private:
	CRITICAL_SECTION m_csLock;
};

typedef CriticalSection CriticalSectionEx;
#else
typedef MutexLock CriticalSection;
#endif //WIN32

#endif //__LOCKER_H__
