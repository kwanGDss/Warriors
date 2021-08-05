// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include <iostream>
#include <WinSock2.h>
#include <map>
#include "Runtime/Core/Public/HAL/Runnable.h"

#pragma comment (lib, "ws2_32.lib")

using namespace std;

#include "..\..\..\Warriors_Server\Warriors_Server\Protocol.h"

enum EPacketType
{
	SIGNUP_PLAYER,
	LOGIN_PLAYER,
	ENROLL_PLAYER,
	SEND_PLAYER,
	RECV_PLAYER,
	LOGOUT_PLAYER,
	HIT_PLAYER,
	DAMAGED_PLAYER,
	CHAT,
	ENTER_NEW_PLAYER
	/*HIT_MONSTER,
	SYNC_MONSTER,
	SPAWN_MONSTER,
	DESTROY_MONSTER*/
};

// IOCP 소켓 구조체
struct stSOCKETINFO
{
	WSAOVERLAPPED	overlapped;
	WSABUF			dataBuf;
	SOCKET			socket;
	char			messageBuffer[MAX_BUFFER];
	int				recvBytes;
	int				sendBytes;
};

struct SOCKETINFO
{
	WSAOVERLAPPED	overlapped;
	WSABUF			wsabuf[1];
	unsigned char	messagebuf[MAX_BUFFER];
	EPacketType		packettype;
	OP_TYPE			m_op;
	SOCKET			client_socket;
};


class cCharacter {
public:
	cCharacter();
	~cCharacter();

	// 세션 아이디
	int		SessionId;
	// 위치
	float	X;
	float	Y;
	float	Z;
	// 회전값
	float	Yaw;
	float	Pitch;
	float	Roll;
	// 속도
	float	VX;
	float	VY;
	float	 VZ;
	// 속성
	bool	IsAlive;		
	float	HealthValue;
	bool	IsAttacking;

	friend ostream& operator<<(ostream &stream, cCharacter& info)
	{
		stream << info.SessionId << endl;
		stream << info.X << endl;
		stream << info.Y << endl;
		stream << info.Z << endl;
		stream << info.VX << endl;
		stream << info.VY << endl;
		stream << info.VZ << endl;
		stream << info.Yaw << endl;
		stream << info.Pitch << endl;
		stream << info.Roll << endl;
		stream << info.IsAlive << endl;		
		stream << info.HealthValue << endl;
		stream << info.IsAttacking << endl;

		return stream;
	}

	friend istream& operator>>(istream& stream, cCharacter& info)
	{
		stream >> info.SessionId;
		stream >> info.X;
		stream >> info.Y;
		stream >> info.Z;
		stream >> info.VX;
		stream >> info.VY;
		stream >> info.VZ;
		stream >> info.Yaw;
		stream >> info.Pitch;
		stream >> info.Roll;
		stream >> info.IsAlive;		
		stream >> info.HealthValue;
		stream >> info.IsAttacking;

		return stream;
	}
};

class cCharactersInfo
{
public:
	cCharactersInfo();
	~cCharactersInfo();
	
	map<int, cCharacter> players;

	friend ostream& operator<<(ostream &stream, cCharactersInfo& info)
	{
		stream << info.players.size() << endl;
		for (auto& kvp : info.players)
		{
			stream << kvp.first << endl;
			stream << kvp.second << endl;
		}

		return stream;
	}

	friend istream &operator>>(istream &stream, cCharactersInfo& info)
	{
		int nPlayers = 0;
		int SessionId = 0;
		cCharacter Player;
		info.players.clear();

		stream >> nPlayers;
		for (int i = 0; i < nPlayers; i++)
		{
			stream >> SessionId;
			stream >> Player;
			info.players[SessionId] = Player;			
		}

		return stream;
	}
};


class WARRIORS_API AClientSocket : public FRunnable
{
	AClientSocket();
	virtual ~AClientSocket();

	// 소켓 등록 및 설정
	bool InitSocket();
	// 서버와 연결
	bool Connect(const char * pszIP, int nPort);

	//////////////////////////////////////////////////////////////////////////
	// 서버와 통신
	//////////////////////////////////////////////////////////////////////////

	// 회원가입
	bool SignUp(const FText & Id, const FText & Pw);
	// 서버에 로그인
	bool Login(const FText & Id, const FText & Pw);
	// 초기 캐릭터 등록
	void EnrollPlayer(cCharacter& info);
	// 캐릭터 동기화
	void SendPlayer(cCharacter& info);	
	// 캐릭터 로그아웃
	void LogoutPlayer(const int& SessionId);	
	// 캐릭터 피격 처리
	void HitPlayer(const int& SessionId);
	//////////////////////////////////////////////////////////////////////////	

	// 플레이어 컨트롤러 세팅
	//void SetPlayerController(ASungminPlayerController* pPlayerController);

	void CloseSocket();

	// FRunnable Thread members	
	FRunnableThread* Thread;
	FThreadSafeCounter StopTaskCounter;

	// FRunnable override 함수
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	virtual void Exit();

	// 스레드 시작 및 종료
	bool StartListen();
	void StopListen();	

	// 싱글턴 객체 가져오기
	/*static ClientSocket* GetSingleton()
	{
		static ClientSocket ins;
		return &ins;
	}*/

private:
	SOCKET	ServerSocket;				// 서버와 연결할 소켓	
	char 	recvBuffer[MAX_BUFFER];		// 수신 버퍼 스트림	
	
	//ASungminPlayerController* PlayerController;	// 플레이어 컨트롤러 정보	

	//////////////////////////////////////////////////////////////////////////
	// 역직렬화
	//////////////////////////////////////////////////////////////////////////
	cCharactersInfo CharactersInfo;		// 캐릭터 정보
	cCharactersInfo* RecvCharacterInfo(stringstream& RecvStream);

	string sChat;
	string* RecvChat(stringstream& RecvStream);

	cCharacter	NewPlayer;
	cCharacter* RecvNewPlayer(stringstream& RecvStream);
};
