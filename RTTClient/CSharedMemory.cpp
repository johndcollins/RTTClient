// ------------------------------------------------------------------------------------------------
// Copyright (c) John Collins. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------

#include "CSharedMemory.h"
#include "CLogger.h"

using namespace std;
using namespace RTTClient::Common;
using namespace RakNet;

CSharedMemory::CSharedMemory(int defaultFps, bool networked, string ipAddress, int port)
{
    m_iFps = defaultFps;
	m_bNetworked = networked;
	m_sIpAddress = ipAddress;
	m_iPort = port;

    m_pNetworkMgr = new CNetworkManager();
    if (m_bNetworked)
    {
        if (!m_pNetworkMgr->Start())
            LOG_DEBUG("CSharedMemory::CSharedMemory() Network Start Failed");
        else
            LOG_DEBUG("CSharedMemory::CSharedMemory() Network Started");
    }

    LOG_DEBUG("CSharedMemory::CSharedMemory() Connecting...");
    m_pNetworkMgr->Connect(m_sIpAddress.c_str(), m_iPort);
    if (m_pNetworkMgr->g_iConnectionState == CONSTATE_COND)
        LOG_DEBUG("CSharedMemory::CSharedMemory() Connected");
    else if (m_pNetworkMgr->g_iConnectionState == CONSTATE_CONN)
        LOG_DEBUG("CSharedMemory::CSharedMemory() Connecting");
    else if (m_pNetworkMgr->g_iConnectionState == CONSTATE_DISC)
        LOG_DEBUG("CSharedMemory::CSharedMemory() Disconnected");
    else if (m_pNetworkMgr->g_iConnectionState == CONSTATE_FAIL)
        LOG_DEBUG("CSharedMemory::CSharedMemory() Conenction failed");
}

CSharedMemory::~CSharedMemory()
{
    if (m_bNetworked)
        m_pNetworkMgr->Disconnect(true);
}

bool CSharedMemory::Connected()
{
    return (m_pNetworkMgr->g_iConnectionState == CONSTATE_COND);
}

std::vector<unsigned char> CSharedMemory::DisplayImage(int display)
{
    if (m_bNetworked)
    {
        return m_pNetworkMgr->DisplayImage(display);
    }
    
    return std::vector<unsigned char>(0);
}

void CSharedMemory::Update()
{
    if (m_bNetworked)
    {
        if (m_pNetworkMgr->g_iConnectionState == CONSTATE_DISC || m_pNetworkMgr->g_iConnectionState == CONSTATE_FAIL)
            m_pNetworkMgr->Connect(m_sIpAddress.c_str(), m_iPort);

        m_pNetworkMgr->Pulse();
        int serverFps = m_pNetworkMgr->GetFPS();
        if (serverFps != 0)
            m_iFps = serverFps;
    }
    else
    {
        // Read Shared memory if we are on a windows machine...not happening here
    }
}

int CSharedMemory::GetFps()
{
    return m_iFps;
}
