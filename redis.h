#pragma once 

#include "libtinyredis/RedisFactory.h"

class CRedis
{
private:
    tinyredis::CRedisFactory m_redisFactory;

public:
    CRedis();
    virtual ~CRedis();

    void setConfig(const std::string& strIp, unsigned short uPort, const std::string& strPass, int nDB);

    tinyredis::CRedisClient* getRedis(const std::string& strKey);
};
