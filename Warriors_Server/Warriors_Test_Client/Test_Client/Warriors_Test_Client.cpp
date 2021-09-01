#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "..\..\Warriors_Server\Common.h"
#include "..\..\Warriors_Server\Protocol.h"

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")

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

float EnergyValue = 1.f;
float HealthValue = 1.f;

SOCKET serverSocket;

char key_input[1];
char is_cursor_key {0};

int result{0};

SOCKETINFO s_wsabuf;
SOCKETINFO r_wsabuf;


void do_play();
void show_view_map();
void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);

PLAYERINFO *player = new PLAYERINFO;

void setting_map()
{
}

void show_view_map()
{
}


void process_login_packet()
{
	server_packet_login_ok* packet = reinterpret_cast<server_packet_login_ok*>(r_wsabuf.m_wsabuf[0].buf);

	player->id = packet->id;

	player->m_hp = packet->hp;
	player->m_stamina = packet->stamina;
}

void process_update_status()
{
	server_packet_players_status* packet = reinterpret_cast<server_packet_players_status*>(r_wsabuf.m_wsabuf[0].buf);
	player->m_stamina = packet->stamina;
	player->m_hp = packet->health;
}

void process_update_position()
{
	server_packet_move* packet = reinterpret_cast<server_packet_move*>(r_wsabuf.m_wsabuf[0].buf);
	player->m_x = packet->x;
	player->m_y = packet->y;
}

void process_add_obj()
{
}

void process_remove_obj()
{
}

void process_packet()
{
	server_packet_login* ex_over = reinterpret_cast<server_packet_login *>(r_wsabuf.m_buf);

	cout << "recv start" << endl;

	switch(ex_over->type)
	{
	case SERVER_LOGIN_OK:
		cout << "LOGIN_OK" << endl;
		process_login_packet();
		break;
	case SERVER_LOGIN_FAIL:
		cout << "LOGIN_FAIL" << endl;
		break;
	case SERVER_PLAYERS_STATUS:
		cout << "UPDATE_STATUS" << endl;

		break;
	case SERVER_PLAYER_MOVE:
		cout << "UPDATE_POSITION" << endl;
		process_update_position();
		break;
	
	}
}

void recv_packet()
{
	SOCKETINFO& r_over = r_wsabuf;
	memset(&r_over.m_over, 0, sizeof(r_over.m_over));
	r_over.m_wsabuf[0].len = MAX_BUFFER;
	r_over.m_wsabuf[0].buf = reinterpret_cast<CHAR*>(r_over.m_buf);
	DWORD r_flag = 0;
	WSARecv(serverSocket, r_over.m_wsabuf, 1, 0, &r_flag, &r_over.m_over, 0);

	process_packet();
}

void send_packet(void* buf, char packet_type)
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

void send_login_packet()
{
	client_packet_login packet;

	packet.size = sizeof(packet);
	packet.type = CLIENT_LOGIN;
	strcpy_s(packet.name, "BBaKKi");

	SOCKETINFO* s_info = new SOCKETINFO;

	unsigned char packet_size = reinterpret_cast<unsigned char*>(&packet)[0];
	s_info->m_packet_type[0] = TO_SERVER;
	s_info->m_packet_type[1] = CLIENT_LOGIN;
	memset(&s_info->m_over, 0, sizeof(s_info->m_over));
	memcpy(s_info->m_buf, &packet, packet_size);
	s_info->m_wsabuf[0].buf = reinterpret_cast<char*>(s_info->m_buf);
	s_info->m_wsabuf[0].len = packet_size;

	WSASend(serverSocket, s_info->m_wsabuf, 1, 0, 0, &s_info->m_over, send_callback);

	recv_packet();
}

void send_move_packet()
{
	client_packet_move packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_MOVE;
	packet.dir = 3;

	send_packet(&packet, CLIENT_MOVE);
}

void send_attack_packet()
{
	client_packet_attack packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_ATTACK;

	send_packet(&packet, CLIENT_ATTACK);
}


void send_logout_packet()
{
	client_packet_logout packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_LOGOUT;

	send_packet(&packet, CLIENT_LOGOUT);
}

void do_play()
{
	int i = 0;
	while(true)
	{
		if(!i)
		{
			send_move_packet();
			i++;
		}
				
	}
}

void recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
}

void send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
}

int main(void)
{
	std::wcout.imbue(std::locale("korean"));

	s_wsabuf.m_wsabuf[0].buf = key_input;
	s_wsabuf.m_wsabuf[0].len = 1;

	char *server_IP = const_cast<char*>("127.0.0.1");

	WSADATA WSAData;
	if(WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) return 1;

	serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, server_IP, &serverAddr.sin_addr);

	WSAConnect(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr), NULL, NULL, 0, 0);
	
	DWORD r_flag = 0;

	send_login_packet();

	thread play_thread {do_play};
	play_thread.join();

	while (true) SleepEx(100, true);

	closesocket(serverSocket);
	WSACleanup();
	return 0;
}