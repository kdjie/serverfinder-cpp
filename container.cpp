#include "container.h"

#include <algorithm>

void ServersContainer::Update(const std::vector<ServerNode>& servers)
{
    thread::CSpinLock<thread::CMutex> lk(m_lock);

    m_servers = servers;

    // 创建索引
    __createNameIndexs();
    __createTagIndexs();
}

void ServersContainer::__createNameIndexs()
{
    m_mapNameIndexs.clear();

    for (unsigned int i = 0; i < m_servers.size(); ++i)
    {
        m_mapNameIndexs[ m_servers[i].name ].push_back(i);
    }
}
void ServersContainer::__createTagIndexs()
{
    m_mapTagIndexs.clear();

    for (unsigned int i = 0; i < m_servers.size(); ++i)
    {
        const ServerNode& server = m_servers[i];
        for (std::vector<std::string>::const_iterator c_iter = server.tags.begin(); c_iter != server.tags.end(); ++c_iter)
        {
            m_mapTagIndexs[ *c_iter ].push_back(i);
        }
    }
}

std::vector<ServerNode> ServersContainer::__getServersByIndexs(const std::vector<int>& indexs)
{
    std::vector<ServerNode> servers;

    for (std::vector<int>::const_iterator c_iter = indexs.begin(); c_iter != indexs.end(); ++c_iter)
    {
        servers.push_back( m_servers[*c_iter] );
    }

    return servers;
}

std::vector<ServerNode> ServersContainer::GetServers()
{
    thread::CSpinLock<thread::CMutex> lk(m_lock);

    return m_servers;
}

std::vector<ServerNode> ServersContainer::GetServersByFilters(const std::vector<Filter>& filters)
{
    thread::CSpinLock<thread::CMutex> lk(m_lock);

	// 对名称、标签过滤器进行分类汇总，
	// 分别统计出各自的过滤器数量和索引集合

    std::vector<int> nameIndexs, tagIndexs;
    bool nameOperate = false, tagOperate = false;

    for (std::vector<Filter>::const_iterator c_iter = filters.begin(); c_iter != filters.end(); ++c_iter)
    {
        const Filter& filter = *c_iter;

        if (filter.type == "name")
        {
            nameOperate = true;

            MAP_KEY_INDEXS_t::iterator iterF = m_mapNameIndexs.find(filter.value);
            if (iterF != m_mapNameIndexs.end())
            {
                std::copy(iterF->second.begin(), iterF->second.end(), std::back_inserter(nameIndexs));
            }
        }

        if (filter.type == "tag")
        {
            tagOperate = true;

            MAP_KEY_INDEXS_t::iterator iterF = m_mapTagIndexs.find(filter.value);
            if (iterF != m_mapTagIndexs.end())
            {
                for (std::vector<int>::iterator iterT = iterF->second.begin(); iterT != iterF->second.end(); ++iterT)
                {
                    // 去重
                    if (std::find(tagIndexs.begin(), tagIndexs.end(), *iterT) == tagIndexs.end())
                    {
                        tagIndexs.push_back(*iterT);
                    }
                }
            }
        }
    }

    // 然后根据过滤器分类，进行按类别取交集

    std::vector<int> resultIndexs;
    bool andOperate = false;

    if (nameOperate > 0)
    {
        if (!andOperate)
        {
            std::copy(nameIndexs.begin(), nameIndexs.end(), std::back_inserter(resultIndexs));
        }
        else
        {
            std::vector<int> joinIndexs;
            for (std::vector<int>::iterator iterA = resultIndexs.begin(); iterA != resultIndexs.end(); ++iterA)
            {
                for (std::vector<int>::iterator iterB = nameIndexs.begin(); iterB != nameIndexs.end(); ++iterB)
                {
                    if (*iterA == *iterB)
                    {
                        joinIndexs.push_back(*iterA);
                        continue;
                    }  
                }
            }

            resultIndexs = joinIndexs;
        }
        
        andOperate = true;
    }

    if (tagOperate > 0)
    {
        if (!andOperate)
        {
            std::copy(tagIndexs.begin(), tagIndexs.end(), std::back_inserter(resultIndexs));
        }
        else
        {
            std::vector<int> joinIndexs;
            for (std::vector<int>::iterator iterA = resultIndexs.begin(); iterA != resultIndexs.end(); ++iterA)
            {
                for (std::vector<int>::iterator iterB = tagIndexs.begin(); iterB != tagIndexs.end(); ++iterB)
                {
                    if (*iterA == *iterB)
                    {
                        joinIndexs.push_back(*iterA);
                        continue;
                    }  
                }
            }

            resultIndexs = joinIndexs;
        }
        
        andOperate = true;
    }

    // 根据索引取服务器
	return __getServersByIndexs(resultIndexs);
}
