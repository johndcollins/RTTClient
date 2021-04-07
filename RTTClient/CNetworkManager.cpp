#include "CNetworkManager.h"

CNetworkManager::CNetworkManager()
{
	Initialize();
};

CNetworkManager::~CNetworkManager()
{
	Destroy();
};


void CNetworkManager::Initialize()
{
	// Get RakPeerInterface
	g_RakPeer = RakNet::RakPeerInterface::GetInstance();

	//Logger::Msg("CNetworkManager::Constructed");

	g_ConnectionState = CONSTATE_DISC;
	g_validHandshake = false;
}

void CNetworkManager::Destroy()
{
	// Stop RakNet, stops synchronization
	g_RakPeer->Shutdown(2000);

	//Logger::Msg("CNetworkManager::Deconstructed");
}

bool CNetworkManager::Start()
{
	//Logger::Msg("CNetworkManager::Started");

	// Return whether Startup worked or not
	RakNet::SocketDescriptor desc = RakNet::SocketDescriptor();
	return (g_RakPeer->Startup(1, &desc, 1, SCHED_RR /*, THREAD_PRIORITY_NORMAL*/) == RakNet::RAKNET_STARTED);
}

void CNetworkManager::Stop()
{
	// Disconnect if we're connected
	if (g_ConnectionState == CONSTATE_COND)
		Disconnect();

	// Stop our RakPeerInterface
	g_RakPeer->Shutdown(500);

	//Logger::Msg("CNetworkManager::Stopped");
}

void CNetworkManager::Connect(const char* ip, const char* pass, int port)
{
	// Disconnect if we're already connected
	if (g_ConnectionState == CONSTATE_COND)
		Disconnect();

	// Set our last connection so we can connect again later and we set our state to connecting
	SetLastConnection(ip, pass, port);
	g_ConnectionState = CONSTATE_CONN;

	// Connect to the server, leaving the result
	int Result = g_RakPeer->Connect(ip, port, pass, sizeof(pass));

	// Check if connection failed, then set our state to failed
	if (Result != 0)
	{
		g_ConnectionState = CONSTATE_FAIL;
		//Logger::Msg("Failed to connect, errorcode: %d", Result);
		return;
	}
}

void CNetworkManager::Disconnect()
{
	// Don't do anything if we're not connected
	if (g_ConnectionState == CONSTATE_DISC)
		return;

	// Stop RakPeer from accepting anymore incoming packets
	g_RakPeer->CloseConnection(g_SystemAddr, true);

	// Set our state to disconnected
	g_ConnectionState = CONSTATE_DISC;
	g_validHandshake = false;

	// Reinitialize our RakPeerInterface
	Stop();
	Start();

	// Clean the server GUID
	g_SystemAddr = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	//Logger::Msg("CNetworkManager::Disconnected");
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
				break;
			}
			case ID_ADVERTISE_SYSTEM:
			{
				break;
			}
			case ID_DOWNLOAD_PROGRESS:
			{
				break;
			}
			case ID_IP_RECENTLY_CONNECTED:
			{
				CNetworkManager::Disconnect();
				//Logger::Msg("Failed to connect, recently connected", g_Packet->data[0]);
				break;
			}
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			{
				CNetworkManager::Disconnect();
				//Logger::Msg("Failed to connect, incompatible protocol version", g_Packet->data[0]);
				break;
			}
			case ID_ALREADY_CONNECTED:
			{
				CNetworkManager::Disconnect();
				//Logger::Msg("Failed to connect, already connected", g_Packet->data[0]);
				break;
			}
			case ID_NO_FREE_INCOMING_CONNECTIONS:
			{
				CNetworkManager::Disconnect();
				//Logger::Msg("Failed to connect, max client", g_Packet->data[0]);
				break;
			}
			case ID_INVALID_PASSWORD:
			{
				CNetworkManager::Disconnect();
				//Logger::Msg("Failed to connect, invalid password", g_Packet->data[0]);
				break;
			}
			case ID_CONNECTION_ATTEMPT_FAILED:
			{
				CNetworkManager::Disconnect();
				//Logger::Msg("Failed to connect, server not responding", g_Packet->data[0]);
				break;
			}
			case ID_CONNECTION_BANNED:
			{
				CNetworkManager::Disconnect();
				//Logger::Msg("Failed to connect, banned", g_Packet->data[0]);
				break;
			}
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				// Set the server Adress
				g_SystemAddr = g_Packet->systemAddress;

				// Handshake Struct
				HANDSHAKE hs {};
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
				
				//Logger::Msg("CNetworkManager::Connected Send Handshake");
				break;
			}
			case ID_DISCONNECTION_NOTIFICATION:
			{
				CNetworkManager::Disconnect();
				//Logger::Msg("CNetworkManager::ConnectionClosed");
				break;
			}
			case ID_CONNECTION_LOST:
			{
				CNetworkManager::Disconnect();
				//Logger::Msg("CNetworkManager::ConnectionLost");
				break;
			}
			case MSG_CONNECT:
			{
				//Logger::Msg("RTT Connect");
				break;
			}
			case MSG_DISCONNECT:
			{
				//Logger::Msg("RTT Disconnect");
				break;
			}
			case MSG_HANDSHAKE:
			{
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
					//Logger::Msg("RTT Handshake invalid");
					CNetworkManager::Disconnect();
				}
				else
					//Logger::Msg("RTT Handshake completed");
				break;
			}
			case MSG_IMAGE:
			{
				if (g_validHandshake)
				{

				}
				break;
			}
			case MSG_DATA:
			{
				if (g_validHandshake)
				{

				}
				break;
			}

		//Logger::Msg("%d", g_Packet->data[0]);
		}
		g_RakPeer->DeallocatePacket(g_Packet);
	}
}