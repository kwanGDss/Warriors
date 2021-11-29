// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <MSWSock.h>
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
	int						enemy_id = NOT_INGAME;
	unsigned char			m_prev_recv = 0;
	SOCKETINFO				m_recv_over;
	SOCKET					m_socket = -1;			// -1 : not connect / 1~ : connect 

	char					m_name[16];
	bool					m_be_hit_change = false, m_guard_hit_change = false;
	float					m_hp = 1.f, m_stamina = 1.f;
	bool					m_be_hit = false;
	bool					m_guard = false;
	bool					m_parrying = false;
	bool					m_groggy = false;
	bool					m_guard_hit = false;
	bool					m_character_type = 0; // 0 = knight / 1=viking

	PLAYERINFO& operator = (const PLAYERINFO& Right)
	{
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

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	void set_my_guard(bool guard);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	bool get_my_guard();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	void set_enemy_guard(bool guard);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	bool get_enemy_guard();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	void set_my_parrying(bool parrying);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	bool get_my_parrying();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	void set_enemy_parrying(bool parrying);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	bool get_enemy_parrying();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	void set_my_groggy(bool groggy);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	bool get_my_groggy();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	void set_enemy_groggy(bool groggy);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	bool get_enemy_groggy();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	void set_my_guard_hit(bool guard_hit);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	bool get_my_guard_hit();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	void set_enemy_guard_hit(bool guard_hit);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	bool get_enemy_guard_hit();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	void set_my_character_type(bool character_type);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	bool get_my_character_type();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	void set_enemy_charactor_type(bool character_type);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	bool get_enemy_charactor_type();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	bool get_my_be_hit();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	void set_my_be_hit(bool be_hit);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	int get_my_id();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	bool get_be_hit_packet();

	UFUNCTION(BlueprintCallable, Category = "ProcessSocket")
	void initSocket();

	UFUNCTION(BlueprintCallable, Category = "ProcessSocket")
	void connectSocket();

	void process_login_packet();

	void process_start_packet();

	void process_tick();

	void process_attack();

	void process_packet();

	void recv_packet();

	void send_packet(void* buf, char packet_type);

	void send_packet_not_recv(void* buf, char packet_type);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_login_packet();

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_change_character_packet();

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_reduce_health(float reduce_amount);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_stamina_packet(float reduce_amount);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_attack_packet(float reduce_amount);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_start_packet();

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_be_hit_packet(bool be_hit);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_guard_packet(bool guard);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_parrying_packet(bool parrying);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_groggy_packet(bool id, bool groggy);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_guard_hit_packet(bool whosplayer, bool guard_hit);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_tick_packet();

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_logout_packet();

	bool packet_be_hit = false;

	SOCKET serverSocket;

	SOCKETINFO s_wsabuf;
	SOCKETINFO r_wsabuf;

	PLAYERINFO* player = new PLAYERINFO;
	PLAYERINFO* enemy = new PLAYERINFO;

	PACKETINFO *tick_packet = new PACKETINFO;
};