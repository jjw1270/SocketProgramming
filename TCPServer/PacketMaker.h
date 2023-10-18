#pragma once

#include "Packet.h"
#include <utility>
#include <string>
using namespace std;

#include <WinSock2.h>

class PacketMaker
{
protected:
	static const int DefaultBufferSize = 4;

public:
	static bool SendPacket(SOCKET* ClientSocket, EPacket PacketToSend);

	static bool SendPacket(SOCKET* ClientSocket, EPacket PacketToSend, const char* MessageToSend);

	static bool SendPacketToAllConnectedClients(const fd_set& Reads, EPacket PacketToSend);

protected:
	// Use this PacketMaker if does not have params
	static pair<char*, int> MakeBuffer(EPacket Type);

	// const char* params
	static pair<char*, int> MakeBuffer(EPacket Type, const char* NewData);

protected:
	static char* MakeHeader(char* Buffer, EPacket Type, unsigned short Size);
};

class UserData
{
public:
	UserData() {}
	UserData(const string& NewUserID)
	{
		UserID = NewUserID;
	}

	string UserID;
	string NickName;
};
