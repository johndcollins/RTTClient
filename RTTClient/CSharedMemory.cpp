#include "CSharedMemory.h"
#include "CLogger.h"

using namespace std;
using namespace RTTClient::Common;
using namespace RakNet;

CSharedMemory::CSharedMemory(bool networked, string ipAddress, int port)
{
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

void CSharedMemory::Update()
{
    if (m_bNetworked)
    {
        if (m_pNetworkMgr->g_ConnectionState == CONSTATE_DISC && m_pNetworkMgr->g_ConnectionState == CONSTATE_FAIL)
            m_pNetworkMgr->Connect(m_sIpAddress.c_str(), m_iPort);

        m_pNetworkMgr->Pulse();
    }
}
