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
struct SOCKETINFO
{
	WSAOVERLAPPED	m_over;
	WSABUF			m_wsabuf[1];
	char			m_packet_type[2];
	SOCKET			m_clientsocket;
	unsigned char	m_buf[1024];
};

struct PLAYERINFO
{
	int						id = NOT_INGAME;				// -1 : not ingame / 1 : ingame
	unsigned char			m_prev_recv = 0;
	SOCKETINFO				m_recv_over;
	SOCKET					m_socket = -1;			// -1 : not connect / 1~ : connect 

	mutex					m_lock;
	char					m_name[16];
	int						m_x = rand() % 10, m_y = rand() % 10;
	float					m_hp = 1.f, m_stamina = 1.f;

	PLAYERINFO& operator = (const PLAYERINFO& Right)
	{
		m_x = Right.m_x;
		m_y = Right.m_y;
		m_hp = Right.m_hp;
		m_stamina = Right.m_stamina;
		strcpy_s(m_name, Right.m_name);

		return *this;
	}
};

UCLASS()
class WARRIORS_API UGameInfoInstance : public UGameInstance
{
	GENERATED_BODY()


public:
	UGameInfoInstance();
	~UGameInfoInstance();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ServerInfo")
	FString IPAddress;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ServerInfo")
	FString Player_Name;

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	float reduce_stamina(float reduce_amount);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	float increase_stamina(float increase_amount);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	float reduce_health(float reduce_amount);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	float get_my_stamina();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	void set_my_stamina(float increase_amount);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	float get_my_health();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	float get_enemy_health();

	UFUNCTION(BlueprintCallable, Category = "ProcessSocket")
	void initSocket();

	UFUNCTION(BlueprintCallable, Category = "ProcessSocket")
	void connectSocket();

	UFUNCTION(BlueprintCallable, Category = "ProcessPacket")
	void process_login_packet();

	UFUNCTION(BlueprintCallable, Category = "ProcessPacket")
	void process_update_status();

	UFUNCTION(BlueprintCallable, Category = "ProcessPacket")
	void process_update_position();

	UFUNCTION(BlueprintCallable, Category = "ProcessPacket")
	void process_packet();

	UFUNCTION(BlueprintCallable, Category = "RecvPacket")
	void recv_packet();

	void send_packet(void* buf, char packet_type);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_login_packet();

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_move_packet();

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_attack_packet();

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_logout_packet();

	UFUNCTION(BlueprintCallable, Category = "MainLoop")
	void do_play();

	SOCKET serverSocket;

	SOCKETINFO s_wsabuf;
	SOCKETINFO r_wsabuf;

	PLAYERINFO* player = new PLAYERINFO;
	PLAYERINFO* enemy = new PLAYERINFO;
};