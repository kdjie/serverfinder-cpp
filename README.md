serverfinder-cpp 简单分布布服务器发现c++实现库
===================================

### 项目说明：

在分布式应用中，怎样处理多个节点的互相感知问题，这是一门必修课。然而，应用有大有小，功课也可繁可简，本项目正是从简出发的一个工程实现。

很多项目在初创阶段，从简单性考虑，有时会采用配置文件来实现节点共享。然而，这却带来了一个很大的运维问题，因为每次节点变更，都需要手工变更配置文件，并进行节点间分发。更严重的是，节点异常掉线时，却无法感知。

本项目正是为了解决这个问题，但是又不至于太复杂。实现思路是利用redis做为代理人，所有节点定时向它上报自已的节点信息，并且定时从它那儿拉取所有节点信息。如果某个节点发生故障，redis会自动过期其数据，实现节点间动态感知分享。
```
graph LR
ServerA-->Redis
Redis-->ServerB
```
由于定时拉取对流量和CPU的资源消耗比较大，建议总节点数在500以内的中小型站点使用。

### 数据结构介绍：

###### 服务器节点：
Key的完整格式："SK_服务名_IP:PORT" <br>
Value的数据格式定义：<br>

```
struct ServerNode 
{
	std::string name;
	std::string ip;
	unsigned short port;
	std::vector<std::string> tags;
	long long livetick;
	……
}
```

字段说明：<br>
name 服务器名字，用于节点类型的识别。<br>
ip、Port 服务器工作的Ip、Port，用于外部向其连接。<br>
tags 服务器的标签集合，可用于App识别、机房识别等用途。<br>
livetick 这个是服务器最新的写入时间，用于活跃性检查。<br>

###### redis操作对象：

```
class CRedis
{
private:
	tinyredis::CRedisFactory m_redisFactory;
	……
public:
	void setConfig(const std::string& strIp, unsigned short uPort, const std::string& strPass, int nDB);
	tinyredis::CRedisClient* getRedis(const std::string& strKey);
	……
}
```

字段说明：<br>
m_redisFactory libtinyredis库的工厂对象，用于创建和分配redis客户端。

方法介绍：<br>
void setConfig(const std::string& strIp, unsigned short uPort, const std::string& strPass, int nDB); <br>
这个方法用于设置redis服务器地址。<br>

tinyredis::CRedisClient* getRedis(const std::string& strKey); <br>
这个方法用于传入服务器节点Key，获取redis客户端，主要由内部调用，所以我们不需要关心。<br>


###### 服务器节点推送者：

```
class PushWorker
{
private:
	ServerNode m_server;
	int m_expireSeconds;
	int m_pushInterval;
	thread::CSemaphore m_semaStop;
	CRedis* m_pRedis;
	pthread_t m_tid;
	……
public:
	void setName(const std::string& strName);
	void setIpPort(const std::string& strIp, unsigned short uPort);
	void addTag(const std::string& strTag);
	void setExpire(int seconds);
	void setPushInterval(int seconds);
	void setRedis(CRedis* pRedis);

	void startPush();
	void stopPush();
	……
}
```

字段说明：<br>
m_server 负责推送的服务器节点。<br>
m_expireSeconds 过期时间设置，默认30S。<br>
m_pushInterval 推送时间间隔设置，默认10S。<br>
m_semaStop 工作线程退出信号。<br>
m_pRedis redis操作对象指针。<br>
m_tid 工作线程ID。<br>

方法介绍：<br>
void setName(const std::string& strName);<br>
void setIpPort(const std::string& strIp, unsigned short uPort);<br>
void addTag(const std::string& strTag);<br>
void setExpire(int seconds);<br>
以上方法用于设置推送服务器节点的名称、IP、Port、标签、过期时间。<br>

void setPushInterval(int seconds);<br>
这个方法设置定时推送的周期，默认10S。<br>

void setRedis(CRedis* pRedis);<br>
这个方法用于设置redis操作对象。<br>

void startPush();<br>
void stopPush();<br>
这两个方法用于启动和停止推送。推送会在工作线程中进行，每周期推送一次。<br>


###### 所有服务器列表的容器索引：

```
class ServersContainer
{
private:
	std::vector<ServerNode> m_servers;
	typedef std::map<std::string, std::vector<int> > MAP_KEY_INDEXS_t;
	MAP_KEY_INDEXS_t m_mapNameIndexs;
	MAP_KEY_INDEXS_t m_mapTagIndexs;
	thread::CMutex m_lock;
	……
public:
	void Update(const std::vector<ServerNode>& servers);
	std::vector<ServerNode> GetServers();
	std::vector<ServerNode> GetServersByFilters(const std::vector<Filter>& filters);
	……
}

```

字段说明：<br>
m_servers 所有服务器节点集合。<br>
m_mapNameIndexs 服务名索引。<br>
m_mapTagIndexs 服务标签索引。<br>
m_lock 线程锁。<br>

方法介绍：<br>
void Update(const std::vector<ServerNode>& servers);<br>
这个方法用于更新容器索引，内部使用。<br>

std::vector<ServerNode> GetServers();<br>
这个方法用于获取所有的服务器节点集合。<br>

std::vector<ServerNode> GetServersByFilters(const std::vector<Filter>& filters);<br>
这个方法用于获取指定规则的服务器节点集合，Filter可按需组合，结果取交集。<br>

关于Filter的定义如下：

```
struct Filter
{
    std::string type;
    std::string value;

    Filter()
    {
        type = "";
        value = "";
    }
};

struct NameFilter : public Filter
{
    NameFilter(const std::string& name)
    {
        type = "name";
        value = name;
    }
};

struct TagFilter : public Filter
{
    TagFilter(const std::string& tag)
    {
        type = "tag";
        value = tag;
    }
};

```

###### 服务器节点拉取者：

```
class PullWorker 
{
private:
	std::vector<ServerNode> m_servers;
	int m_pullInterval;
	thread::CSemaphore m_semaStop;
	CRedis* m_pRedis;
	pthread_t m_tid;
	ServersContainer* m_pContainer;
	……
public:
	void setPullInterval(int pullInterval);
	void setRedis(CRedis* pRedis);
	ServersContainer* startPull();
	void stopPull();
	……
}
```

字段说明：<br>
m_servers 所有服务器节点集合。<br>
m_pullInterval 定时拉取时间周期，默认10S。<br>
m_semaStop 工作线程退出信号。<br>
m_pRedis redis操作对象指针。<br>
m_tid 工作线程ID。<br>
m_pContainer 服务器列表容器索引。<br>

方法介绍：<br>
void setPullInterval(int pullInterval);<br>
这个方法用于设置拉取定时间隔，默认10S。<br>

void setRedis(CRedis* pRedis);<br>
这个方法用于设置redis操作对象。<br>

ServersContainer* startPull();<br>
这个方法用于启动拉取，返回容器操作指针。通过这个容器，可以获取指定规则的服务器节点集合。拉取会在工作线程中进行，每周期拉取一次。

void stopPull();<br>
这个方法停止拉取。<br>


###### 最后举一个完整的例子：

```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "push.h"
#include "pull.h"

void printServers(const std::string& strDesc, const std::vector<ServerNode>& servers)
{
    printf("%s => \n", strDesc.c_str());
    for (std::vector<ServerNode>::const_iterator c_iter = servers.begin(); c_iter != servers.end(); ++c_iter)
    {
        printf("%s \n", c_iter->toJson().c_str());
    }
}

int main(int argc, char* argv[])
{
    CRedis redis;
    redis.setConfig("127.0.0.1", 6379, "", 9);

    PushWorker push;
    push.setName("test");
    push.setIpPort("127.0.0.1", 1234);
    push.addTag("a");
    push.addTag("b");
    push.setRedis(&redis);
    push.startPush();

    PullWorker pull;
    pull.setRedis(&redis);
    ServersContainer* pContainer = pull.startPull();

    for (int i = 0; i < 10; ++i)
    {
        std::vector<ServerNode> servers0 = pContainer->GetServers();
        printServers("all", servers0);

        std::vector<Filter> filter1;
        filter1.push_back( NameFilter("test") );
        std::vector<ServerNode> servers1 = pContainer->GetServersByFilters(filter1);
        printServers("name:test", servers1);

        std::vector<Filter> filter2;
        filter2.push_back( TagFilter("a") );
        std::vector<ServerNode> servers2 = pContainer->GetServersByFilters(filter2);
        printServers("tag:a", servers2);

        std::vector<Filter> filter3;
        filter3.push_back( TagFilter("c") );
        std::vector<ServerNode> servers3 = pContainer->GetServersByFilters(filter3);
        printServers("tag:c", servers3);

        std::vector<Filter> filter4;
        filter4.push_back( NameFilter("test") );
        filter4.push_back( TagFilter("c") );
        std::vector<ServerNode> servers4 = pContainer->GetServersByFilters(filter4);
        printServers("name:test,tag:c", servers4);

        sleep(10);
    }

    push.stopPush();
    pull.stopPull();

    return 0;
}

```

这个例子在仓库根目录下，名字为sample.cpp，通过make all编译即可。