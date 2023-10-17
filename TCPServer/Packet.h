#pragma once

#ifndef __PACKET_H__ 
#define __PACKET_H__

enum class EPacket
{
	None								= 0,

	S2C_CastMessage						= 1,
	C2S_CastMessage						= 2,

	S2C_Login_UserIDReq					= 100,
	C2S_Login_UserIDAck					= 101,
	
	S2C_Login_UserIDFailureReq			= 102,
	C2S_Login_UserIDReq					= 103,
	C2S_Login_MakeNewUserReq			= 110,

	S2C_Login_NewUserNickNameReq		= 111,
	C2S_Login_NewUserNickNameAck		= 112,

	S2C_Login_NewUserPwdReq				= 113,
	C2S_Login_NewUserPwdAck				= 114,

	S2C_Login_UserPwdReq				= 120,
	C2S_Login_UserPwdAck				= 121,

	S2C_Login_UserPwdFailureReq			= 122,
	C2S_Login_UserPwdReq				= 123,

	S2C_Login_LoginAck					= 130,


	S2C_QuitClient = 400,

	Max,
};

#pragma pack(1)

//typedef struct _PacketQuitClient
//{
//	unsigned int UserID;
//} PacketQuitClient;

#pragma pack()

#endif