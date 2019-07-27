#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <json/json.h>

#define SERVER_KEY_PREFIX "SK"

struct ServerNode {
	std::string name;
	std::string ip;
	unsigned short port;
	std::vector<std::string> tags;
	long long livetick;

	ServerNode()
	{
		port = 0;
		livetick = 0;
	}

	std::string toNodeKey() const
	{
		char szLine[128] = {0};
		snprintf(szLine, sizeof(szLine)-1, "%s_%s_%s:%d", SERVER_KEY_PREFIX, name.c_str(), ip.c_str(), port);
		return szLine;
	}

	std::string toJson() const
	{
		Json::Value jsVal;
		jsVal["name"] = name;
		jsVal["ip"] = ip;
		jsVal["port"] = port;
		for (std::vector<std::string>::const_iterator c_iter = tags.begin(); c_iter != tags.end(); ++c_iter) 
		{
			jsVal["tags"].append( Json::Value(*c_iter) );
		}
		jsVal["livetick"] = (long long)time(NULL);

		Json::FastWriter writer;
		return writer.write(jsVal);
	}

	bool fromJson(const std::string& strJson)
	{
		Json::Value jsVal;
		Json::Reader reader;
		if (!reader.parse(strJson, jsVal))
			return false;

		name = jsVal["name"].asString();
		ip = jsVal["ip"].asString();
		port = jsVal["port"].asUInt();
		for (unsigned int i = 0; i < jsVal["tags"].size(); ++i)
		{
			tags.push_back( jsVal["tags"][i].asString() );
		}
		livetick = jsVal["livetick"].asInt64();

		return true;
	}
};
