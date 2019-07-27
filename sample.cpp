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
