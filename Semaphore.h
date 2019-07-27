//============================================================================
// Name        : Semaphore.h
// Author      : kdjie
// Version     : 0.1
// Copyright   : @2012
// Description : 14166097@qq.com
//============================================================================

#pragma once

#include <semaphore.h>
#include <errno.h>
#include <sys/time.h>

namespace thread
{
	class CSemaphore
	{
	public:
		CSemaphore()
		{
			sem_init(&m_sema, 0, 0);
		}
		virtual ~CSemaphore()
		{
			sem_destroy(&m_sema);
		}

		sem_t& Handle()
		{
			return m_sema;
		}

		bool Post()
		{
			return (sem_post(&m_sema) == 0);
		}

		bool Wait()
		{
		    while (sem_wait(&m_sema) != 0)
		    {
		        if (errno != EINTR)
		            return false;
		    }
		    return true;
		}

		bool TryWait()
		{
			return (sem_trywait(&m_sema) == 0);
		}

		bool TimedWait(int minSecond)
		{
			struct timeval tv;
			gettimeofday(&tv, NULL);

			struct timespec ts;
			ts.tv_sec = tv.tv_sec;
			ts.tv_nsec = 0;
			ts.tv_sec += (tv.tv_usec + minSecond*1000)/1000000;
			ts.tv_nsec = ((tv.tv_usec + minSecond*1000)%1000000)*1000;

		    while (sem_timedwait(&m_sema, &ts) != 0)
		    {
		        if (errno != EINTR)
		            return false;
		    }
		    return true;
		}

	protected:
		CSemaphore(const CSemaphore&) {}
		CSemaphore& operator = (const CSemaphore&) { return *this; }

	private:
		sem_t m_sema;
	};
}
