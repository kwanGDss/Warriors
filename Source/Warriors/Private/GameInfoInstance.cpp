// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInfoInstance.h"



UGameInfoInstance::UGameInfoInstance()
{
}

UGameInfoInstance::~UGameInfoInstance()
{
	closesocket(serverSocket);
	WSACleanup();
}

float UGameInfoInstance::reduce_stamina(float reduce_amount)
{
	send_stamina_packet(reduce_amount);
	return player->m_stamina;
}

float UGameInfoInstance::increase_stamina(float increase_amount)
{
	float reduce_amount = -increase_amount;
	send_stamina_packet(reduce_amount);
	return player->m_stamina;
}

float UGameInfoInstance::reduce_health(float reduce_amount)
{
	player->m_hp -= reduce_amount;
	send_reduce_health(reduce_amount);
	return player->m_hp;
}

float UGameInfoInstance::get_my_stamina()
{
	return player->m_stamina;
}

void UGameInfoInstance::set_my_stamina(float increase_amount)
{
	float reduce_amount = -increase_amount;
	send_stamina_packet(reduce_amount);
}

float UGameInfoInstance::get_my_health()
{
	return player->m_hp;
}

float UGameInfoInstance::get_enemy_health()
{
	return enemy->m_hp;
}

void UGameInfoInstance::set_my_guard(bool guard)
{
	player->m_guard = guard;
	send_guard_packet(guard);
}

bool UGameInfoInstance::get_my_guard()
{
	return player->m_guard;
}

void UGameInfoInstance::set_enemy_guard(bool guard)
{
	enemy->m_guard = guard;
}

bool UGameInfoInstance::get_enemy_guard()
{
	return enemy->m_guard;
}

void UGameInfoInstance::set_my_parrying(bool parrying)
{
	player->m_parrying = parrying;
	send_parrying_packet(parrying);
}

bool UGameInfoInstance::get_my_parrying()
{
	return player->m_parrying;
}

void UGameInfoInstance::set_enemy_parrying(bool parrying)
{
	enemy->m_parrying = parrying;
}

bool UGameInfoInstance::get_enemy_parrying()
{
	return enemy->m_parrying;
}

void UGameInfoInstance::set_my_groggy(bool groggy)
{
	player->m_groggy = groggy;
	send_groggy_packet(0, groggy);
}

bool UGameInfoInstance::get_my_groggy()
{
	return player->m_groggy;
}

void UGameInfoInstance::set_enemy_groggy(bool groggy)
{
	enemy->m_groggy = groggy;
	send_groggy_packet(1, groggy);
}

bool UGameInfoInstance::get_enemy_groggy()
{
	return enemy->m_groggy;
}

void UGameInfoInstance::set_my_guard_hit(bool guard_hit)
{
	player->m_guard_hit = guard_hit;
	send_guard_hit_packet(0, guard_hit);
}

bool UGameInfoInstance::get_my_guard_hit()
{
	return player->m_guard_hit;
}

void UGameInfoInstance::set_enemy_guard_hit(bool guard_hit)
{
	enemy->m_guard_hit = guard_hit;
	send_guard_hit_packet(1, guard_hit);
}

bool UGameInfoInstance::get_enemy_guard_hit()
{
	return enemy->m_guard_hit;
}

void UGameInfoInstance::set_my_character_type(bool character_type)
{
	player->m_character_type = character_type;
}

bool UGameInfoInstance::get_my_character_type()
{
	return player->m_character_type;
}

void UGameInfoInstance::set_enemy_charactor_type(bool character_type)
{
	enemy->m_character_type = character_type;
}

bool UGameInfoInstance::get_enemy_charactor_type()
{
	return enemy->m_character_type;
}

void UGameInfoInstance::set_my_position(float x, float y)
{
	player->m_x = x;
	player->m_y = y;
}

bool UGameInfoInstance::get_my_be_hit()
{
	return player->m_be_hit;
}

void UGameInfoInstance::set_my_be_hit(bool be_hit)
{
	player->m_be_hit = be_hit;
	send_be_hit_packet(be_hit);
}

FVector2D UGameInfoInstance::get_my_position()
{
	FVector2D player_position;
	player_position.X = player->m_x;
	player_position.Y = player->m_y;
	return player_position;
}

FVector2D UGameInfoInstance::get_enemy_position()
{
	FVector2D enemy_position;
	enemy_position.X = enemy->m_x;
	enemy_position.Y = enemy->m_y;
	return enemy_position;
}

void UGameInfoInstance::initSocket()
{
	char* Server_IP = TCHAR_TO_ANSI(*IPAddress);

	char* server_IP = const_cast<char*>("127.0.0.1");

	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) printf_s("Can't Start WSA");

	serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
}

void UGameInfoInstance::connectSocket()
{
	char* Server_IP = TCHAR_TO_ANSI(*IPAddress);
	char* server_IP = const_cast<char*>("127.0.0.1");
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, Server_IP, &serverAddr.sin_addr);
	
	WSAConnect(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr), NULL, NULL, 0, 0);
}

void UGameInfoInstance::process_login_packet()
{
	server_packet_login_ok* packet = reinterpret_cast<server_packet_login_ok*>(r_wsabuf.m_wsabuf[0].buf);

	player->id = packet->id;
	player->m_hp = packet->hp;
	player->m_stamina = packet->stamina;
	player->enemy_id = packet->enemy_id;
}

void UGameInfoInstance::process_start_packet()
{
	server_packet_start* packet = reinterpret_cast<server_packet_start*>(r_wsabuf.m_wsabuf[0].buf);

	enemy->m_character_type = packet->enemy_character_type;
}

void UGameInfoInstance::process_tick()
{
	server_packet_tick* packet = reinterpret_cast<server_packet_tick*>(r_wsabuf.m_wsabuf[0].buf);

	player->m_hp = packet->player_hp;
	player->m_stamina = packet->player_stamina;
	player->m_be_hit = packet->player_be_hit;
	player->m_guard_hit = packet->player_guard_hit;
	enemy->m_x = packet->enemy_x;
	enemy->m_y = packet->enemy_y;
	enemy->m_hp = packet->enemy_hp;
	enemy->m_guard = packet->enemy_guard;
	enemy->m_parrying = packet->enemy_parrying;
	enemy->m_groggy = packet->enemy_groggy;
	enemy->m_guard_hit = packet->enemy_guard_hit;

	/*if (player->m_be_hit)
	{
		send_be_hit_packet(false);
	}
	if (player->m_guard_hit)
	{
		send_guard_hit_packet(false, false);
	}*/
}

void UGameInfoInstance::process_attack()
{
	server_packet_attack* packet = reinterpret_cast<server_packet_attack*>(r_wsabuf.m_wsabuf[0].buf);

	if (!(packet->enemy_be_hit))
	{
		send_attack_packet(0.2f);
	}
}

void UGameInfoInstance::process_packet()
{
	server_packet_login* ex_over = reinterpret_cast<server_packet_login*>(r_wsabuf.m_buf);

	cout << "recv start" << endl;

	switch (ex_over->type)
	{
	case SERVER_LOGIN_OK:
		process_login_packet();
		break;
	case SERVER_LOGIN_FAIL:
		break;
	case SERVER_START:
		process_start_packet();
		break;
	case SERVER_TICK:
		process_tick();
		break;
	case SERVER_ATTACK:
		process_attack();
		break;
	}
}

void UGameInfoInstance::recv_packet()
{
	SOCKETINFO& r_over = r_wsabuf;
	memset(&r_over, 0, sizeof(r_over));
	r_over.m_wsabuf[0].len = MAX_BUFFER;
	r_over.m_wsabuf[0].buf = reinterpret_cast<CHAR*>(r_over.m_buf);
	DWORD r_flag = 0;

	WSARecv(serverSocket, r_over.m_wsabuf, 1, 0, &r_flag, &r_over.m_over, 0);
	//WSARecv(serverSocket, r_over.m_wsabuf, 1, 0, &r_flag, 0, 0);

	process_packet();
}

void UGameInfoInstance::send_packet(void* buf, char packet_type)
{
	SOCKETINFO* s_info = new SOCKETINFO;

	unsigned char packet_size = reinterpret_cast<unsigned char*>(buf)[0];
	s_info->m_packet_type[0] = TO_SERVER;
	s_info->m_packet_type[1] = packet_type;
	memset(&s_info->m_over, 0, sizeof(s_info->m_over));
	memcpy(s_info->m_buf, buf, packet_size);
	s_info->m_wsabuf[0].buf = reinterpret_cast<char*>(s_info->m_buf);
	s_info->m_wsabuf[0].len = packet_size;

	WSASend(serverSocket, s_info->m_wsabuf, 1, 0, 0, &s_info->m_over, 0);

	recv_packet();

	delete(s_info);
}

void UGameInfoInstance::send_packet_not_recv(void* buf, char packet_type)
{
	SOCKETINFO* s_info = new SOCKETINFO;

	unsigned char packet_size = reinterpret_cast<unsigned char*>(buf)[0];
	s_info->m_packet_type[0] = TO_SERVER;
	s_info->m_packet_type[1] = packet_type;
	memset(&s_info->m_over, 0, sizeof(s_info->m_over));
	memcpy(s_info->m_buf, buf, packet_size);
	s_info->m_wsabuf[0].buf = reinterpret_cast<char*>(s_info->m_buf);
	s_info->m_wsabuf[0].len = packet_size;

	WSASend(serverSocket, s_info->m_wsabuf, 1, 0, 0, &s_info->m_over, 0);

	delete(s_info);
}

void UGameInfoInstance::send_login_packet()
{
	char* playername = TCHAR_TO_ANSI(*Player_Name);
	client_packet_login packet;

	packet.size = sizeof(packet);
	packet.type = CLIENT_LOGIN;
	strcpy_s(packet.name, playername);

	send_packet(&packet, CLIENT_LOGIN);
}

void UGameInfoInstance::send_change_character_packet()
{
	client_packet_change_character packet;

	packet.size = sizeof(packet);
	packet.type = CLIENT_CHANGE_CHARACTER;
	packet.change_character = player->m_character_type;

	send_packet_not_recv(&packet, CLIENT_CHANGE_CHARACTER);
}

void UGameInfoInstance::send_reduce_health(float reduce_amount)
{
	client_packet_reduce_health packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_ATTACK;
	packet.id = player->id;
	packet.reduce_health = reduce_amount;

	send_packet_not_recv(&packet, CLIENT_ATTACK);
}

void UGameInfoInstance::send_stamina_packet(float reduce_amount)
{
	client_packet_reduce_stamina packet;

	packet.size = sizeof(packet);
	packet.type = CLIENT_REDUCE_STAMINA;
	packet.id = player->id;
	packet.reduce_stamina = reduce_amount;

	send_packet_not_recv(&packet, CLIENT_REDUCE_STAMINA);
}

void UGameInfoInstance::send_attack_packet(float reduce_amount)
{
	client_packet_reduce_health packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_ATTACK;
	packet.reduce_health = reduce_amount;

	send_packet_not_recv(&packet, CLIENT_ATTACK);
}

void UGameInfoInstance::send_start_packet()
{
	client_packet_start packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_START;
	packet.id = player->id;
	packet.character_type = player->m_character_type;

	send_packet(&packet, CLIENT_START);
}

void UGameInfoInstance::send_be_hit_packet(bool be_hit)
{
	client_packet_be_hit packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_BE_HIT;
	packet.be_hit = be_hit;

	send_packet_not_recv(&packet, CLIENT_BE_HIT);

}

void UGameInfoInstance::send_guard_packet(bool guard)
{
	client_packet_guard packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_GUARD;
	packet.guard = guard;

	send_packet_not_recv(&packet, CLIENT_GUARD);
}

void UGameInfoInstance::send_parrying_packet(bool parrying)
{
	client_packet_parrying packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_PARRYING;
	packet.parrying = parrying;

	send_packet_not_recv(&packet, CLIENT_PARRYING);
}

void UGameInfoInstance::send_groggy_packet(bool id, bool groggy)
{
	client_packet_groggy packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_GROGGY;
	packet.groggy = groggy;
	packet.id = id;

	send_packet_not_recv(&packet, CLIENT_GROGGY);
}

void UGameInfoInstance::send_guard_hit_packet(bool whosplayer, bool guard_hit)
{
	client_packet_guard_hit packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_GUARD_HIT;
	if(!whosplayer)
	{
		packet.id = player->id;
		packet.guard_hit = guard_hit;
	}
	else
	{
		packet.id = player->enemy_id;
		packet.guard_hit = guard_hit;
	}
	send_packet_not_recv(&packet, CLIENT_GUARD_HIT);
}

void UGameInfoInstance::send_tick_packet()
{
	client_packet_tick packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_TICK;
	packet.x = player->m_x;
	packet.y = player->m_y;

	send_packet(&packet, CLIENT_TICK);
}

void UGameInfoInstance::send_logout_packet()
{
	player->m_hp = 1.f;
	player->m_stamina = 1.f;
	enemy->m_hp = 1.f;
	enemy->m_stamina = 1.f;
	client_packet_logout packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_LOGOUT;

	send_packet_not_recv(&packet, CLIENT_LOGOUT);
}