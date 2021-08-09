// Fill out your copyright notice in the Description page of Project Settings.

#include "ClientSocket.h"
#include <sstream>
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformAffinity.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"
#include <algorithm>
#include <string>
//#include "PlayerController.h"

cCharactersInfo* AClientSocket::RecvCharacterInfo(stringstream& RecvStream)
{
	// 캐릭터 정보를 얻어 반환		
	RecvStream >> CharactersInfo;
	return &CharactersInfo;	
}

string* AClientSocket::RecvChat(stringstream& RecvStream)
{
	// 채팅 정보를 얻어 반환
	RecvStream >> sChat;
	std::replace(sChat.begin(), sChat.end(), '_', ' ');
	return &sChat;
}

cCharacter* AClientSocket::RecvNewPlayer(stringstream& RecvStream)
{
	// 새 플레이어 정보를 얻어 반환
	RecvStream >> NewPlayer;
	return &NewPlayer;
}

bool AClientSocket::Init()
{
	return true;
}

uint32 AClientSocket::Run()
{
	// 초기 init 과정을 기다림
	FPlatformProcess::Sleep(0.03);	
	// recv while loop 시작
	// StopTaskCounter 클래스 변수를 사용해 Thread Safety하게 해줌
	/*while (StopTaskCounter.GetValue() == 0 && PlayerController != nullptr)
	{
		stringstream RecvStream;
		int PacketType;
		int nRecvLen = recv(
			ServerSocket, (CHAR*)&recvBuffer, MAX_BUFFER, 0
		);
		if (nRecvLen > 0)
		{
			// 패킷 처리
			RecvStream << recvBuffer;
			RecvStream >> PacketType;

			switch (PacketType)
			{
			case EPacketType::RECV_PLAYER:
			{
				PlayerController->RecvWorldInfo(RecvCharacterInfo(RecvStream));
			}
			break;
			case EPacketType::CHAT:
			{
				PlayerController->RecvChat(RecvChat(RecvStream));
			}
			break;
			case EPacketType::ENTER_NEW_PLAYER:
			{
				PlayerController->RecvNewPlayer(RecvNewPlayer(RecvStream));
			}
			break;
			case EPacketType::SYNC_MONSTER:
			{
				PlayerController->RecvMonsterSet(RecvMonsterSet(RecvStream));
			}
			break;
			case EPacketType::SPAWN_MONSTER:
			{
				PlayerController->RecvSpawnMonster(RecvMonster(RecvStream));
			}
			break;
			case EPacketType::DESTROY_MONSTER:
			{
				PlayerController->RecvDestroyMonster(RecvMonster(RecvStream));
			}
			break;
			default:
				break;
			}
		}
	}*/
	return 0;
}

void AClientSocket::Stop()
{
	// thread safety 변수를 조작해 while loop 가 돌지 못하게 함
	StopTaskCounter.Increment();
}

void AClientSocket::Exit()
{
}

bool AClientSocket::StartListen()
{
	// 스레드 시작
	if (Thread != nullptr) return false;
	Thread = FRunnableThread::Create(this, TEXT("ClientSocket"), 0, TPri_BelowNormal);
	return (Thread != nullptr);
}

void AClientSocket::StopListen()
{
	// 스레드 종료
	Stop();
	Thread->WaitForCompletion();
	Thread->Kill();	
	delete Thread;
	Thread = nullptr;	
	StopTaskCounter.Reset();
}

AClientSocket::AClientSocket()
{
}

AClientSocket::~AClientSocket()
{
	closesocket(ServerSocket);
	WSACleanup();
}

bool AClientSocket::InitSocket()
{
	WSADATA wsaData;
	// 윈속 버전을 2.2로 초기화
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nRet != 0) {		
		return false;
	}

	// TCP 소켓 생성	
	ServerSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);	
	if (ServerSocket == INVALID_SOCKET) {		
		return false;
	}
	
	return true;
}

bool AClientSocket::Connect(const char* pszIP, int nPort)
{
	// 접속할 서버 정보를 저장할 구조체
	SOCKADDR_IN stServerAddr;

	stServerAddr.sin_family = AF_INET;
	// 접속할 서버 포트 및 IP
	stServerAddr.sin_port = htons(nPort);
	stServerAddr.sin_addr.s_addr = inet_addr(pszIP);

	int nRet = connect(ServerSocket, (sockaddr*)&stServerAddr, sizeof(sockaddr));	
	if (nRet == SOCKET_ERROR) {		
		return false;
	}	

	return true;
}

bool AClientSocket::SignUp(const FText& Id, const FText& Pw)
{
	stringstream SendStream;
	// 회원가입 정보를 서버에 보낸다
	SendStream << EPacketType::SIGNUP_PLAYER << endl;
	SendStream << TCHAR_TO_UTF8(*Id.ToString()) << endl;
	SendStream << TCHAR_TO_UTF8(*Pw.ToString()) << endl;

	int nSendLen = send(
		ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0
	);

	if (nSendLen == -1)
		return false;

	// 서버로부터 응답 대기
	int nRecvLen = recv(
		ServerSocket, (CHAR*)&recvBuffer, MAX_BUFFER, 0
	);

	if (nRecvLen <= 0)
		return false;

	stringstream RecvStream;
	int PacketType;
	bool SignUpResult;

	RecvStream << recvBuffer;
	RecvStream >> PacketType;
	RecvStream >> SignUpResult;

	if (PacketType != EPacketType::SIGNUP_PLAYER)
		return false;

	// 회원가입 성공 유무를 반환
	return SignUpResult;
}

bool AClientSocket::Login(const FText& Id, const FText& Pw)
{
	stringstream SendStream;
	// 로그인 정보를 서버에 보낸다
	SendStream << EPacketType::LOGIN_PLAYER << endl;
	SendStream << TCHAR_TO_UTF8(*Id.ToString()) << endl;
	SendStream << TCHAR_TO_UTF8(*Pw.ToString()) << endl;

	int nSendLen = send(
		ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0
	);

	if (nSendLen == -1)
		return false;
	// 서버로부터 응답 대기
	int nRecvLen = recv(
		ServerSocket, (CHAR*)&recvBuffer, MAX_BUFFER, 0
	);

	if (nRecvLen <= 0)
		return false;

	stringstream RecvStream;
	int PacketType;
	bool LoginResult;

	RecvStream << recvBuffer;
	RecvStream >> PacketType;
	RecvStream >> LoginResult;

	if (PacketType != EPacketType::LOGIN_PLAYER)
		return false;
	// 로그인 성공 유무를 반환
	return LoginResult;
}

void AClientSocket::EnrollPlayer(cCharacter& info)
{
	// 캐릭터 정보 직렬화
	stringstream SendStream;
	// 요청 종류
	SendStream << EPacketType::ENROLL_PLAYER << endl;;
	SendStream << info;

	// 캐릭터 정보 전송
	int nSendLen = send(
		ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0
	);

	if (nSendLen == -1)
	{
		return;
	}
}

void AClientSocket::SendPlayer(cCharacter& info)
{
	// 캐릭터 정보 직렬화
	stringstream SendStream;
	// 요청 종류
	SendStream << EPacketType::SEND_PLAYER << endl;;
	SendStream << info;

	// 캐릭터 정보 전송
	int nSendLen = send(
		ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0
	);
		
	if (nSendLen == -1)
	{
		return;
	}	
}

void AClientSocket::LogoutPlayer(const int& SessionId)
{
	// 서버에게 로그아웃시킬 캐릭터 정보 전송
	stringstream SendStream;
	SendStream << EPacketType::LOGOUT_PLAYER << endl;
	SendStream << SessionId << endl;

	int nSendLen = send(
		ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0
	);

	if (nSendLen == -1)
	{
		return;
	}
	
	closesocket(ServerSocket);
	WSACleanup();
}

void AClientSocket::HitPlayer(const int& SessionId)
{
	// 서버에게 데미지를 준 캐릭터 정보 전송
	stringstream SendStream;
	SendStream << EPacketType::HIT_PLAYER << endl;
	SendStream << SessionId << endl;

	int nSendLen = send(
		ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0
	);
}

/*void AClient_State::SetPlayerController(APlayerController* pPlayerController)
{
	// 플레이어 컨트롤러 세팅
	if (pPlayerController)
	{
		PlayerController = pPlayerController;
	}
}*/

void AClientSocket::CloseSocket()
{
	closesocket(ServerSocket);
	WSACleanup();
}

cCharacter::cCharacter()
{
}

cCharacter::~cCharacter()
{
}

cCharactersInfo::cCharactersInfo()
{
}

cCharactersInfo::~cCharactersInfo()
{
}
