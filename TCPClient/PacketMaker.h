#pragma once

#include "Packet.h"
#include <utility>
using namespace std;

#include <WinSock2.h>

class PacketMaker
{
protected:
	static const int DefaultBufferSize = 4;

public:
	static bool SendPacket(SOCKET* ServerSocket, const EPacket& PacketToSend);

	static bool SendPacket(SOCKET* ServerSocket, const EPacket& PacketToSend, const char* MessageToSend);

protected:
	// Use this PacketMaker if does not have params
	static pair<char*, int> MakePacket(EPacket Type);

	// const char* params
	static pair<char*, int> MakePacket(EPacket Type, const char* NewData);

protected:
	static char* MakeHeader(char* Buffer, EPacket Type, unsigned short Size);
};
