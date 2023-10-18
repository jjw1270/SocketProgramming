#include "PacketMaker.h"
#include <iostream>
#include <string>
using namespace std;

bool PacketMaker::SendPacket(SOCKET* ClientSocket, EPacket PacketToSend)
{
	pair<char*, int> BufferData = MakePacket(PacketToSend);

	int SendByte = send(*ClientSocket, BufferData.first, BufferData.second, 0);
	if (SendByte <= 0)
	{
		cout << "Send Error : " << GetLastError() << endl;
		return false;
	}
	return true;
}

bool PacketMaker::SendPacket(SOCKET* ClientSocket, EPacket PacketToSend, const char* MessageToSend)
{
	pair<char*, int> BufferData = MakePacket(PacketToSend, MessageToSend);

	int SendByte = send(*ClientSocket, BufferData.first, BufferData.second, 0);
	if (SendByte <= 0)
	{
		cout << "Send Error" << GetLastError() << endl;
		return false;
	}
	return true;
}

pair<char*, int> PacketMaker::MakePacket(EPacket Type)
{
	int BufferSize = DefaultBufferSize;
	char* Buffer = new char[BufferSize];

	MakeHeader(Buffer, Type, 0);

	return make_pair(Buffer, BufferSize);
}

pair<char*, int> PacketMaker::MakePacket(EPacket Type, const char* NewData)
{
	//size code Data(UserID)
	//[][] [][] [가변 데이타]

	int PacketSize = (int)strlen(NewData);
	int BufferSize = DefaultBufferSize + PacketSize;

	char* Buffer = new char[BufferSize];
	MakeHeader(Buffer, Type, PacketSize);

	memcpy(&Buffer[4], NewData, PacketSize);

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
