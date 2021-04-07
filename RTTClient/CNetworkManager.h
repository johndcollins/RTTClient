#pragma once

#include <iostream>

#include <RakNetTypes.h>
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

// Connection Status
#define CONSTATE_DISC	0
#define CONSTATE_CONN	1
#define CONSTATE_COND	2
#define CONSTATE_FAIL	3

#define RTT_VERSION 33U // v3.3

class CNetworkManager
{
public:
	CNetworkManager();
	virtual ~CNetworkManager();

	bool Start();
	void Stop();

	void Connect(const char* ip, const char* pass, int port);
	void Disconnect();

	void Pulse();

	void SetLastConnection(const char* ip, const char* pass, int port) { g_lastIP = ip; g_lastPass = pass; g_lastPort = port; }

	RakNet::RakPeerInterface* GetInterface() { return g_RakPeer; }
	RakNet::SystemAddress GetSystemAddress() { return g_SystemAddr; }

	int g_ConnectionState;

private:
	// RakNet Interfaces
	RakNet::RakPeerInterface* g_RakPeer;

	/* Previous/Current connection */
	std::string		g_lastIP;
	std::string		g_lastPass;
	int				g_lastPort;
	RakNet::SystemAddress	g_SystemAddr;
	bool g_validHandshake;

	void Initialize();
	void Destroy();

#pragma pack(push,1)
	enum MESSAGE_TYPE : unsigned char
	{
		MSG_CONNECT = 134U, //RakNet ID_USER_PACKET_ENUM
		MSG_DISCONNECT,
		MSG_HANDSHAKE,
		MSG_IMAGE,
		MSG_DATA
	};

	enum SMEM_DATA : unsigned char
	{
		F4 = 0U, //FalconSharedMemoryArea (FlightData)
		BMS,     //FalconSharedMemoryArea2 (FlightData2)
		OSB,     //FalconSharedOsbMemoryArea (OSBData)
		IVIBE,   //FalconIntellivibeSharedMemoryArea (IntellivibeData)
		DATA_NUM
	};


	enum RTT_DISP : unsigned char
	{
		HUD = 0U,
		PFL,
		DED,
		RWR,
		MFDLEFT,
		MFDRIGHT,
		HMS,
		DISP_NUM
	};


	enum RCV_MODE : unsigned char
	{
		PNG = 0U,
		JPG,
		LZ4,
		RAW,
		MODE_NUM
	};


	struct HANDSHAKE
	{
		unsigned char rttVersion;
		unsigned char fps;
		unsigned char useData[DATA_NUM]; // 0 = false, other = true
		unsigned char useDisp[DISP_NUM]; // 0 = false, other = true
	};


	struct RTT_HEADER
	{
		RCV_MODE mode;
		RTT_DISP disp;
		unsigned short width;
		unsigned short height;
		unsigned long size;
	};


	struct COLOR_RGBA
	{
		unsigned char r, g, b, a;

		COLOR_RGBA(unsigned char r = 0U, unsigned char g = 0U, unsigned char b = 0U, unsigned char a = 255U) :
			r(r), g(g), b(b), a(a) {}
	};
#pragma pack(pop)
};
