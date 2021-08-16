// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "..\..\..\Warriors_Server\Warriors_Server\Common.h"
#include "..\..\..\Warriors_Server\Warriors_Server\Protocol.h"
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameInfoInstance.generated.h"

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")

/**
 * 
 */

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
	SOCKET			client_socket;
};

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);

UCLASS()
class WARRIORS_API UGameInfoInstance : public UGameInstance
{
	GENERATED_BODY()


public:
	UGameInfoInstance();
	~UGameInfoInstance();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	FString IPAddress;

	// 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	float HealthValue;

	// 에너지
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	float EnergyValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	FString PlayerName;

	UFUNCTION(BlueprintCallable, Category = "Properties")
	void Send_Login_Packet();

	UFUNCTION(BlueprintCallable, Category = "Properties")
	void Send_Packet();

	SOCKET serverSocket;

	WSABUF s_wsabuf[1];
	WSABUF r_wsabuf[1];

	SOCKETINFO* s_over = new SOCKETINFO;
};
