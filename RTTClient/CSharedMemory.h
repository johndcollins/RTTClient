#pragma once

#include <iostream>
#include "CNetworkManager.h"

using namespace std;

class CSharedMemory
{
public:
	CSharedMemory(bool networked, string ipAddress, int port);
	~CSharedMemory();

	void Update();

private:
	bool m_bNetworked;
	string m_sIpAddress;
	int m_iPort;

	CNetworkManager* m_pNetworkMgr;

};