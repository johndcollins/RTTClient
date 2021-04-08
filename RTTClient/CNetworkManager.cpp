#include "CNetworkManager.h"
#include "CLogger.h"

#include <iostream>
#include <chrono>
#include <thread>

using namespace RTTClient::Common;

CNetworkManager::CNetworkManager()
{
	LOG_DEBUG("CNetworkManager::CNetworkManager Start");
	Initialize();
	LOG_DEBUG("CNetworkManager::CNetworkManager End");
};

CNetworkManager::~CNetworkManager()
{
	LOG_DEBUG("CNetworkManager::~CNetworkManager Start");
	Destroy();
	LOG_DEBUG("CNetworkManager::~CNetworkManager End");
};


void CNetworkManager::Initialize()
{
	LOG_DEBUG("CNetworkManager::Initialize() Start");

	// Get RakPeerInterface
	g_RakPeer = RakNet::RakPeerInterface::GetInstance();

	LOG_DEBUG("CNetworkManager::Constructed");

	g_ConnectionState = CONSTATE_DISC;
	g_validHandshake = false;

	LOG_DEBUG("CNetworkManager::Initialize() End");
}

void CNetworkManager::Destroy()
{
	LOG_DEBUG("CNetworkManager::Destroy() Start");

	// Stop RakNet, stops synchronization
	g_RakPeer->Shutdown(2000);

	LOG_DEBUG("CNetworkManager::Destroy() Stop");
}

bool CNetworkManager::Start()
{
	LOG_DEBUG("CNetworkManager::Start() Start");

	LOG_DEBUG("CNetworkManager::Start() RakNet Startup");
	// Return whether Startup worked or not
	RakNet::SocketDescriptor desc = RakNet::SocketDescriptor();
	int result = g_RakPeer->Startup(1, &desc, 1, SCHED_RR);
	
	LOG_DEBUG("CNetworkManager::Start() Stop");

	return (result == RakNet::RAKNET_STARTED);
}

void CNetworkManager::Stop()
{
	LOG_DEBUG("CNetworkManager::Stop() Start");

	// Disconnect if we're connected
	if (g_ConnectionState == CONSTATE_COND)
		Disconnect();

	LOG_DEBUG("CNetworkManager::Shutting down RakNet");
	// Stop our RakPeerInterface
	g_RakPeer->Shutdown(50);
	//sleep(15);
	LOG_DEBUG("CNetworkManager::Stop() RakNet shut down.");

	LOG_DEBUG("CNetworkManager::Stop() Stop");
}

void CNetworkManager::Connect(const char* ip, int port)
{
	LOG_DEBUG("CNetworkManager::Connect() Start");

	// Disconnect if we're already connected
	if (g_ConnectionState == CONSTATE_COND)
		Disconnect();

	// Set our last connection so we can connect again later and we set our state to connecting
	SetLastConnection(ip, "", port);
	g_ConnectionState = CONSTATE_CONN;

	CLogger::getInstance()->debug("CNetworkManager::Connect to server %s:%d", ip, port);

	// Connect to the server, leaving the result
	int result = g_RakPeer->Connect(ip, port, NULL, 0);

	// Check if connection failed, then set our state to failed
	if (result != 0)
	{
		g_ConnectionState = CONSTATE_FAIL;
		switch ((RakNet::ConnectionAttemptResult)result)
		{
			case RakNet::ConnectionAttemptResult::ALREADY_CONNECTED_TO_ENDPOINT :
				CLogger::getInstance()->error("Failed to connect, errorcode: ALREADY CONNECTED TO ENDPOINT");
				break;
			case RakNet::ConnectionAttemptResult::CANNOT_RESOLVE_DOMAIN_NAME:
				CLogger::getInstance()->error("Failed to connect, errorcode: CANNOT RESOLVE DOMAIN NAME");
				break;
			case RakNet::ConnectionAttemptResult::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS:
				CLogger::getInstance()->error("Failed to connect, errorcode: CONNECTION ATTEMPT ALREADY IN PROGRESS");
				break;
			case RakNet::ConnectionAttemptResult::INVALID_PARAMETER:
				CLogger::getInstance()->error("Failed to connect, errorcode: INVALID PARAMETER");
				break;
			case RakNet::ConnectionAttemptResult::SECURITY_INITIALIZATION_FAILED:
				CLogger::getInstance()->error("Failed to connect, errorcode: SECURITY INITIALIZATION FAILED");
				break;
		}

		SendHandshake();

		return;
	}

	LOG_DEBUG("CNetworkManager::Connect() End");
}

bool CNetworkManager::FinalizeConnection()
{
	//LOG_DEBUG("CNetworkManager::FinalizeConnection() Start");

	if (g_ConnectionState == CONSTATE_COND)
		return true;

	g_SystemAddr = RakNet::SystemAddress(g_lastIP.c_str(), g_lastPort);
	int result = g_RakPeer->GetConnectionState(g_SystemAddr);
	switch (result)
	{
		case RakNet::ConnectionState::IS_CONNECTED:
			SendHandshake();
			//LOG_DEBUG("CNetworkManager::FinalizeConnection() Connected End");
			return true;
		default:
			//LOG_DEBUG("CNetworkManager::FinalizeConnection() Not Connected End");
			return false;
	};
};

void CNetworkManager::Disconnect()
{
	LOG_DEBUG("CNetworkManager::Disconnect() Start");

	// Don't do anything if we're not connected
	if (g_ConnectionState == CONSTATE_DISC)
		return;

	// Stop RakPeer from accepting anymore incoming packets
	g_RakPeer->CloseConnection(g_SystemAddr, true);

	// Set our state to disconnected
	g_ConnectionState = CONSTATE_DISC;
	g_validHandshake = false;

	// Clean the server GUID
	g_SystemAddr = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	// Reinitialize our RakPeerInterface
	Stop();
	LOG_DEBUG("CNetworkManager::Disconnect() Restarting");
	if (!Start())
		LOG_DEBUG("CNetworkManager::Disconnect() Restart Failed");
	else
		LOG_DEBUG("CNetworkManager::Disconnect() Restarted");

	LOG_DEBUG("CNetworkManager::Disconnect() End");
}

void CNetworkManager::SendHandshake()
{
	//LOG_DEBUG("CNetworkManager::SendHandshake() Start");

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

	//LOG_DEBUG("CNetworkManager::SendHandshake() End");
}

void CNetworkManager::Pulse()
{
	// Don't do anything if we're disconnected
	if (g_ConnectionState == CONSTATE_DISC || g_ConnectionState == CONSTATE_FAIL)
		return;

	RakNet::Packet* g_Packet = NULL;

	while (g_Packet = g_RakPeer->Receive())
	{
		RakNet::BitStream g_BitStream(g_Packet->data + 1, g_Packet->length + 1, false);

		switch (g_Packet->data[0])
		{
			case ID_UNCONNECTED_PONG:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_UNCONNECTED_PONG Start");
				LOG_DEBUG("CNetworkManager::Pulse() ID_UNCONNECTED_PONG End");
				break;
			}
			case ID_ADVERTISE_SYSTEM:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_ADVERTISE_SYSTEM Start");
				LOG_DEBUG("CNetworkManager::Pulse() ID_ADVERTISE_SYSTEM End");
				break;
			}
			case ID_DOWNLOAD_PROGRESS:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_DOWNLOAD_PROGRESS Start");
				LOG_DEBUG("CNetworkManager::Pulse() ID_DOWNLOAD_PROGRESS End");
				break;
			}
			case ID_IP_RECENTLY_CONNECTED:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_IP_RECENTLY_CONNECTED Start");
				CLogger::getInstance()->error("Failed to connect, recently connected", g_Packet->data[0]);
				g_RakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_IP_RECENTLY_CONNECTED End");
				return;
			}
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_INCOMPATIBLE_PROTOCOL_VERSION Start");
				CLogger::getInstance()->error("Failed to connect, incompatible protocol version", g_Packet->data[0]);
				g_RakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_INCOMPATIBLE_PROTOCOL_VERSION End");
				return;
			}
			case ID_ALREADY_CONNECTED:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_ALREADY_CONNECTED Start");
				CLogger::getInstance()->error("Failed to connect, already connected", g_Packet->data[0]);
				g_RakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_ALREADY_CONNECTED End");
				return;
			}
			case ID_NO_FREE_INCOMING_CONNECTIONS:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_NO_FREE_INCOMING_CONNECTIONS Start");
				CLogger::getInstance()->error("Failed to connect, max client", g_Packet->data[0]);
				g_RakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_NO_FREE_INCOMING_CONNECTIONS End");
				return;
			}
			case ID_INVALID_PASSWORD:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_INVALID_PASSWORD Start");
				CLogger::getInstance()->error("Failed to connect, invalid password", g_Packet->data[0]);
				g_RakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_INVALID_PASSWORD End");
				return;
			}
			case ID_CONNECTION_ATTEMPT_FAILED:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_ATTEMPT_FAILED Start");
				CLogger::getInstance()->error("Failed to connect, server not responding. %d", g_Packet->data[0]);
				g_RakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_ATTEMPT_FAILED End");
				return;
			}
			case ID_CONNECTION_BANNED:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_BANNED Start");
				CLogger::getInstance()->error("Failed to connect, banned", g_Packet->data[0]);
				g_RakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_BANNED End");
				return;
			}
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_REQUEST_ACCEPTED Start");
				
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_REQUEST_ACCEPTED End");
				break;
			}
			case ID_DISCONNECTION_NOTIFICATION:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_DISCONNECTION_NOTIFICATION Start");
				g_RakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_DISCONNECTION_NOTIFICATION End");
				return;
			}
			case ID_CONNECTION_LOST:
			{
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_LOST Start");
				g_RakPeer->DeallocatePacket(g_Packet);
				CNetworkManager::Disconnect();
				LOG_DEBUG("CNetworkManager::Pulse() ID_CONNECTION_LOST End");
				return;
			}
			case MSG_CONNECT:
			{
				LOG_DEBUG("CNetworkManager::Pulse() MSG_CONNECT Start");
				LOG_DEBUG("CNetworkManager::Pulse() MSG_CONNECT End");
				break;
			}
			case MSG_DISCONNECT:
			{
				LOG_DEBUG("CNetworkManager::Pulse() MSG_DISCONNECT Start");
				LOG_DEBUG("CNetworkManager::Pulse() MSG_DISCONNECT End");
				break;
			}
			case MSG_HANDSHAKE:
			{
				LOG_DEBUG("CNetworkManager::Pulse() MSG_HANDSHAKE Start");

				RakNet::BitStream bitstream(g_Packet->data + 1, g_Packet->length + 1, false);
				unsigned char value;
				bitstream.Read(value);
				bitstream.Read(value);
				unsigned char dataValues[DATA_NUM];
				bitstream.ReadBits(dataValues, (unsigned int)DATA_NUM, false);
				for (int i = 0; i < DATA_NUM; i++)
					if (dataValues[i] > 0)
						g_validHandshake = true;

				if (!g_validHandshake)
				{
					unsigned char dispValues[DISP_NUM];
					bitstream.ReadBits(dispValues, (unsigned int)DISP_NUM, false);

					for (int i = 0; i < DISP_NUM; i++)
						if (dispValues[i] > 0)
							g_validHandshake = true;
				}

				if (!g_validHandshake)
				{
					LOG_ERROR("RTT Handshake invalid");
					g_RakPeer->DeallocatePacket(g_Packet);
					CNetworkManager::Disconnect();
					return;
				}
				else
				{
					LOG_INFO("RTT Handshake completed");
					g_ConnectionState = CONSTATE_COND;
				}
				LOG_DEBUG("MSG_HANDSHAKE End");
				break;
			}
			case MSG_IMAGE:
			{
				LOG_DEBUG("CNetworkManager::Pulse() MSG_IMAGE Start");
				if (g_validHandshake)
				{

				}
				LOG_DEBUG("CNetworkManager::Pulse() MSG_IMAGE End");
				break;
			}
			case MSG_DATA:
			{
				LOG_DEBUG("CNetworkManager::Pulse() MSG_DATA Start");
				if (g_validHandshake)
				{

				}
				LOG_DEBUG("CNetworkManager::Pulse() MSG_DATA End");
				break;
			}

			CLogger::getInstance()->debug(" Packet Received : %d", g_Packet->data[0]);
		}
		g_RakPeer->DeallocatePacket(g_Packet);

		LOG_DEBUG("Done Receiving Packages");
	}
}