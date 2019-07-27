#include "pull.h"

#include <sstream>

using namespace tinyredis;

PullWorker::PullWorker()
: m_pullInterval(10)
, m_pRedis(NULL)
, m_pContainer(NULL)
{
    m_pContainer = new ServersContainer();
}
PullWorker::~PullWorker()
{
    delete m_pContainer;
}

void PullWorker::setPullInterval(int pullInterval)
{
    m_pullInterval = pullInterval;
}

void PullWorker::setRedis(CRedis* pRedis)
{
    m_pRedis = pRedis;
}

ServersContainer* PullWorker::startPull()
{
    pthread_create(&m_tid, NULL, __threadFunc, this);
    return m_pContainer;
}

void PullWorker::stopPull()
{
    m_semaStop.Post();
    // 等待线程退出
    pthread_join(m_tid, NULL);
}

void* PullWorker::__threadFunc(void *_lp)
{
    PullWorker* pThis = (PullWorker*)_lp;

    pThis->__pullLoop();

    return NULL;
}

void PullWorker::__pullLoop()
{
    __pull();

    while ( !m_semaStop.TimedWait(m_pullInterval*1000) ) 
    {
        __pull();
    }
}

void PullWorker::__pull()
{
    if (m_pRedis == NULL)
        return;

    CRedisClient* pClient = m_pRedis->getRedis("");

    CResult result(true);
    result = pClient->command("keys %s_*", SERVER_KEY_PREFIX);
    if (!result)
        return;

    if (!result.isArray())
        return;

    std::vector<std::string> vecKeys;

    for (size_t i = 0; i < result.getArraySize(); ++i)
    {
        CResult subResult(false);
        subResult = result.getSubReply(i);
        if (subResult.isString())
        {
            std::string strKey;
            subResult.getString(strKey);
            vecKeys.push_back(strKey);
        }
    }

    // 为空时仍然更新容器
    if (vecKeys.empty())
    {
        m_servers.clear();
        m_pContainer->Update(m_servers);
        return;
    }

    std::stringstream ssCmd;
    ssCmd << "mget";
    for (std::vector<std::string>::const_iterator c_iter = vecKeys.begin(); c_iter != vecKeys.end(); ++c_iter)
    {
        ssCmd << " " << *c_iter;
    }

    result = pClient->command(ssCmd.str().c_str());
    if (!result)
        return;

    if (!result.isArray())
        return;

    m_servers.clear();
    
    for (size_t i = 0; i < result.getArraySize(); ++i)
    {
        CResult subResult(false);
        subResult = result.getSubReply(i);
        if (subResult.isString())
        {
            std::string strJson;
            subResult.getString(strJson);

            //printf("%s : %s \n", vecKeys[i].c_str(), strJson.c_str());

            // 解析
            ServerNode node;
            if ( node.fromJson(strJson) )
            {
                m_servers.push_back(node);
            }
        }
    }

    // 更新容器
    m_pContainer->Update(m_servers);
}
