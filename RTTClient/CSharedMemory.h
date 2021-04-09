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
	int SetFps(int fps);

private:
	bool m_bNetworked;
	string m_sIpAddress;
	int m_iPort;

	int m_iFrameRate;
	int m_iLastTime;
	int m_iFrameCount;
	int m_iTimerFPS;
	int m_iLastFrame;
	int m_iFpsActual;
	int m_iFps;

	CNetworkManager* m_pNetworkMgr;

};