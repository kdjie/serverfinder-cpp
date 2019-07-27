#pragma once

#include <pthread.h>

#include "server.h"
#include "Semaphore.h"
#include "redis.h"

class PushWorker
{
private:
    ServerNode m_server;
    int m_expireSeconds;
    int m_pushInterval;
    thread::CSemaphore m_semaStop;
    CRedis* m_pRedis;
    pthread_t m_tid;

public:
    PushWorker();
    virtual ~PushWorker();

    void setName(const std::string& strName);
    void setIpPort(const std::string& strIp, unsigned short uPort);
    void addTag(const std::string& strTag);
    void setExpire(int seconds);
    void setPushInterval(int seconds);
    void setRedis(CRedis* pRedis);

    void startPush();
    void stopPush();

private:
    static void* __threadFunc(void *_lp);
    void __pushLoop();
    void __push();
    void __delete();
};
