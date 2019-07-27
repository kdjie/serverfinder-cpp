#include "redis.h"

using namespace tinyredis;

CRedis::CRedis() {}
CRedis::~CRedis() {}

void CRedis::setConfig(const std::string &strIp, unsigned short uPort, const std::string &strPass, int nDB)
{
    m_redisFactory.addRedis(strIp, uPort, strPass, nDB, 1000);
}

CRedisClient* CRedis::getRedis(const std::string &strKey)
{
    return m_redisFactory.getRedis(strKey); 
}
