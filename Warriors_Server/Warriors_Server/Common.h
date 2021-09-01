#pragma once

#include <iostream>
#include <map>
#include <mutex>
#include "Protocol.h"

using namespace std;

#define MAX_CLIENTS 100

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
	short					m_x = rand() % 10, m_y = rand() % 10;
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