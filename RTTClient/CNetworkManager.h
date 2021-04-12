#pragma once

#include <iostream>
#include <map>
#include <vector>

#include <RakNetTypes.h>
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

#include "rtt.h"

// Connection Status
#define CONSTATE_DISC	0
#define CONSTATE_CONN	1
#define CONSTATE_COND	2
#define CONSTATE_FAIL	3

class CNetworkManager
{
public:
	CNetworkManager();
	virtual ~CNetworkManager();

	bool Start();
	void Stop();

	void Connect(const char* ip, int port);
	void Disconnect(bool shutdown = false);

	void Pulse();
	std::vector<unsigned char> DisplayImage(int display);

	void SetLastConnection(const char* ip, int port) { m_sLastIP = ip; m_iLastPort = port; }

	int GetFPS();

	RakNet::RakPeerInterface* GetInterface() { return m_pRakPeer; }
	RakNet::SystemAddress GetSystemAddress() { return m_SystemAddr; }

	int	g_iConnectionState = CONSTATE_DISC;

private:
	void Initialize();
	void Destroy();
	void SendHandshake();

	RakNet::RakPeerInterface*	m_pRakPeer = nullptr;
	std::map<int, std::vector<unsigned char>> m_pCurrentImages;

	int							m_iCurrentImageSize = 0;
	int							m_iCurrentImageWidth = 0;
	int							m_iCurrentImageHeight = 0;

	std::string					m_sLastIP = "";
	int							m_iLastPort = 0;
	RakNet::SystemAddress		m_SystemAddr;
	bool						m_bValidHandshake = false;

	int							m_iFps = 0;
};
