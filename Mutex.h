//============================================================================
// Name        : Mutex.h
// Author      : kdjie
// Version     : 0.1
// Copyright   : @2012
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include <pthread.h>

namespace thread
{
	class CMutex
	{
	public:
		CMutex(bool bRecur = false)
		{
			pthread_mutexattr_t mutexattr;
			pthread_mutexattr_init(&mutexattr);

			if (bRecur)
				pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);

			pthread_mutex_init(&m_mutex, &mutexattr);
			pthread_mutexattr_destroy(&mutexattr);
		}
		virtual ~CMutex()
		{
			pthread_mutex_destroy(&m_mutex);
		}

		pthread_mutex_t& Handle()
		{
			return m_mutex;
		}

		void Lock()
		{
			pthread_mutex_lock(&m_mutex);
		}
		void UnLock()
		{
			pthread_mutex_unlock(&m_mutex);
		}
		bool TryLock()
		{
			return (pthread_mutex_trylock(&m_mutex) == 0);
		}

	protected:
		CMutex(const CMutex&) {}
		CMutex& operator = (const CMutex&) { return *this; }

	private:
		pthread_mutex_t m_mutex;
	};

	class CRWlock
	{
	public:
		CRWlock()
		{
			pthread_rwlock_init(&m_rwlock, NULL);
		}
		virtual ~CRWlock()
		{
			pthread_rwlock_destroy(&m_rwlock);
		}

		pthread_rwlock_t& Handle()
		{
			return m_rwlock;
		}

		void RDLock()
		{
			pthread_rwlock_rdlock(&m_rwlock);
		}
		void WRLock()
		{
			pthread_rwlock_wrlock(&m_rwlock);
		}
		void UnLock()
		{
			pthread_rwlock_unlock(&m_rwlock);
		}

		bool TryRDLock()
		{
			return (pthread_rwlock_tryrdlock(&m_rwlock) == 0);
		}
		bool TryWRLock()
		{
			return (pthread_rwlock_trywrlock(&m_rwlock) == 0);
		}

	protected:
		CRWlock(const CRWlock&) {}
		CRWlock& operator = (const CRWlock&) { return *this; }

	private:
		pthread_rwlock_t m_rwlock;
	};

    template <typename T>
	class CSpinLock
	{
		T& m_lock;
	public:
		CSpinLock(T &lock) : m_lock(lock)
		{
			m_lock.Lock();
		}
		~CSpinLock()
		{
			m_lock.UnLock();
		}
	};
}
