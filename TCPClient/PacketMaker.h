#pragma once

#include "Packet.h"
#include <utility>
#include <iostream>
using namespace std;

class PacketMaker
{
protected:
	static const int DefaultBufferSize = 4;

public:
	static pair<char*, int> MakeLogin_UserIDAck(const char* NewUserID);

	static pair<char*, int> MakeQuit(unsigned short ClientNumber);

protected:
	static char* MakeHeader(char* Buffer, EPacket Type, unsigned short Size);
};

class UserData
{
public:
	UserData() {}
	UserData(int NewUserID, char* NewNickName)
	{
		UserID = NewUserID;
		NickName = NewNickName;
	}

	int UserID;
	char* NickName;

};

