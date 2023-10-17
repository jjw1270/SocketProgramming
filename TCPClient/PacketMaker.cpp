#include "PacketMaker.h"
#include <winsock.h>
#include <iostream>
#include <string>
using namespace std;

pair<char*, int> PacketMaker::MakeLogin_UserIDAck(const char* NewUserID)
{
	//size code Data(UserID)
	//[][] [][] [가변 데이타]

	int PacketSize = (int)strlen(NewUserID);
	int BufferSize = DefaultBufferSize + PacketSize;
	
	char* Buffer = new char[BufferSize];
	MakeHeader(Buffer, EPacket::C2S_Login_UserIDAck, PacketSize);

	memcpy(&Buffer[4], NewUserID, PacketSize);

	return make_pair(Buffer, BufferSize);
}

pair<char*, int> PacketMaker::MakeQuit(unsigned short _UserID)
{
	//size code Data(ClientNumber) 
	//[][] [][] [][]	
	PacketQuitClient Packet;

	int PacketSize = sizeof(Packet);
	int BufferSize = DefaultBufferSize + PacketSize;
	char* Buffer = new char[BufferSize];

	MakeHeader(Buffer, EPacket::S2C_QuitClient, PacketSize);

	//Packet.UserID = htons(_UserID);

	memcpy(&Buffer[4], &Packet, PacketSize);

	return make_pair(Buffer, BufferSize);
}

char* PacketMaker::MakeHeader(char* Buffer, EPacket Type, unsigned short Size)
{
	//size   code
	//[][]   [][]
	unsigned short size = htons(Size + 2);
	unsigned short code = htons(static_cast<unsigned short>(Type));

	memcpy(Buffer, &size, 2);
	memcpy(&Buffer[2], &code, 2);

	return Buffer;
}
