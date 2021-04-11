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
    if (m_pNetworkMgr->g_ConnectionState == CONSTATE_COND)
        LOG_DEBUG("CSharedMemory::CSharedMemory() Connected");
    else if (m_pNetworkMgr->g_ConnectionState == CONSTATE_DISC)
        LOG_DEBUG("CSharedMemory::CSharedMemory() Disconnected");
    else if (m_pNetworkMgr->g_ConnectionState == CONSTATE_FAIL)
        LOG_DEBUG("CSharedMemory::CSharedMemory() Conenction failed");
}

CSharedMemory::~CSharedMemory()
{
    if (m_bNetworked)
        m_pNetworkMgr->Disconnect(true);
}

SDL_Surface* CSharedMemory::DisplayImage(int display)
{
    if (m_bNetworked)
    {
        return m_pNetworkMgr->DisplayImage(display);
    }
    
    return nullptr;
}

void CSharedMemory::Update()
{
    m_iLastFrame = SDL_GetTicks();
    if (m_iLastFrame >= (m_iLastFrame + 1000))
    {
        m_iLastTime = m_iLastFrame;
        m_iFpsActual = m_iFrameCount;
        m_iFrameCount = 0;
    }

    m_iFrameCount++;
    m_iTimerFPS = SDL_GetTicks() - m_iLastFrame;
    if (m_iTimerFPS < (1000 / m_iFps))
    {
        if (m_bNetworked)
        {
            if (m_pNetworkMgr->g_ConnectionState == CONSTATE_DISC && m_pNetworkMgr->g_ConnectionState == CONSTATE_FAIL)
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
}

int CSharedMemory::GetFps()
{
    return m_iFps;
}
