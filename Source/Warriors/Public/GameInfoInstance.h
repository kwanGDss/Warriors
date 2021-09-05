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
	int						enemy_id = NOT_INGAME;
	unsigned char			m_prev_recv = 0;
	SOCKETINFO				m_recv_over;
	SOCKET					m_socket = -1;			// -1 : not connect / 1~ : connect 

	mutex					m_lock;
	char					m_name[16];
	float					m_x = 0, m_y = 0;
	float					m_hp = 1.f, m_stamina = 1.f;
	bool					m_guard = false;
	bool					m_parrying = false;
	bool					m_groggy = false;
	bool					m_guard_hit = false;
	bool					m_character_type = 0; // 0 = knight / 1=viking

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
	void set_my_position(float x, float y);

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	FVector2D get_my_position();

	UFUNCTION(BlueprintCallable, Category = "CharacterInfo")
	FVector2D get_enemy_position();

	UFUNCTION(BlueprintCallable, Category = "ProcessSocket")
	void initSocket();

	UFUNCTION(BlueprintCallable, Category = "ProcessSocket")
	void connectSocket();

	void process_login_packet();

	void process_start_packet();

	void process_tick();

	void process_packet();

	void recv_packet();

	void send_packet(void* buf, char packet_type);

	void send_packet_not_recv(void* buf, char packet_type);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_login_packet();

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_change_character_packet();

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_stamina_packet(float reduce_amount);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_attack_packet(float reduce_amount);

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_start_packet();

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_tick_packet();

	UFUNCTION(BlueprintCallable, Category = "SendPacket")
	void send_logout_packet();

	SOCKET serverSocket;

	SOCKETINFO s_wsabuf;
	SOCKETINFO r_wsabuf;

	PLAYERINFO* player = new PLAYERINFO;
	PLAYERINFO* enemy = new PLAYERINFO;
};