// ------------------------------------------------------------------------------------------------
// Copyright (c) John collins. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------

#pragma once

#include <iostream>

#include <SDL2/SDL.h>

#include "CNetworkManager.h"

using namespace std;

class CSharedMemory
{
public:
	CSharedMemory(int defaultFps, bool networked, string ipAddress, int port);
	~CSharedMemory();

	bool Connected();

	std::vector<unsigned char> DisplayImage(int display);
	void Update();

	int GetFps();

private:
	bool m_bNetworked = false;
	string m_sIpAddress = "";
	int m_iPort = 0;

	int m_iFps = 0;

	CNetworkManager* m_pNetworkMgr;

};