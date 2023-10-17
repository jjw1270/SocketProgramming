#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <process.h>
using namespace std;

#include <WinSock2.h>
#include <ws2tcpip.h>

#include "PacketMaker.h"
#include "Packet.h"

#pragma comment(lib, "ws2_32")

// map<unsigned short, UserData> SessionList;

EPacket CurrentPacket;

unsigned WINAPI RecvThread(void* arg)
{
	SOCKET ServerSocket = *(SOCKET*)arg;

	while (true)
	{
		unsigned short PacketSize = 0;
		int RecvByte = recv(ServerSocket, (char*)(&PacketSize), 2, MSG_WAITALL);
		if (RecvByte == 0 || RecvByte < 0) //close, Error
		{
			//disconnect
			break;
		}
		else
		{
			PacketSize = ntohs(PacketSize);
			char* Buffer = new char[PacketSize];

			int RecvByte = recv(ServerSocket, Buffer, PacketSize, MSG_WAITALL);
			if (RecvByte == 0 || RecvByte < 0) //close, Error
			{
				break;
				//disconnect
			}
			else
			{
				//ProcessPacket
				unsigned short Code = 0;
				memcpy(&Code, Buffer, 2);
				Code = ntohs(Code);

				CurrentPacket = (EPacket)(Code);

				switch (CurrentPacket)
				{
				case EPacket::S2C_Login_UserIDReq:
					//cout << "User ID Requested" << endl;
					break;
				case EPacket::S2C_Login_UserIDFailureReq:
					//cout << "User ID Failure Requested" << endl;
					break;
				}
			}
		}
	}

	return 0;
}

unsigned WINAPI SendThread(void* arg)
{
	SOCKET ServerSocket = *(SOCKET*)arg;

	bool bIsRunning = true;
	while (bIsRunning)
	{
		switch (CurrentPacket)
		{
		case EPacket::S2C_Login_UserIDReq:
		{
			cout << "Enter User ID : ";
			char UserID[100] = { 0, };
			cin.getline(UserID, 100);

			pair<char*, int> BufferData = PacketMaker::MakeLogin_UserIDAck(UserID);
			int SendByte = send(ServerSocket, BufferData.first, BufferData.second, 0);
			if (SendByte > 0)
			{
				CurrentPacket = EPacket::None;
			}
		}
		case EPacket::S2C_Login_UserIDFailureReq:
		{
			cout << "------------------------------------" << endl
				 << "ID does not exist." << endl
				 << "If you want make new ID, press Y," << endl
				 << "If you want re-enter ID, press N." << endl
				 << "------------------------------------" << endl
				 << "(Y/N) : ";
			char Check;
			cin >> Check;
			switch (Check)
			{
			case 'y':
			case 'Y':
				cout << "Enter Y" << endl;

				break;
			case 'n':
			case 'N':
				cout << "Enter N" << endl;

				break;
			default:
				continue;
			}
			
		}
		break;
		}
	}

	return 0;
}

int main()
{
	WSAData WsaData;
	int Result = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if (Result != 0)
	{
		cout << "WSAStartup Error" << endl;
		exit(-1);
	}

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (ServerSocket == SOCKET_ERROR)
	{
		cout << "ServerSocket Error" << endl;
		exit(-1);
	}

	SOCKADDR_IN ServerSock;
	ZeroMemory(&ServerSock, 0);
	ServerSock.sin_family = PF_INET;
	ServerSock.sin_port = htons(7871);
	Result = inet_pton(AF_INET,
		"127.0.0.1", &(ServerSock.sin_addr.s_addr));
	if (Result == SOCKET_ERROR)
	{
		cout << "inet_pton Error : " << GetLastError() << endl;
		exit(-1);
	}

	Result = connect(ServerSocket, (SOCKADDR*)&ServerSock, sizeof(ServerSock));
	if (Result == SOCKET_ERROR)
	{
		cout << "connect Error : " << GetLastError() << endl;
		exit(-1);
	}
	else
	{
		cout << "Connect to Server" << endl;
	}

	HANDLE ThreadHandles[2];
	ThreadHandles[0] = (HANDLE)_beginthreadex(nullptr, 0, RecvThread, (void*)&ServerSocket, 0, nullptr);
	ThreadHandles[1] = (HANDLE)_beginthreadex(nullptr, 0, SendThread, (void*)&ServerSocket, 0, nullptr);

	WaitForMultipleObjects(2, ThreadHandles, TRUE, INFINITE);

	WSACleanup();

	return 0;
}
