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

	void Update();

	int GetFps();

private:
	bool m_bNetworked = false;
	string m_sIpAddress = "";
	int m_iPort = 0;

	int m_iFrameRate = 0;
	int m_iLastTime = 0;
	int m_iFrameCount = 0;
	int m_iTimerFPS = 0;
	int m_iLastFrame = 0;
	int m_iFpsActual = 0;
	int m_iFps = 0;

	CNetworkManager* m_pNetworkMgr;

};