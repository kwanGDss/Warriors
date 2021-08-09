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
	// ĳ���� ������ ��� ��ȯ		
	RecvStream >> CharactersInfo;
	return &CharactersInfo;	
}

string* AClientSocket::RecvChat(stringstream& RecvStream)
{
	// ä�� ������ ��� ��ȯ
	RecvStream >> sChat;
	std::replace(sChat.begin(), sChat.end(), '_', ' ');
	return &sChat;
}

cCharacter* AClientSocket::RecvNewPlayer(stringstream& RecvStream)
{
	// �� �÷��̾� ������ ��� ��ȯ
	RecvStream >> NewPlayer;
	return &NewPlayer;
}

bool AClientSocket::Init()
{
	return true;
}

uint32 AClientSocket::Run()
{
	// �ʱ� init ������ ��ٸ�
	FPlatformProcess::Sleep(0.03);	
	// recv while loop ����
	// StopTaskCounter Ŭ���� ������ ����� Thread Safety�ϰ� ����
	/*while (StopTaskCounter.GetValue() == 0 && PlayerController != nullptr)
	{
		stringstream RecvStream;
		int PacketType;
		int nRecvLen = recv(
			ServerSocket, (CHAR*)&recvBuffer, MAX_BUFFER, 0
		);
		if (nRecvLen > 0)
		{
			// ��Ŷ ó��
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
	// thread safety ������ ������ while loop �� ���� ���ϰ� ��
	StopTaskCounter.Increment();
}

void AClientSocket::Exit()
{
}

bool AClientSocket::StartListen()
{
	// ������ ����
	if (Thread != nullptr) return false;
	Thread = FRunnableThread::Create(this, TEXT("ClientSocket"), 0, TPri_BelowNormal);
	return (Thread != nullptr);
}

void AClientSocket::StopListen()
{
	// ������ ����
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
	// ���� ������ 2.2�� �ʱ�ȭ
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nRet != 0) {		
		return false;
	}

	// TCP ���� ����	
	ServerSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);	
	if (ServerSocket == INVALID_SOCKET) {		
		return false;
	}
	
	return true;
}

bool AClientSocket::Connect(const char* pszIP, int nPort)
{
	// ������ ���� ������ ������ ����ü
	SOCKADDR_IN stServerAddr;

	stServerAddr.sin_family = AF_INET;
	// ������ ���� ��Ʈ �� IP
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
	// ȸ������ ������ ������ ������
	SendStream << EPacketType::SIGNUP_PLAYER << endl;
	SendStream << TCHAR_TO_UTF8(*Id.ToString()) << endl;
	SendStream << TCHAR_TO_UTF8(*Pw.ToString()) << endl;

	int nSendLen = send(
		ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0
	);

	if (nSendLen == -1)
		return false;

	// �����κ��� ���� ���
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

	// ȸ������ ���� ������ ��ȯ
	return SignUpResult;
}

bool AClientSocket::Login(const FText& Id, const FText& Pw)
{
	stringstream SendStream;
	// �α��� ������ ������ ������
	SendStream << EPacketType::LOGIN_PLAYER << endl;
	SendStream << TCHAR_TO_UTF8(*Id.ToString()) << endl;
	SendStream << TCHAR_TO_UTF8(*Pw.ToString()) << endl;

	int nSendLen = send(
		ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0
	);

	if (nSendLen == -1)
		return false;
	// �����κ��� ���� ���
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
	// �α��� ���� ������ ��ȯ
	return LoginResult;
}

void AClientSocket::EnrollPlayer(cCharacter& info)
{
	// ĳ���� ���� ����ȭ
	stringstream SendStream;
	// ��û ����
	SendStream << EPacketType::ENROLL_PLAYER << endl;;
	SendStream << info;

	// ĳ���� ���� ����
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
	// ĳ���� ���� ����ȭ
	stringstream SendStream;
	// ��û ����
	SendStream << EPacketType::SEND_PLAYER << endl;;
	SendStream << info;

	// ĳ���� ���� ����
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
	// �������� �α׾ƿ���ų ĳ���� ���� ����
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
	// �������� �������� �� ĳ���� ���� ����
	stringstream SendStream;
	SendStream << EPacketType::HIT_PLAYER << endl;
	SendStream << SessionId << endl;

	int nSendLen = send(
		ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0
	);
}

/*void AClient_State::SetPlayerController(APlayerController* pPlayerController)
{
	// �÷��̾� ��Ʈ�ѷ� ����
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
