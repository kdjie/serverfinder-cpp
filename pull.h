#pragma once 

#include <pthread.h>
#include <vector>

#include "server.h"
#include "Semaphore.h"
#include "redis.h"
#include "container.h"

class PullWorker 
{
private:
    std::vector<ServerNode> m_servers;
    int m_pullInterval;
    thread::CSemaphore m_semaStop;
    CRedis* m_pRedis;
    pthread_t m_tid;
    ServersContainer* m_pContainer;

public:
    PullWorker();
    virtual ~PullWorker();

    void setPullInterval(int pullInterval);
    void setRedis(CRedis* pRedis);
    ServersContainer* startPull();
    void stopPull();

private:
    static void* __threadFunc(void *_lp);
    void __pullLoop();
    void __pull();
};
