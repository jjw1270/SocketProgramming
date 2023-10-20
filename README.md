# SocketProgramming

## TCP 채팅서버 구현


### <패킷 구조>

**해더(4 byte) + 데이터(0~1024 byte)**

- 해더 : 패킷 크기(2byte) + 패킷 코드(2byte)

#### 패킷 코드(enum)

**S2C : Server to Client**

**C2S : Client to Server**

<details>
<summary>enum</summary>
<div markdown="1">
- None								                      = 0
- S2C_CastMessage						             = 1
- C2S_CastMessage						             = 2  //reserved

-	S2C_Login_UserIDReq					          = 100
-	C2S_Login_UserIDAck					          = 101
	
-	S2C_Login_UserIDFailureReq			     = 102
-	C2S_Login_UserIDReq					          = 103
-	C2S_Login_MakeNewUserReq			       = 110

-	S2C_Login_NewUserNickNameReq		    = 111
-	C2S_Login_NewUserNickNameAck		    = 112

-	S2C_Login_NewUserPwdReq				       = 113
-	C2S_Login_NewUserPwdAck				       = 114

-	S2C_Login_UserPwdReq				          = 120
-	C2S_Login_UserPwdAck				          = 121

- S2C_Login_UserPwdFailureReq			    = 122
-	C2S_Login_UserPwdReq			 	         = 123

-	S2C_LoginSuccess					             = 150
-	S2C_AlreadyLoginOnServer			       = 151

-	S2C_CanChat							                = 200
-	C2S_Chat							                   = 201
-	S2C_Chat							                   = 202

-	Max

</div>
</details>

---
---

### Server

- 미리 만들어둔 config.txt 파일에서 DB 서버 IP, UserName, Pwd 가져옴. config.txt 파일은 ignore 처리함.
- mysql/c++ connector 라이브러리 사용해서 DB와 연결
- Select Server 구현
  - 각각의 클라들에 대한 ServerThread 실행(싱글 스레드)

#### ServerThread

1. ID 요청 패킷 전송
2. 아래 반복
   
   1. 헤더의 패킷크기 수신 대기 (2byte)
 
   2. 헤더의 패킷 크기만큼 패킷 데이터 수신 (2byte(패킷 코드) + 패킷크기 - 2byte)

   3. 패킷 코드에 따라 case문 실행
        - 전송할 데이터가 있으면 패킷을 만들어 전송

#### DB

- 한글(멀티 바이트)를 깨지지 않게 DB에 입출력 하기위해 MultibyteToUtf8, Utf8ToMultibyte 함수 구현
    - Nickname, 채팅을 위 함수를 사용하여 변환하여 DB에 입출력

- DB 테이블
   - userconfig
      - 유저의 ID, 닉네임, 비밀번호를 저장하는 테이블
   - chatlog
      - 유저들이 채팅 내역을 저장하는 테이블


---
---

### Client

- TCP 서버에 접속 요청
- 서버에 접속 성공 시 RecvThread, SendThread 실행 (멀티 스레드)

#### RecvThread

- 아래 반복
  1. 서버 소켓으로 패킷크기 수신 대기 (2byte)

  2. 헤더의 패킷 크기만큼 패킷 데이터 수신 (2byte(패킷 코드) + 패킷크기 - 2byte)
 
  3. 수신받은 패킷 코드를 전역변수에 저장, 패킷 코드에 따라 case문 실행
 

#### SendThread

- 아래 반복
  1. 수신받은 패킷 코드에 따라 case문 실행
       - 전송할 데이터를 패킷을 만들어 전송

---
---


