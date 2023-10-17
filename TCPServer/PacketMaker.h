#pragma once

#include "Packet.h"
using namespace std;

class PacketMaker
{
protected:
	static const int DefaultBufferSize = 4;

public:
	// Use this PacketMaker if does not have params
	static pair<char*, int> MakeDefaultPacket(EPacket Type);

public:


protected:
	static char* MakeHeader(char* Buffer, EPacket Type, unsigned short Size);
};

class UserData
{
public:
	UserData() {}
	UserData(char* NewUserID)
	{
		UserID = NewUserID;
	}

	//int UserNumber;

	char* UserID;
	char* NickName;

};

