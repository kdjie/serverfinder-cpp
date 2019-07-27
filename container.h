#pragma once 

#include <string>
#include <vector>
#include <map>

#include "server.h"
#include "Mutex.h"

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

class ServersContainer
{
private:
    std::vector<ServerNode> m_servers;
    typedef std::map<std::string, std::vector<int> > MAP_KEY_INDEXS_t;
    MAP_KEY_INDEXS_t m_mapNameIndexs;
    MAP_KEY_INDEXS_t m_mapTagIndexs;
    thread::CMutex m_lock;

public:
    ServersContainer() {}
    virtual ~ServersContainer() {}

    void Update(const std::vector<ServerNode>& servers);
    std::vector<ServerNode> GetServers();
    std::vector<ServerNode> GetServersByFilters(const std::vector<Filter>& filters);

private:
    void __createNameIndexs();
    void __createTagIndexs();
    std::vector<ServerNode> __getServersByIndexs(const std::vector<int>& indexs);
};
