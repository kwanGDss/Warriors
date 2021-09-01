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


float EnergyValue = 1.f;
float HealthValue = 1.f;

SOCKET serverSocket;

char key_input[1];
char is_cursor_key {0};

int result{0};

SOCKETINFO s_wsabuf;
SOCKETINFO r_wsabuf;

SOCKET serverSocket;

void do_play();
void view_world_map();
void show_view_map();
void process_postion_packet();
void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);

PLAYERINFO *player = new PLAYERINFO;

void setting_map()
{
}

void show_view_map()
{
}

void process_postion_packet()
{
	server_packet_move* packet = reinterpret_cast<server_packet_move*>(r_wsabuf.m_wsabuf[0].buf);
	player->m_x = packet->x;
	player->m_y = packet->y;
}

void process_login_packet()
{
	server_packet_login_ok* packet = reinterpret_cast<server_packet_login_ok*>(r_wsabuf.m_wsabuf[0].buf);

	player->id = packet->id;

	player->m_hp = packet->hp;
	player->m_stamina = packet->stamina;
}

void process_add_obj()
{
	sc_packet_add_object* packet = reinterpret_cast<sc_packet_add_object*>(r_wsabuf.m_wsabuf[0].buf);

	Obj obj;

	obj.id = packet->id;
	obj.obj_class = packet->obj_class;
	obj.x_locate = packet->x;
	obj.y_locate = packet->y;

	obj.HP = packet->HP;
	obj.LEVEL = packet->LEVEL;
	obj.EXP = packet->EXP;

	strcpy_s(obj.name, packet->name);

	Others.insert({obj.id, obj});
}

void process_remove_obj()
{
	sc_packet_remove_object* packet = reinterpret_cast<sc_packet_remove_object*>(r_wsabuf.m_wsabuf[0].buf);

	Others.erase((int)(packet->id));
}

void process_packet()
{
	sc_packet_add_object* ex_over = reinterpret_cast<sc_packet_add_object *>(r_wsabuf.m_buf);

	cout << "recv start" << endl;

	switch(ex_over->type)
	{
	case SC_LOGIN_OK:
		cout << "SC_LOGIN_OK" << endl;
		process_login_packet();
		break;
	case SC_LOGIN_FAIL:
		cout << "SC_LOGIN_FAIL" << endl;
		break;
	case SC_POSITION:
		cout << "SC_POSITION" << endl;
		process_postion_packet();
		break;
	case SC_CHAT:
		cout << "SC_CHAT" << endl;
		break;
	case SC_STAT_CHANGE:
		cout << "SC_STAT_CHANGE" << endl;
		break;
	case SC_REMOVE_OBJECT:
		cout << "SC_REMOVE_OBJECT" << endl;
		process_remove_obj();
		break;
	case SC_ADD_OBJECT:
		cout << "SC_ADD_OBJECT" << endl;
		process_add_obj();
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
	cs_packet_login packet;

	packet.size = sizeof(packet);
	packet.type = CS_LOGIN;
	strcpy_s(packet.player_id, "BBaKKi");

	SOCKETINFO* s_info = new SOCKETINFO;

	unsigned char packet_size = reinterpret_cast<unsigned char*>(&packet)[0];
	s_info->m_packet_type[0] = TO_SERVER;
	s_info->m_packet_type[1] = CS_LOGIN;
	memset(&s_info->m_over, 0, sizeof(s_info->m_over));
	memcpy(s_info->m_buf, &packet, packet_size);
	s_info->m_wsabuf[0].buf = reinterpret_cast<char*>(s_info->m_buf);
	s_info->m_wsabuf[0].len = packet_size;

	WSASend(serverSocket, s_info->m_wsabuf, 1, 0, 0, &s_info->m_over, send_callback);

	recv_packet();
}

void send_move_packet(char key_input)
{
	cs_packet_move packet;
	packet.size = sizeof(packet);
	packet.type = CS_MOVE;
	switch(key_input)
	{
	case UP:
	{
		packet.direction = 0;
		break;
	}
	case DOWN:
	{
		packet.direction = 1;
		break;
	}
	case LEFT:
	{
		packet.direction = 2;
		break;
	}
	case RIGHT:
	{
		packet.direction = 3;
		break;
	}
	}
	packet.move_time = 1;

	send_packet(&packet, CS_MOVE);
}

void send_attack_packet()
{
	cs_packet_attack packet;
	packet.size = sizeof(packet);
	packet.type = CS_ATTACK;

	send_packet(&packet, CS_ATTACK);
}

void send_chat_packet(char *mess)
{
	cs_packet_chat packet;
	packet.size = sizeof(packet);
	packet.type = CS_CHAT;
	strcpy_s(packet.message, mess);

	send_packet(&packet, CS_CHAT);
}

void send_logout_packet()
{
	cs_packet_logout packet;
	packet.size = sizeof(packet);
	packet.type = CS_LOGOUT;

	send_packet(&packet, CS_LOGOUT);
}

void send_teleport_packet()
{
	cs_packet_teleport packet;
	packet.size = sizeof(packet);
	packet.type = CS_TELEPORT;

	send_packet(&packet, CS_TELEPORT);
}

void do_play()
{
	while(true)
	{
		show_view_map();
		if(_kbhit() && (is_cursor_key = _getch()))
		{
			if(is_cursor_key == -32)
			{
				key_input[0] = _getch();
				send_move_packet(key_input[0]);
				key_input[0] = '0';
			}
			else if(is_cursor_key == 'q' || is_cursor_key == 'Q') 
			{
				//
			}
		}
	}
}

void view_world_map()
{
	
}

void recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	exit(-1);
}

void send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	

}

int main(void)
{
	std::wcout.imbue(std::locale("korean"));

	key_input[0] = '0';

	s_wsabuf.m_wsabuf[0].buf = key_input;
	s_wsabuf.m_wsabuf[0].len = 1;

	char server_IP[15];
	char user_ID[20];

	cout << "Enter Server IP : ";
	cin >> server_IP;

	cout << "Enter Your ID : ";
	cin >> user_ID;

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
	memset(&s_over, 0, sizeof(s_over));

	send_login_packet();

	thread play_thread {do_play};
	play_thread.join();

	while (true) SleepEx(100, true);

	closesocket(serverSocket);
	WSACleanup();
	return 0;
}