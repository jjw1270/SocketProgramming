#pragma once

#include "Packet.h"
#include <iostream>
using namespace std;

class PacketMaker
{
protected:
	static const int DefaultBufferSize = 4;

public:
	static pair<char*, int> MakeLogin_UserIDReq();
	static pair<char*, int> MakeLogin_UserIDFailureReq();


protected:
	static char* MakeHeader(char* Buffer, EPacket Type, unsigned short Size);
};

class UserData
{
public:
	UserData() {}
	UserData(int NewUserID, char* NewNickName)
	{
		srand((unsigned int)time(nullptr));
		UserID = NewUserID;
		NickName = NewNickName;
	}

	int UserID;
	char* NickName;

};

