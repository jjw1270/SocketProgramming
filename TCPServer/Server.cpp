#include <iostream>
#include <fstream>
#include <string>
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

string server;
string username;
string password;

map<unsigned short, UserData> SessionList;

map<unsigned short, UserData> TempUserList;

bool GetConfigFromFile()
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
			server = Line.substr(Start + 8, End);		// Extract server information
		}
		else if (Start = Line.find("Username = ") != string::npos)
		{
			End = Line.find('\n', Start);
			username = Line.substr(Start + 10, End);	// Extract username
		}
		else if (Start = Line.find("Password = ") != string::npos)
		{
			End = Line.find('\n', Start);
			password = Line.substr(Start + 10, End);	// Extract password
		}
	}
	ConfigFile.close();
	return true;
}

void ReqUserID(SOCKET* ClientSocket)
{
	pair<char*, int> BufferData = PacketMaker::MakePacket(EPacket::S2C_Login_UserIDReq);

	cout << "Request User ID" << endl;

	send(*ClientSocket, BufferData.first, BufferData.second, 0);
}

void ReqUserIDFailure(SOCKET* ClientSocket)
{
	pair<char*, int> BufferData = PacketMaker::MakePacket(EPacket::S2C_Login_UserIDFailureReq);

	cout << "Request User ID Failure" << endl;

	send(*ClientSocket, BufferData.first, BufferData.second, 0);
}

void ReqNewUserNickName(SOCKET* ClientSocket)
{
	pair<char*, int> BufferData = PacketMaker::MakePacket(EPacket::S2C_Login_NewUserNickNameReq);

	cout << "Request New User NickName" << endl;

	send(*ClientSocket, BufferData.first, BufferData.second, 0);
}

void RecvError(SOCKET& Soket);

int main()
{
	bool GetConfigSuccess = GetConfigFromFile();
	if (!GetConfigSuccess)
	{
		cout << "Get Config txt Error" << endl;
		exit(-1);
	}

	sql::Driver* Sql_Driver;
	sql::Connection* Sql_Connection;
	sql::PreparedStatement* Sql_PreStatement;
	sql::Statement* Sql_Statement;
	sql::ResultSet* Sql_Result;

	try
	{
		Sql_Driver = get_driver_instance();
		Sql_Connection = Sql_Driver->connect(server, username, password);  // 서버에 연결
		cout << "Connect DB Server" << endl;
	}
	catch (sql::SQLException e)
	{
		cout << "Could not connect to server. Error message: " << e.what() << endl;
		system("pause");
		exit(-1);
	}

	Sql_Connection->setSchema("tcpproject");

	WSADATA WsaData;
	int Result = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if (Result != 0)
	{
		cout << "Error On StartUp" << endl;
		exit(-1);
	}

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		cout << "ListenSocket Error" << endl;
		cout << "Socket Error Num : " << GetLastError() << endl;
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
		cout << "Bind Error" << endl;
		cout << "Socket Error Num : " << GetLastError() << endl;
		exit(-1);
	}

	Result = listen(ListenSocket, SOMAXCONN);  //Socket, 한번에 요청 가능한 최대 접속 승인 수
	if (Result == SOCKET_ERROR)
	{
		cout << "listen Error" << endl;
		cout << "Socket Error Num : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	struct timeval Timeout;
	Timeout.tv_sec = 0;
	Timeout.tv_usec = 500;

	FD_ZERO(&Reads);
	FD_SET(ListenSocket, &Reads);

	cout << "-----------Launch Server------------" << endl;

	while (true)
	{
		CopyReads = Reads;

		int ChangeSocketCount = select(0, &CopyReads, 0, 0, &Timeout);

		if (ChangeSocketCount == 0)
		{
			//다른 처리 
			continue;
		}
		else
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
							exit(-1);
						}

						FD_SET(ClientSocket, &Reads);
						CopyReads = Reads;
						char IP[1024] = { 0, };
						inet_ntop(AF_INET, &ClientSocketAddr.sin_addr.s_addr, IP, 1024);
						cout << "connected : " << IP << endl;

						// send req login
						ReqUserID(&ClientSocket);

						break;
					}
					else //recv
					{
						// Recv (size)
						unsigned short PacketSize = 0;
						int RecvByte = recv(Reads.fd_array[i], (char*)(&PacketSize), 2, MSG_WAITALL);

						if (RecvByte == 0 || RecvByte < 0)
						{
							//close, recv Error
							RecvError(Reads.fd_array[i]);
							break;
						}
						else
						{
							//Recv (code, packet)
							PacketSize = ntohs(PacketSize);
							char* Buffer = new char[PacketSize];

							int RecvByte = recv(Reads.fd_array[i], Buffer, PacketSize, MSG_WAITALL);
							if (RecvByte == 0 || RecvByte < 0)
							{
								//close, recv Error
								RecvError(Reads.fd_array[i]);
								break;
							}
							else
							{
								//code 
								//[][]
								unsigned short Code = 0;
								memcpy(&Code, Buffer, 2);
								Code = ntohs(Code);

								// Data
								switch ((EPacket)Code)
								{
								case EPacket::C2S_Login_UserIDAck:
								{
									char UserID[100] = { 0, };
									memcpy(&UserID, Buffer + 2, PacketSize - 2);

									cout << "Client ID : " << UserID << endl;

									// Create a SQL query string with a placeholder for the UserID
									string SqlQuery = "SELECT * FROM userconfig WHERE ID = ?";
									// Prepare the SQL statement
									Sql_PreStatement = Sql_Connection->prepareStatement(SqlQuery);
									// Bind the UserID to the prepared statement
									Sql_PreStatement->setString(1, UserID);
									Sql_Result = Sql_PreStatement->executeQuery();

									if (Sql_Result->rowsCount() == 0)
									{
										cout << "ID Does Not Exist." << endl;

										unsigned short TempUserNumber = (unsigned short)Reads.fd_array[i];
										// check TempUser already exist
										if (TempUserList.count(TempUserNumber) == 0)
										{
											UserData NewTempUser(UserID);

											TempUserList[TempUserNumber] = NewTempUser;

											ReqUserIDFailure(&Reads.fd_array[i]);
										}
										else
										{
											TempUserList[TempUserNumber].UserID = UserID;

											ReqUserIDFailure(&Reads.fd_array[i]);
										}
									}
									else
									{
										// ID 확인완료, 비밀번호 확인할 차례
									}
								}
								break;
								case EPacket::C2S_Login_MakeNewUserReq:
								{
									cout << "C2S_Login_MakeNewUserReq" << endl;
									ReqNewUserNickName(&Reads.fd_array[i]);
								}
								break;
								case EPacket::C2S_Login_UserIDReq:
								{
									cout << "C2S_Login_UserIDReq" << endl;
									ReqUserID(&Reads.fd_array[i]);
								}
								break;
								case EPacket::C2S_Login_NewUserNickNameAck:
								{
									char UserNickName[100] = { 0, };
									memcpy(&UserNickName, Buffer + 2, PacketSize - 2);

									cout << "User NickName : " << UserNickName << endl;

									// Create a SQL query string with a placeholder for the UserID
									string SqlQuery = "SELECT * FROM userconfig WHERE NickName = ?";
									// Prepare the SQL statement
									Sql_PreStatement = Sql_Connection->prepareStatement(SqlQuery);
									// Bind the UserID to the prepared statement
									Sql_PreStatement->setString(1, UserNickName);
									Sql_Result = Sql_PreStatement->executeQuery();

									unsigned short TempUserNumber = (unsigned short)Reads.fd_array[i];
									if (Sql_Result->rowsCount() > 0)
									{
										cout << "NickName Already Exist" << endl;

									}
									else
									{
										TempUserList[TempUserNumber].NickName = UserNickName;
										cout << "NickName varified" << endl;
									}
								}
								break;
								}
							}
							delete[] Buffer;
						}
					}
				}
			}
		}
	}

	// Clean Up

	delete Sql_Result;
	delete Sql_Statement;
	delete Sql_PreStatement;
	delete Sql_Connection;

	closesocket(ListenSocket);

	WSACleanup();

	return 0;
}

void RecvError(SOCKET& Soket)
{
	SOCKADDR_IN ClientSocketAddr;
	int ClientSockAddrLength = sizeof(ClientSocketAddr);
	getpeername(Soket, (SOCKADDR*)&ClientSocketAddr, &ClientSockAddrLength);

	SOCKET DisconnectSocket = Soket;
	closesocket(Soket);
	FD_CLR(Soket, &Reads);
	CopyReads = Reads;

	// SessionList.erase(SessionList.find((unsigned short)DisconnectSocket));

	char IP[1024] = { 0, };
	inet_ntop(AF_INET, &ClientSocketAddr.sin_addr.s_addr, IP, 1024);
	cout << "disconnected : " << IP << endl;
}
