#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <process.h>
using namespace std;

#include <WinSock2.h>
#include <ws2tcpip.h>

#include "PacketMaker.h"
#include "Packet.h"

#pragma comment(lib, "ws2_32")

EPacket CurrentPacket;
bool bIsRunning = true;

unsigned WINAPI RecvThread(void* arg)
{
	SOCKET ServerSocket = *(SOCKET*)arg;

	while (bIsRunning)
	{
		unsigned short PacketSize = 0;
		int RecvByte = recv(ServerSocket, (char*)(&PacketSize), 2, MSG_WAITALL);
		if (RecvByte == 0 || RecvByte < 0) //close, Error
		{
			cout << "Recv Error : " << GetLastError() << endl;
			bIsRunning = false;
			break;
		}
		else
		{
			PacketSize = ntohs(PacketSize);
			char* Buffer = new char[PacketSize];

			int RecvByte = recv(ServerSocket, Buffer, PacketSize, MSG_WAITALL);
			if (RecvByte == 0 || RecvByte < 0) //close, Error
			{
				cout << "Recv Error : " << GetLastError() << endl;
				bIsRunning = false;
				break;
			}
			else
			{
				//ProcessPacket
				unsigned short Code = 0;
				memcpy(&Code, Buffer, 2);
				Code = ntohs(Code);

				if ((EPacket)(Code) == EPacket::S2C_CastMessage)
				{
					cout << "Server Send Messages : ";
					char Message[1024] = { 0, };
					memcpy(&Message, Buffer + 2, PacketSize - 2);
					cout << Message << endl;
				}
				else if ((EPacket)(Code) == EPacket::S2C_Chat)
				{
					char Message[1024] = { 0, };
					memcpy(&Message, Buffer + 2, PacketSize - 2);
					cout << Message << endl;
				}
				else
				{
					CurrentPacket = (EPacket)(Code);

					switch (CurrentPacket)
					{
					case EPacket::S2C_LoginSuccess:
						system("cls"); // Clear the console
						cout << "Now you can Chat!" << endl;
						cout << "---------------------------------------------------------" << endl;
						break;
					}
				}
			}
		}
	}

	return 0;
}

unsigned WINAPI SendThread(void* arg)
{
	SOCKET ServerSocket = *(SOCKET*)arg;

	while (bIsRunning)
	{
		bool bSendSuccess = false;
		switch (CurrentPacket)
		{
		case EPacket::S2C_Login_UserIDReq:
		{
			cout << "Enter User ID : ";
			char UserID[101] = { 0, };
			cin >> UserID;
			cin.ignore();

			if (strlen(UserID) > 100)
			{
				cout << "User ID is too long." << endl;
				continue;
			}

			bSendSuccess = PacketMaker::SendPacket(&ServerSocket, EPacket::C2S_Login_UserIDAck, UserID);
			if (bSendSuccess)
			{
				CurrentPacket = EPacket::None;
			}
			else
			{
				//Send Error
				bIsRunning = false;
				goto EndThread;
			}
		}
		break;
		case EPacket::S2C_Login_UserIDFailureReq:
		{
			cout << "------------------------------------" << endl
				<< "ID does not exist." << endl
				<< "If you want make new ID, press Y," << endl
				<< "If you want re-enter ID, press N." << endl
				<< "------------------------------------" << endl
				<< "(Y/N) : ";

			char Check[10] = { 0, };
			cin >> Check;
			cin.ignore();

			if (strlen(Check) > 1)
			{
				cout << "Input Error" << endl;
				continue;
			}

			switch (Check[0])
			{
			case 'y':
			case 'Y':
			{
				bSendSuccess = PacketMaker::SendPacket(&ServerSocket, EPacket::C2S_Login_MakeNewUserReq);
				if (bSendSuccess)
				{
					CurrentPacket = EPacket::None;
				}
				else
				{
					//Send Error
					bIsRunning = false;
					goto EndThread;
				}
			}
			break;
			case 'n':
			case 'N':
			{
				bSendSuccess = PacketMaker::SendPacket(&ServerSocket, EPacket::C2S_Login_UserIDReq);
				if (bSendSuccess)
				{
					CurrentPacket = EPacket::None;
				}
				else
				{
					//Send Error
					bIsRunning = false;
					goto EndThread;
				}
			}
			break;
			default:
				cout << "Input Error" << endl;
				break;
			}
		}
		break;
		case EPacket::S2C_Login_NewUserNickNameReq:
		{
			cout << "Please Enter New User Nick Name : ";
			char UserNickName[101] = { 0, };
			cin >> UserNickName;
			cin.ignore();

			if (strlen(UserNickName) > 100)
			{
				cout << "Nick Name is too long." << endl;
				continue;
			}

			bSendSuccess = PacketMaker::SendPacket(&ServerSocket, EPacket::C2S_Login_NewUserNickNameAck, UserNickName);
			if (bSendSuccess)
			{
				CurrentPacket = EPacket::None;
			}
			else
			{
				//Send Error
				bIsRunning = false;
				goto EndThread;
			}
		}
		break;
		case EPacket::S2C_Login_NewUserPwdReq:
		{
			cout << "Please Enter New User Password : ";
			char NewUserPwd[101] = { 0, };
			cin >> NewUserPwd;
			cin.ignore();

			if (strlen(NewUserPwd) > 100)
			{
				cout << "Password is too long." << endl;
				continue;
			}

			bSendSuccess = PacketMaker::SendPacket(&ServerSocket, EPacket::C2S_Login_NewUserPwdAck, NewUserPwd);
			if (bSendSuccess)
			{
				CurrentPacket = EPacket::None;
			}
			else
			{
				//Send Error
				bIsRunning = false;
				goto EndThread;
			}
		}
		break;
		case EPacket::S2C_Login_UserPwdReq:
		{
			cout << "Please Enter Password : ";
			char UserPwd[101] = { 0, };
			cin >> UserPwd;
			cin.ignore();

			if (strlen(UserPwd) > 100)
			{
				cout << "Password is too long." << endl;
				continue;
			}

			bSendSuccess = PacketMaker::SendPacket(&ServerSocket, EPacket::C2S_Login_UserPwdAck, UserPwd);
			if (bSendSuccess)
			{
				CurrentPacket = EPacket::None;
			}
			else
			{
				//Send Error
				bIsRunning = false;
				goto EndThread;
			}

		}
		break;
		case EPacket::S2C_Login_UserPwdFailureReq:
		{
			cout << "------------------------------------" << endl
				<< "Password is incorrect." << endl
				<< "If you want re-enter password, press Y," << endl
				<< "If you want re-enter ID, press N." << endl
				<< "------------------------------------" << endl
				<< "(Y/N) : ";

			char Check[10] = { 0, };
			cin >> Check;
			cin.ignore();

			if (strlen(Check) > 1)
			{
				cout << "Input Error" << endl;
				continue;
			}

			switch (Check[0])
			{
			case 'y':
			case 'Y':
			{
				bSendSuccess = PacketMaker::SendPacket(&ServerSocket, EPacket::C2S_Login_UserPwdReq);
				if (bSendSuccess)
				{
					CurrentPacket = EPacket::None;
				}
				else
				{
					//Send Error
					bIsRunning = false;
					goto EndThread;
				}
			}
			break;
			case 'n':
			case 'N':
			{
				bSendSuccess = PacketMaker::SendPacket(&ServerSocket, EPacket::C2S_Login_UserIDReq);
				if (bSendSuccess)
				{
					CurrentPacket = EPacket::None;
				}
				else
				{
					//Send Error
					bIsRunning = false;
					goto EndThread;
				}
			}
			break;
			default:
				cout << "Input Error" << endl;
				break;
			}
		}
		break;
		case EPacket::S2C_CanChat:
		{
			string ChatInput;
			getline(cin, ChatInput);

			if (ChatInput.length() == 0)
			{
				break;
			}

			bSendSuccess = PacketMaker::SendPacket(&ServerSocket, EPacket::C2S_Chat, ChatInput.data());
			if (!bSendSuccess)
			{
				//Send Error
				bIsRunning = false;
				goto EndThread;
			}
		}
		break;
		default:
			break;
		}
	}
EndThread:
	return 0;
}

int main()
{
	WSAData WsaData;
	int Result = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if (Result != 0)
	{
		cout << "WSAStartup Error" << endl;
		system("pause");
		exit(-1);
	}

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (ServerSocket == SOCKET_ERROR)
	{
		cout << "ServerSocket Error" << endl;
		system("pause");
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
		system("pause");
		exit(-1);
	}

	Result = connect(ServerSocket, (SOCKADDR*)&ServerSock, sizeof(ServerSock));
	if (Result == SOCKET_ERROR)
	{
		if (GetLastError() == 10061)
		{
			cout << "Server Is Sleeping.." << endl;
		}
		else
		{
			cout << "connect Error : " << GetLastError() << endl;
		}
		system("pause");
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

	// Clean Up

	CloseHandle(ThreadHandles[1]);
	CloseHandle(ThreadHandles[0]);

	closesocket(ServerSocket);
	WSACleanup();

	system("pause");
	return 0;
}
