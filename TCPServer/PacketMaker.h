#pragma once

#include "Packet.h"
#include <utility>
#include <string>
using namespace std;

class PacketMaker
{
protected:
	static const int DefaultBufferSize = 4;

public:
	// Use this PacketMaker if does not have params
	static pair<char*, int> MakePacket(EPacket Type);

	// const char* params
	static pair<char*, int> MakePacket(EPacket Type, const char* NewData);

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
