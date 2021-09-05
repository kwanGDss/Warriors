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
	//player->m_stamina -= reduce_amount;
	send_stamina_packet(reduce_amount);
	return player->m_stamina;
}

float UGameInfoInstance::increase_stamina(float increase_amount)
{
	//player->m_stamina += increase_amount;
	float reduce_amount = -increase_amount;
	send_stamina_packet(reduce_amount);
	return player->m_stamina;
}

float UGameInfoInstance::reduce_health(float reduce_amount)
{
	player->m_hp -= reduce_amount;
	send_attack_packet(reduce_amount);
	return player->m_hp;
}

float UGameInfoInstance::get_my_stamina()
{
	return player->m_stamina;
}

void UGameInfoInstance::set_my_stamina(float increase_amount)
{
	//player->m_stamina += increase_amount;
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

void UGameInfoInstance::process_tick()
{
	server_packet_tick* packet = reinterpret_cast<server_packet_tick*>(r_wsabuf.m_wsabuf[0].buf);

	player->m_x = packet->player_x;
	player->m_y = packet->player_y;
	player->m_hp = packet->player_hp;
	player->m_stamina = packet->player_stamina;

	enemy->m_x = packet->enemy_x;
	enemy->m_y = packet->enemy_y;
	enemy->m_hp = packet->enemy_hp;
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
	case SERVER_TICK:
		process_tick();
		break;
	}
}

void UGameInfoInstance::recv_packet()
{
	SOCKETINFO& r_over = r_wsabuf;
	memset(&r_over.m_over, 0, sizeof(r_over.m_over));
	r_over.m_wsabuf[0].len = MAX_BUFFER;
	r_over.m_wsabuf[0].buf = reinterpret_cast<CHAR*>(r_over.m_buf);
	DWORD r_flag = 0;

	WSARecv(serverSocket, r_over.m_wsabuf, 1, 0, &r_flag, &r_over.m_over, 0);

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
	packet.id = player->id;
	packet.reduce_health = reduce_amount;

	send_packet_not_recv(&packet, CLIENT_ATTACK);
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
	client_packet_logout packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_LOGOUT;

	send_packet_not_recv(&packet, CLIENT_LOGOUT);
}