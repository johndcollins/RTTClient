/* Dunc, January 2021, on behalf of Benchmark Sims
 *
 *
 * Upfront note: in *addition* to the regular RTTServer/RTTClient network
 * protocol described here, *custom* remote shared men clients may also receive
 * the data by means of UDP multicast on plain sockets independent of RakNet.
 * Please see the "MULTICAST.txt" document for further information.
 *
 *
 * This document defines/describes the network message format that RTTServer is
 * using to communicate with RTTClients. You can use this information to build
 * your own RTTClient receiver applications.
 *
 * The transport layer is implemented on top of RakNet 4.081, i.e. your client
 * must use RakNet to implement the network connectivity. The official source
 * repository is located here:
 * https://github.com/facebookarchive/RakNet
 *
 * However, it is strongly advised to use the "larku" fork as the basis for your
 * implementation, as it includes the most pressing bug fixes:
 * https://github.com/larku/RakNet
 *
 *
 * The following options need to be set in "RakNetDefinesOverrides.h" to make it
 * compatible at with RTTServer at compile time:
 *
 * #define RAKNET_SUPPORT_IPV6 1
 * #define USE_SLIDING_WINDOW_CONGESTION_CONTROL 0
 *
 *
 * Connect/disconnect handling should be purely handled by RakNet means, there
 * is no further RTT specific message handling needed (i.e. you can ignore the
 * message types MSG_CONNECT and MSG_DISCONNECT in the enum below, they are only
 * used internally in the native RTTServer/RTTClient).
 *
 *
 * After the initial network connection is established, the message flow is as
 * follows:
 *
 * 1) The client sends an initial MSG_HANDSHAKE message to tell the server what
 *    data exactly it *requests*.
 * 
 * 2) The server will send back an MSG_HANDSHAKE message with the *actual* data
 *    that it will send to the client during the session.
 *
 * 3) The server will send out MSG_IMAGE (RTT) and/or MSG_DATA (SharedMem)
 *    messages to the client continuously once it detects that BMS is providing
 *    the data. This is the "main message loop" (albeit one-way only). Note that
 *    IMAGE/DATA messages are sent unreliable, but ordered. That means that
 *    while it is possible for the server to skip sending out messages (e.g. due
 *    to network congestion), it is nevertheless ensured that each message the
 *    client receives is always *newer* than the previous one.
 *
 * 4) If a client disconnects or loses connection, it has to start out at step 1
 *    again to re-initiate the message flow.
 *
 *
 * Since RakNet is used as the transport layer, all messages you receive are of
 * type "RakNet::Packet". Each RN packet holds its payload in an array of type
 * "unsigned char", called "data", which you must evaluate and handle
 * accordingly (this is not RTTServer/Client specific, but generic RakNet
 * functionality).
 *
 * In the following, I will describe the various data contents for each message
 * type. Note that I will only identify the *starting* fields in the data array,
 * you can deduce the length of the data from the data types itself.
 *
 * RakNet stores the message type in data[0]. It uses a sizable number of
 * internal message types that get evaluated by RN automatically. Other internal
 * RN message types need to be handled by *you* (e.g. connect/disconnect/already
 * connected/request accepted/denied...) in order to enable your client to track
 * the connection status to the server from a *business* point of view. Check
 * out the various RakNet examples in case you need more info on this.
 *
 * (Note: you will never see an actual IP address in our payload "data". If you
 * want to know where a packet came from, you need to evaluate the
 * "systemAddress" field in the "RakNet::Packet".)
 *
 * Last not least, it can hold custom message types, in our case MSG_HANDSHAKE,
 * MSG_DATA, MSG_IMAGE.
 *
 * 
 * MSG_HANDSHAKE messages are composed as follows:
 *
 * data[0] = MSG_HANDSHAKE
 * data[sizeof(MESSAGE_TYPE)] = <struct HANDSHAKE>
 *
 * The "struct HANDSHAKE" needs to hold the following info (check below):
 *
 * BYTE rttVersion = RTT_VERSION
 * (If there is a mismatch between the server version and the client version,
 * the server will not send out any data to the client.)
 *
 * BYTE fps = <how often will the IMAGE/DATA be sent?>
 * (The client does not need to specify a value here. It is the server who
 * provides this info to the client in its MSG_HANDSHAKE response. This info can
 * be used by the client program to optimize message polling and or render
 * update cycles.)
 *
 * BYTE useData[DATA_NUM] = <which sharedmem areas are requested by the client?>
 * (The client needs to set a non-0 value to the fields in the array that
 * correspond to the data areas it wants to receive. Check the "enum SMEM_DATA"
 * below.)
 *
 * BYTE useDisp[DISP_NUM] = <which RTT images are requested by the client?>
 * (The client needs to set a non-0 value to the fields in the array that
 * correspond to the RTT images it wants to receive. Check the "enum RTT_DISP"
 * below.)
 *
 * Note that the MSG_HANDSHAKE response message from the server might send back
 * different info than originally set by the client. This is to inform the
 * client about the actual data it will be sending. For example, if the server
 * detects a version mismatch, it will set ALL useData/useDisp fields to 0,
 * because it will not send out any data at all.
 *
 *
 * MSG_DATA messages are composed as follows:
 * 
 * data[0] = MSG_DATA
 * data[sizeof(MESSAGE_TYPE)] = <which SharedMem area are we sending?>
 * (Check the "enum SMEM_DATA" below for possible values.)
 *
 * data[sizeof(MESSAGE_TYPE) + sizeof(SMEM_DATA)] = <the actual SharedMem data>
 * (The actual SharedMem object names, their composition and sizes are defined
 * in the "FlightData.h" file in the BMS folder, e.g. in
 * "<BMS>\Tools\SharedMem". If you use the C/C++ object names, that would be
 * FlightData, FlightData2, OSBData, IntellivibeData.)
 *
 *
 * MSG_IMAGE messages are composed as follows:
 *
 * data[0] = MSG_IMAGE
 * data[sizeof(MESSAGE_TYPE)] = <struct RTT_HEADER>
 * data[sizeof(MESSAGE_TYPE)+sizeof(RTT_HEADER)] = <the actual image data>
 * 
 * The "struct RTT_HEADER" needs to hold the following info (check below):
 *
 * RCV_MODE mode = <which image type is this?>
 * (Check the "enum RCV_MODE" below for possible values. The payload data after
 * RTT_HEADER will be binary data in the format specified here.)
 *
 * RTT_DISP disp = <which RTT image is this?>
 * (Check the "enum RTT_DISP" below for possible values.)
 * 
 * WORD width = <the resolution width of the image>
 * WORD height = <the resolution height of the image>
 * 
 * DWORD size = <the size of the binary image data after RTT_HEADER in bytes>
 *
 * Note that for RCV_MODE "JPG" and "PNG", the binary data is just that, a
 * "proper" JPG or PNG image. It is up to your client to decode/render that
 * image.
 *
 * For RCV_MODE "RAW", you will receive a continuous array of individual "struct
 * COLOR_RGBA" (8 bit per color/alpha, see below) values, each image line simply
 * concatenated to the former one. So you need to chop them up according to the
 * RTT_HEADER "width" and reconstruct/render the image from the individual
 * COLOR_RGBA values (one value per pixel).
 *
 * For RCV_MODE "LZ4", you also will receive a continuous array of individual
 * "struct COLOR_RGBA" values like the "RAW" option, however, the data is
 * compressed with LZ4 and hence you need to unpack it before handling it in the
 * same way as you handled "RAW".
 *
 * 
 * ...and that's all there is to it. How you actually use the received
 * image/shared mem data is completely up to your client application and your
 * imagination.
 *
 * I hope this info is helpful and concise enough to get you going.
 *
 * Cheers,
 * Dunc
 */


#pragma once


//#define RTT_VERSION 33U // v3.3

/*
// data structures within "pragma pack" will be serialized over the wire
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


enum RTT_DISP : BYTE
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


enum RCV_MODE : BYTE
{
	PNG = 0U,
	JPG,
	LZ4,
	RAW,
	MODE_NUM
};


struct HANDSHAKE
{
	BYTE rttVersion;
	BYTE fps;
	BYTE useData[DATA_NUM]; // 0 = false, other = true
	BYTE useDisp[DISP_NUM]; // 0 = false, other = true
};


struct RTT_HEADER
{
	RCV_MODE mode;
	RTT_DISP disp;
	WORD     width;
	WORD     height;
	DWORD    size;
};


struct COLOR_RGBA
{
	BYTE r, g, b, a;

	COLOR_RGBA(BYTE r = 0U, BYTE g = 0U, BYTE b = 0U, BYTE a = 255U) :
		r(r), g(g), b(b), a(a) {}
};


#pragma pack(pop)
*/