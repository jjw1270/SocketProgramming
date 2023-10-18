#include <iostream>
#include <fstream>
#include <string>
#include <process.h>

using namespace std;

#include "Packet.h"
#include "PacketMaker.h"

//--mysql
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
//--

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32")

#define FD_SETSIZE				100

fd_set Reads;
fd_set CopyReads;

// Used for Regist User
map<unsigned short, UserData> TempUserList;

// Use when User Login
map<unsigned short, UserData> SessionList;

bool GetConfigFromFile(string& OutServer, string& OutUserName, string& OutPassword)
{
	ifstream ConfigFile("config.txt");
	if (!ConfigFile.is_open())
	{
		cout << "Error: Could not open config file." << endl;
		return false;
	}

	string Line;
	size_t Start;
	size_t End;
	while (getline(ConfigFile, Line)) {
		if (Start = Line.find("Server = ") != string::npos) {
			End = Line.find('\n', Start);
			OutServer = Line.substr(Start + 8, End);		// Extract server information
		}
		else if (Start = Line.find("Username = ") != string::npos)
		{
			End = Line.find('\n', Start);
			OutUserName = Line.substr(Start + 10, End);	// Extract username
		}
		else if (Start = Line.find("Password = ") != string::npos)
		{
			End = Line.find('\n', Start);
			OutPassword = Line.substr(Start + 10, End);	// Extract password
		}
	}
	ConfigFile.close();
	return true;
}

bool SendPacket(SOCKET* ClientSocket, const EPacket& PacketToSend)
{
	pair<char*, int> BufferData = PacketMaker::MakePacket(PacketToSend);

	int SendByte = send(*ClientSocket, BufferData.first, BufferData.second, 0);
	if (SendByte <= 0)
	{
		cout << "Send Error" << GetLastError() << endl;
		return false;
	}
	return true;
}

bool SendPacket(SOCKET* ClientSocket, const EPacket& PacketToSend, const char* MessageToSend)
{
	pair<char*, int> BufferData = PacketMaker::MakePacket(PacketToSend, MessageToSend);

	int SendByte = send(*ClientSocket, BufferData.first, BufferData.second, 0);
	if (SendByte <= 0)
	{
		cout << "Send Error" << GetLastError() << endl;
		return false;
	}
	return true;
}

unsigned WINAPI ServerThread(void* arg);

sql::Connection* Sql_Connection;

int main()
{
	string Server;
	string Username;
	string Password;
	bool GetConfigSuccess = GetConfigFromFile(Server, Username, Password);
	if (!GetConfigSuccess)
	{
		cout << "Get Config txt Error" << endl;
		system("pause");
		exit(-1);
	}

	sql::Driver* Sql_Driver;

	try
	{
		Sql_Driver = get_driver_instance();
		Sql_Connection = Sql_Driver->connect(Server, Username, Password);  // 서버에 연결
		cout << "Connect DB Server" << endl;
	}
	catch (sql::SQLException e)
	{
		cout << "Could not connect to data base : " << e.what() << endl;
		system("pause");
		exit(-1);
	}

	Sql_Connection->setSchema("tcpproject");

	WSADATA WsaData;
	int Result = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if (Result != 0)
	{
		cout << "Error On StartUp : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		cout << "ListenSocket Error : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ListenSockAddr.sin_port = htons(7871);

	Result = _WINSOCK2API_::bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));
	if (Result == SOCKET_ERROR)
	{
		cout << "Bind Error : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	Result = listen(ListenSocket, SOMAXCONN);  //Socket, 한번에 요청 가능한 최대 접속 승인 수
	if (Result == SOCKET_ERROR)
	{
		cout << "Listen Error : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	struct timeval Timeout;
	Timeout.tv_sec = 0;
	Timeout.tv_usec = 500;

	FD_ZERO(&Reads);
	FD_SET(ListenSocket, &Reads);

	cout << "-----------------Launch Server-----------------" << endl;

	while (true)
	{
		CopyReads = Reads;

		int ChangeSocketCount = select(0, &CopyReads, 0, 0, &Timeout);
		if (ChangeSocketCount > 0)
		{
			for (int i = 0; i < (int)Reads.fd_count; ++i)
			{
				if (FD_ISSET(Reads.fd_array[i], &CopyReads))
				{
					//connect
					if (Reads.fd_array[i] == ListenSocket)
					{
						SOCKADDR_IN ClientSocketAddr;
						memset(&ClientSocketAddr, 0, sizeof(ClientSocketAddr));
						int ClientSockAddrLength = sizeof(ClientSocketAddr);

						SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSocketAddr, &ClientSockAddrLength);
						if (ClientSocket == INVALID_SOCKET)
						{
							cout << "Accept Error" << endl;
							cout << "Socket Error Num : " << GetLastError() << endl;
							continue;
						}

						FD_SET(ClientSocket, &Reads);
						CopyReads = Reads;
						char IP[1024] = { 0, };
						inet_ntop(AF_INET, &ClientSocketAddr.sin_addr.s_addr, IP, 1024);
						cout << "connected : " << IP << endl;

						// create thread
						_beginthreadex(nullptr, 0, ServerThread, (void*)&Reads.fd_array[i], 0, nullptr);

						break;
					}
				}
			}
		}
		else
		{
			// when no changes on socket count while timeout
			continue;
		}
	}

	// Clean Up

	closesocket(ListenSocket);
	WSACleanup();

	delete Sql_Connection;

	system("pause");

	return 0;
}

void RecvError(SOCKET& ClientSocket)
{
	SOCKADDR_IN ClientSocketAddr;
	int ClientSockAddrLength = sizeof(ClientSocketAddr);
	getpeername(ClientSocket, (SOCKADDR*)&ClientSocketAddr, &ClientSockAddrLength);

	SOCKET DisconnectSocket = ClientSocket;
	closesocket(ClientSocket);
	FD_CLR(ClientSocket, &Reads);
	CopyReads = Reads;

	if (TempUserList.count((unsigned short)DisconnectSocket) > 0)
	{
		TempUserList.erase(TempUserList.find((unsigned short)DisconnectSocket));
	}

	char IP[1024] = { 0, };
	inet_ntop(AF_INET, &ClientSocketAddr.sin_addr.s_addr, IP, 1024);
	cout << "disconnected : " << IP << endl;
}

void SendError(SOCKET& ClientSocket)
{
	SOCKADDR_IN ClientSocketAddr;
	int ClientSockAddrLength = sizeof(ClientSocketAddr);
	getpeername(ClientSocket, (SOCKADDR*)&ClientSocketAddr, &ClientSockAddrLength);

	SOCKET DisconnectSocket = ClientSocket;
	closesocket(ClientSocket);
	FD_CLR(ClientSocket, &Reads);
	CopyReads = Reads;

	if (TempUserList.count((unsigned short)DisconnectSocket) > 0)
	{
		TempUserList.erase(TempUserList.find((unsigned short)DisconnectSocket));
	}

	char IP[1024] = { 0, };
	inet_ntop(AF_INET, &ClientSocketAddr.sin_addr.s_addr, IP, 1024);
	cout << "disconnected : " << IP << endl;
}

unsigned WINAPI ServerThread(void* arg)
{
	SOCKET ClientSocket = *(SOCKET*)arg;

	// send req login
	bool bSendSuccess = SendPacket(&ClientSocket, EPacket::S2C_Login_UserIDReq);
	if (!bSendSuccess)
	{
		SendError(ClientSocket);
	}
	else
	{
		while (true)
		{
			// Recv PacketSize
			unsigned short PacketSize = 0;
			int RecvByte = recv(ClientSocket, (char*)(&PacketSize), 2, MSG_WAITALL);
			if (RecvByte == 0 || RecvByte < 0)
			{
				//close, recv Error
				RecvError(ClientSocket);
				break;
			}

			//Recv Code, Data
			PacketSize = ntohs(PacketSize);
			char* Buffer = new char[PacketSize];

			RecvByte = recv(ClientSocket, Buffer, PacketSize, MSG_WAITALL);
			if (RecvByte == 0 || RecvByte < 0)
			{
				//close, recv Error
				RecvError(ClientSocket);
				break;
			}

			//code 
			//[][]
			unsigned short Code = 0;
			memcpy(&Code, Buffer, 2);
			Code = ntohs(Code);

			// Data
			unsigned short DataSize = PacketSize - 2;
			bool bSendSuccess = false;
			switch ((EPacket)Code)
			{
			case EPacket::C2S_Login_UserIDAck:
			{
				char UserID[100] = { 0, };
				memcpy(&UserID, Buffer + 2, DataSize);

				//cout << "[" << (unsigned short)ClientSocket << "] Client ID : " << UserID << endl;
				printf("[%d] Client ID : ", (unsigned short)ClientSocket);

				// Check ID Exist in DB
				string SqlQuery = "SELECT * FROM userconfig WHERE ID = ?";
				sql::PreparedStatement* Sql_PreStatement = Sql_Connection->prepareStatement(SqlQuery);
				Sql_PreStatement->setString(1, UserID);
				sql::ResultSet* Sql_Result = Sql_PreStatement->executeQuery();

				// If ID doesnt exist in db
				if (Sql_Result->rowsCount() == 0)
				{
					cout << "ID Does Not Exist." << endl;

					unsigned short TempUserNumber = (unsigned short)ClientSocket;
					// check TempUser already exist
					if (TempUserList.count(TempUserNumber) == 0)
					{
						cout << "Make new Temp User" << endl;
						UserData NewTempUser(UserID);
						TempUserList.emplace(TempUserNumber, NewTempUser);

						bSendSuccess = SendPacket(&ClientSocket, EPacket::S2C_Login_UserIDFailureReq);
						if (!bSendSuccess)
						{
							SendError(ClientSocket);
							break;
						}
					}
					else
					{
						TempUserList[TempUserNumber].UserID = UserID;

						bSendSuccess = SendPacket(&ClientSocket, EPacket::S2C_Login_UserIDFailureReq);
						if (!bSendSuccess)
						{
							SendError(ClientSocket);
							break;
						}
					}
				}
				else
				{
					// ID 확인완료, 비밀번호 확인할 차례
				}

				delete Sql_Result;
				delete Sql_PreStatement;
			}
			break;
			case EPacket::C2S_Login_MakeNewUserReq:
			{
				cout << "Make New User Requested" << endl;
				bSendSuccess = SendPacket(&ClientSocket, EPacket::S2C_Login_NewUserNickNameReq);
				if (!bSendSuccess)
				{
					SendError(ClientSocket);
					break;
				}
			}
			break;
			case EPacket::C2S_Login_UserIDReq:
			{
				cout << "C2S_Login_UserIDReq" << endl;
				bSendSuccess = SendPacket(&ClientSocket, EPacket::S2C_Login_UserIDReq);
				if (!bSendSuccess)
				{
					SendError(ClientSocket);
					break;
				}
			}
			break;
			case EPacket::C2S_Login_NewUserNickNameAck:
			{
				char UserNickName[100] = { 0, };
				memcpy(&UserNickName, Buffer + 2, DataSize);

				cout << "User NickName : " << UserNickName << endl;

				// Check NickName already exist in db (NickName cant overlaped)
				string SqlQuery = "SELECT * FROM userconfig WHERE NickName = ?";
				sql::PreparedStatement* Sql_PreStatement = Sql_Connection->prepareStatement(SqlQuery);
				Sql_PreStatement->setString(1, UserNickName);
				sql::ResultSet* Sql_Result = Sql_PreStatement->executeQuery();

				unsigned short TempUserNumber = (unsigned short)ClientSocket;
				if (Sql_Result->rowsCount() > 0)
				{
					cout << "NickName Already Exist" << endl;
					bSendSuccess = SendPacket(&ClientSocket, EPacket::S2C_CastMessage, "Nick Name Already Exist.");
					if (!bSendSuccess)
					{
						SendError(ClientSocket);
						break;
					}
					bSendSuccess = SendPacket(&ClientSocket, EPacket::S2C_Login_NewUserNickNameReq);
					if (!bSendSuccess)
					{
						SendError(ClientSocket);
						break;
					}
				}
				else
				{
					TempUserList[TempUserNumber].NickName = UserNickName;
					bSendSuccess = SendPacket(&ClientSocket, EPacket::S2C_Login_NewUserPwdReq);
					if (!bSendSuccess)
					{
						SendError(ClientSocket);
						break;
					}
				}

				delete Sql_Result;
				delete Sql_PreStatement;
			}
			break;
			case EPacket::C2S_Login_NewUserPwdAck:
			{
				char UserPwd[100] = { 0, };
				memcpy(&UserPwd, Buffer + 2, DataSize);

				cout << "User password : " << UserPwd << endl;

				unsigned short TempUserNumber = (unsigned short)ClientSocket;
				cout << TempUserList[TempUserNumber].UserID << endl;
				// Create New Userconfig
				string SqlQuery = "INSERT INTO userconfig(ID, Password, NickName) VALUES(?,?,?)";
				sql::PreparedStatement* Sql_PreStatement = Sql_Connection->prepareStatement(SqlQuery);
				Sql_PreStatement->setString(1, TempUserList[TempUserNumber].UserID);
				Sql_PreStatement->setString(2, UserPwd);
				Sql_PreStatement->setString(3, TempUserList[TempUserNumber].NickName);
				Sql_PreStatement->execute();

				cout << "New User Registed" << endl;
				bSendSuccess = SendPacket(&ClientSocket, EPacket::S2C_CastMessage, "<< New User Registed! >>");
				if (!bSendSuccess)
				{
					SendError(ClientSocket);
					break;
				}bSendSuccess = SendPacket(&ClientSocket, EPacket::S2C_Login_UserIDReq);
				if (!bSendSuccess)
				{
					SendError(ClientSocket);
					break;
				}}
			break;

			default:
				break;
			}
			delete[] Buffer;
		}
	}

	return 0;
}
