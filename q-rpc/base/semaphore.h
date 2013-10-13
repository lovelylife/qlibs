

#ifndef Semaphore_h__
#define Semaphore_h__

#ifdef WIN32
	#include <windows.h>
	typedef HANDLE sem_t;
#else
	#include <pthread.h>
	#include <semaphore.h>
#endif

class Semaphore {
// 构造析构
public:
	Semaphore();
	~Semaphore();

// 操作
public:
	void wait();
	void release();

private:
	sem_t sem;
};


#ifdef WIN32

inline Semaphore::Semaphore() {
	sem = CreateSemaphore(NULL, 0, 8192, NULL);
}

inline Semaphore::~Semaphore() {
	if (sem) CloseHandle(sem);
}

inline void Semaphore::wait() {
	WaitForSingleObject(sem, INFINITE);
}

inline void Semaphore::release() {
	ReleaseSemaphore(sem, 1, NULL);
}

#else		// NO-WIN32

inline Semaphore::Semaphore() {
	sem_init(&sem, 0, 0);
}

inline Semaphore::~Semaphore() {
	sem_destroy(&sem);
}

inline void Semaphore::wait() {
	sem_wait(&sem);
}

inline void Semaphore::release() {
	sem_post(&sem);
}

#endif		// WIN32

#endif // Semaphore_h__
