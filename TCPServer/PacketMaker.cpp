#include "PacketMaker.h"
#include <winsock.h>
#include <iostream>
#include <string>
using namespace std;

pair<char*, int> PacketMaker::MakeLogin_UserIDReq()
{
	int BufferSize = DefaultBufferSize;
	char* Buffer = new char[BufferSize];

	MakeHeader(Buffer, EPacket::S2C_Login_UserIDReq, 0);

	return make_pair(Buffer, BufferSize);
}

pair<char*, int> PacketMaker::MakeLogin_UserIDFailureReq()
{
	int BufferSize = DefaultBufferSize;
	char* Buffer = new char[BufferSize];

	MakeHeader(Buffer, EPacket::S2C_Login_UserIDFailureReq, 0);

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
