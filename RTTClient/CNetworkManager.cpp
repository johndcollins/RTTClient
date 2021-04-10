#include "CNetworkManager.h"
#include "CLogger.h"

#include <iostream>
#include <chrono>
#include <thread>

using namespace RTTClient::Common;

CNetworkManager::CNetworkManager()
{
	LOG_DEBUG("CNetworkManager::CNetworkManager Begin");
	Initialize();
	LOG_DEBUG("CNetworkManager::CNetworkManager End");
};

CNetworkManager::~CNetworkManager()
{
	LOG_DEBUG("CNetworkManager::~CNetworkManager Begin");
	Destroy();
	LOG_DEBUG("CNetworkManager::~CNetworkManager End");
};


void CNetworkManager::Initialize()
{
	LOG_DEBUG("CNetworkManager::Initialize() Begin");

	// Get RakPeerInterface
	m_pRakPeer = RakNet::RakPeerInterface::GetInstance();

	LOG_DEBUG("CNetworkManager::Constructed");

	g_ConnectionState = CONSTATE_DISC;
	m_bValidHandshake = false;

	LOG_DEBUG("CNetworkManager::Initialize() End");
}

void CNetworkManager::Destroy()
{
	LOG_DEBUG("CNetworkManager::Destroy() Begin");

	// Stop RakNet, stops synchronization
	m_pRakPeer->Shutdown(2000);

	LOG_DEBUG("CNetworkManager::Destroy() Stop");
}

bool CNetworkManager::Start()
{
	LOG_DEBUG("CNetworkManager::Start() Begin");

	LOG_DEBUG("CNetworkManager::Start() RakNet Startup");
	// Return whether Startup worked or not
	RakNet::SocketDescriptor desc = RakNet::SocketDescriptor();
	int result = m_pRakPeer->Startup(1, &desc, 1, SCHED_RR);
	
	LOG_DEBUG("CNetworkManager::Start() Stop");

	return (result == RakNet::RAKNET_STARTED);
}

void CNetworkManager::Stop()
{
	LOG_DEBUG("CNetworkManager::Stop() Begin");

	// Disconnect if we're connected
	if (g_ConnectionState == CONSTATE_COND)
		Disconnect();

	LOG_DEBUG("CNetworkManager::Shutting down RakNet");
	// Stop our RakPeerInterface
	m_pRakPeer->Shutdown(50);
	//sleep(15);
	LOG_DEBUG("CNetworkManager::Stop() RakNet shut down.");

	LOG_DEBUG("CNetworkManager::Stop() Stop");
}

void CNetworkManager::Connect(const char* ip, int port)
{
	LOG_DEBUG("CNetworkManager::Connect() Begin");

	// Disconnect if we're already connected
	if (g_ConnectionState == CONSTATE_COND)
		Disconnect();

	// Set our last connection so we can connect again later and we set our state to connecting
	SetLastConnection(ip, port);
	g_ConnectionState = CONSTATE_CONN;

	CLogger::getInstance()->debug("CNetworkManager::Connect() to server %s:%d", ip, port);

	// Connect to the server, leaving the result
	int result = m_pRakPeer->Connect(ip, port, NULL, 0);

	// Check if connection failed, then set our state to failed
	if (result != 0)
	{
		g_ConnectionState = CONSTATE_FAIL;
		switch ((RakNet::ConnectionAttemptResult)result)
		{
			case RakNet::ConnectionAttemptResult::ALREADY_CONNECTED_TO_ENDPOINT :
				CLogger::getInstance()->error("CNetworkManager::Connect() Failed to connect, errorcode: ALREADY CONNECTED TO ENDPOINT");
				break;
			case RakNet::ConnectionAttemptResult::CANNOT_RESOLVE_DOMAIN_NAME:
				CLogger::getInstance()->error("CNetworkManager::Connect() Failed to connect, errorcode: CANNOT RESOLVE DOMAIN NAME");
				break;
			case RakNet::ConnectionAttemptResult::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS:
				CLogger::getInstance()->error("CNetworkManager::Connect() Failed to connect, errorcode: CONNECTION ATTEMPT ALREADY IN PROGRESS");
				break;
			case RakNet::ConnectionAttemptResult::INVALID_PARAMETER:
				CLogger::getInstance()->error("CNetworkManager::Connect() Failed to connect, errorcode: INVALID PARAMETER");
				break;
			case RakNet::ConnectionAttemptResult::SECURITY_INITIALIZATION_FAILED:
				CLogger::getInstance()->error("CNetworkManager::Connect() Failed to connect, errorcode: SECURITY INITIALIZATION FAILED");
				break;
		}
		return;
	}

	LOG_DEBUG("CNetworkManager::Connect() End");
}

void CNetworkManager::Disconnect(bool shutdown)
{
	LOG_DEBUG("CNetworkManager::Disconnect() Begin");

	// Don't do anything if we're not connected
	if (g_ConnectionState == CONSTATE_DISC)
		return;

	// Stop RakPeer from accepting anymore incoming packets
	m_pRakPeer->CloseConnection(m_SystemAddr, true);

	// Set our state to disconnected
	g_ConnectionState = CONSTATE_DISC;
	m_bValidHandshake = false;

	// Clean the server GUID
	m_SystemAddr = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	// Reinitialize our RakPeerInterface
	Stop();
	LOG_DEBUG("CNetworkManager::Disconnect() Restarting");
	if (!shutdown)
	{
		if (!Start())
			LOG_DEBUG("CNetworkManager::Disconnect() Restart Failed");
		else
			LOG_DEBUG("CNetworkManager::Disconnect() Restarted");
	}

	LOG_DEBUG("CNetworkManager::Disconnect() End");
}

void CNetworkManager::SendHandshake()
{
	LOG_DEBUG("CNetworkManager::SendHandshake() Begin");

	// Handshake Struct
	HANDSHAKE hs{};
	hs.rttVersion = RTT_VERSION,
		hs.fps = 0;
	hs.useData[SMEM_DATA::F4] = 0;
	hs.useData[SMEM_DATA::BMS] = 0;
	hs.useData[SMEM_DATA::OSB] = 0;
	hs.useData[SMEM_DATA::IVIBE] = 0;
	hs.useDisp[RTT_DISP::HUD] = 0;
	hs.useDisp[RTT_DISP::PFL] = 0;
	hs.useDisp[RTT_DISP::DED] = 0;
	hs.useDisp[RTT_DISP::RWR] = 0;
	hs.useDisp[RTT_DISP::MFDLEFT] = 1;
	hs.useDisp[RTT_DISP::MFDRIGHT] = 1;
	hs.useDisp[RTT_DISP::HMS] = 0;

	RakNet::BitStream bitstream;
	bitstream.Write((unsigned char)MSG_HANDSHAKE);
	bitstream.Write(hs.rttVersion);
	bitstream.Write(hs.fps);
	bitstream.Write(hs.useData[SMEM_DATA::F4]);
	bitstream.Write(hs.useData[SMEM_DATA::BMS]);
	bitstream.Write(hs.useData[SMEM_DATA::OSB]);
	bitstream.Write(hs.useData[SMEM_DATA::IVIBE]);
	bitstream.Write(hs.useDisp[RTT_DISP::HUD]);
	bitstream.Write(hs.useDisp[RTT_DISP::PFL]);
	bitstream.Write(hs.useDisp[RTT_DISP::DED]);
	bitstream.Write(hs.useDisp[RTT_DISP::RWR]);
	bitstream.Write(hs.useDisp[RTT_DISP::MFDLEFT]);
	bitstream.Write(hs.useDisp[RTT_DISP::MFDRIGHT]);
	bitstream.Write(hs.useDisp[RTT_DISP::HMS]);

	CNetworkManager::GetInterface()->Send(&bitstream, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, CNetworkManager::GetSystemAddress(), false);

	LOG_DEBUG("CNetworkManager::SendHandshake() End");
}

void CNetworkManager::Pulse()
{
	// Don't do anything if we're disconnected
	if (g_ConnectionState == CONSTATE_DISC || g_ConnectionState == CONSTATE_FAIL)
		return;

	RakNet::Packet* g_Packet = NULL;

	while (g_Packet = m_pRakPeer->Receive())
	{
		CLogger::getInstance()->debug("CNetworkManager::Pulse() Packet Received : %d", g_Packet->data[0]);

		RakNet::BitStream g_BitStream(g_Packet->data + 1, g_Packet->length + 1, false);

		switch (g_Packet->data[0])
		{
			case ID_UNCONNECTED_PONG:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_UNCONNECTED_PONG Begin");
				LOG_DEBUG("CNetworkManager::Pulse() ID_UNCONNECTED_PONG End");
				break;
			}
			case ID_ADVERTISE_SYSTEM:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_ADVERTISE_SYSTEM Begin");
				LOG_DEBUG("CNetworkManager::Pulse() ID_ADVERTISE_SYSTEM End");
				break;
			}
			case ID_DOWNLOAD_PROGRESS:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_DOWNLOAD_PROGRESS Begin");
				LOG_DEBUG("CNetworkManager::Pulse() ID_DOWNLOAD_PROGRESS End");
				break;
			}
			case ID_IP_RECENTLY_CONNECTED:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_IP_RECENTLY_CONNECTED Begin");
				CLogger::getInstance()->error("CNetworkManager::Pulse() Failed to connect, recently connected", g_Packet->data[0]);
				m_pRakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_IP_RECENTLY_CONNECTED End");
				return;
			}
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_INCOMPATIBLE_PROTOCOL_VERSION Begin");
				CLogger::getInstance()->error("CNetworkManager::Pulse() Failed to connect, incompatible protocol version", g_Packet->data[0]);
				m_pRakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_INCOMPATIBLE_PROTOCOL_VERSION End");
				return;
			}
			case ID_ALREADY_CONNECTED:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_ALREADY_CONNECTED Begin");
				CLogger::getInstance()->error("CNetworkManager::Pulse() Failed to connect, already connected", g_Packet->data[0]);
				m_pRakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_ALREADY_CONNECTED End");
				return;
			}
			case ID_NO_FREE_INCOMING_CONNECTIONS:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_NO_FREE_INCOMING_CONNECTIONS Begin");
				CLogger::getInstance()->error("CNetworkManager::Pulse() Failed to connect, max client", g_Packet->data[0]);
				m_pRakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_NO_FREE_INCOMING_CONNECTIONS End");
				return;
			}
			case ID_INVALID_PASSWORD:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_INVALID_PASSWORD Begin");
				CLogger::getInstance()->error("CNetworkManager::Pulse() Failed to connect, invalid password", g_Packet->data[0]);
				m_pRakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_INVALID_PASSWORD End");
				return;
			}
			case ID_CONNECTION_ATTEMPT_FAILED:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_ATTEMPT_FAILED Begin");
				CLogger::getInstance()->error("CNetworkManager::Pulse() Failed to connect, server not responding. %d", g_Packet->data[0]);
				m_pRakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_ATTEMPT_FAILED End");
				return;
			}
			case ID_CONNECTION_BANNED:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_BANNED Begin");
				CLogger::getInstance()->error("CNetworkManager::Pulse() Failed to connect, banned", g_Packet->data[0]);
				m_pRakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_BANNED End");
				return;
			}
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_REQUEST_ACCEPTED Begin");
				// Set the server Adress
				m_SystemAddr = g_Packet->systemAddress;

				SendHandshake();
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_REQUEST_ACCEPTED End");
				break;
			}
			case ID_DISCONNECTION_NOTIFICATION:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_DISCONNECTION_NOTIFICATION Begin");
				m_pRakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_DISCONNECTION_NOTIFICATION End");
				return;
			}
			case ID_CONNECTION_LOST:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_LOST Begin");
				m_pRakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_LOST End");
				return;
			}
			case MSG_CONNECT:
			{
				LOG_DEBUG("CNetworkManager::Pulse() MSG_CONNECT Begin");
				LOG_DEBUG("CNetworkManager::Pulse() MSG_CONNECT End");
				break;
			}
			case MSG_DISCONNECT:
			{
				LOG_DEBUG("CNetworkManager::Pulse() MSG_DISCONNECT Begin");
				LOG_DEBUG("CNetworkManager::Pulse() MSG_DISCONNECT End");
				break;
			}
			case MSG_HANDSHAKE:
			{
				LOG_DEBUG("CNetworkManager::Pulse() MSG_HANDSHAKE Begin");

				RakNet::BitStream bitstream(g_Packet->data + 1, g_Packet->length - 1, false);
				unsigned char value;
				bitstream.Read(value);
				bitstream.Read(value);
				m_iFps = value;
				unsigned char dataValues[DATA_NUM];
				bitstream.ReadBits(dataValues, (unsigned int)DATA_NUM, false);
				for (int i = 0; i < DATA_NUM; i++)
					if (dataValues[i] > 0)
						m_bValidHandshake = true;

				if (!m_bValidHandshake)
				{
					unsigned char dispValues[DISP_NUM];
					bitstream.ReadBits(dispValues, (unsigned int)DISP_NUM, false);

					for (int i = 0; i < DISP_NUM; i++)
						if (dispValues[i] > 0)
							m_bValidHandshake = true;
				}

				if (!m_bValidHandshake)
				{
					LOG_ERROR("CNetworkManager::Pulse() RTT Handshake invalid");
					m_pRakPeer->DeallocatePacket(g_Packet);
					CNetworkManager::Disconnect();
					return;
				}
				else
				{
					LOG_DEBUG("CNetworkManager::Pulse() RTT Handshake completed");
					g_ConnectionState = CONSTATE_COND;
				}
				LOG_DEBUG("CNetworkManager::Pulse() MSG_HANDSHAKE End");
				break;
			}
			case MSG_IMAGE:
			{
				LOG_DEBUG("CNetworkManager::Pulse() MSG_IMAGE Begin");
				RakNet::BitStream bitstream(g_Packet->data + 1, g_Packet->length - 1, false);

				LOG_DEBUG("CNetworkManager::Pulse() MSG_IMAGE End");
				break;
			}
			case MSG_DATA:
			{
				LOG_DEBUG("CNetworkManager::Pulse() MSG_DATA Begin");
				LOG_DEBUG("CNetworkManager::Pulse() MSG_DATA End");
				break;
			}
		}
		m_pRakPeer->DeallocatePacket(g_Packet);

		LOG_DEBUG("CNetworkManager::Pulse() Done Receiving Packages");
	}
}

int CNetworkManager::GetFPS()
{
	if (m_bValidHandshake)
		return m_iFps;

	return 0;
}