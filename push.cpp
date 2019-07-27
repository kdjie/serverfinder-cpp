#include "push.h"

using namespace tinyredis;

PushWorker::PushWorker()
: m_expireSeconds(30)
, m_pushInterval(10)
, m_pRedis(NULL)
{
}
PushWorker::~PushWorker()
{
}

void PushWorker::setName(const std::string &strName)
{
    m_server.name = strName;
}

void PushWorker::setIpPort(const std::string &strIp, unsigned short uPort)
{
    m_server.ip = strIp;
    m_server.port = uPort;
}

void PushWorker::addTag(const std::string &strTag)
{
    m_server.tags.push_back(strTag);
}

void PushWorker::setExpire(int seconds)
{
    m_expireSeconds = seconds;
}

void PushWorker::setPushInterval(int seconds)
{
    m_pushInterval = seconds;
}

void PushWorker::setRedis(CRedis* pRedis)
{
    m_pRedis = pRedis;
}

void PushWorker::startPush()
{
    pthread_create(&m_tid, NULL, __threadFunc, this);
}

void PushWorker::stopPush()
{
    m_semaStop.Post();
    // 等待线程退出
    pthread_join(m_tid, NULL);
}

void* PushWorker::__threadFunc(void *_lp)
{
    PushWorker* pThis = (PushWorker*)_lp;

    pThis->__pushLoop();

    return NULL;
}

void PushWorker::__pushLoop()
{
    __push();

    while ( !m_semaStop.TimedWait(m_pushInterval*1000) ) 
    {
        __push();
    }

    __delete();
}

void PushWorker::__push()
{
    if (m_pRedis == NULL)
        return;
    
    std::string strKey = m_server.toNodeKey();
    CRedisClient* pClient = m_pRedis->getRedis( strKey );

    CResult result(true);
    result = pClient->command("set %s %s ex %d", strKey.c_str(), m_server.toJson().c_str(), m_expireSeconds);
}

void PushWorker::__delete()
{
    if (m_pRedis == NULL)
        return;
    
    std::string strKey = m_server.toNodeKey();
    CRedisClient* pClient = m_pRedis->getRedis( strKey );

    CResult result(true);
    result = pClient->command("del %s", strKey.c_str());
}
