#pragma once

#define RTT_VERSION 33U // v3.3

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
