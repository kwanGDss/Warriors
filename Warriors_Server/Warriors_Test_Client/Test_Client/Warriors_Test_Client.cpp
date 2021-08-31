#pragma once

#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <vector>
#include <thread>
#include "..\..\Warriors_Server\Common.h"
#include "..\..\Warriors_Server\Protocol.h"

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")


// IOCP 家南 备炼眉

struct SOCKETINFO
{
	WSAOVERLAPPED	overlapped;
	WSABUF			wsabuf[1];
	unsigned char	messagebuf[MAX_BUFFER];
	EPacketType		packettype;
	SOCKET			client_socket;
};

float EnergyValue = 1.f;
float HealthValue = 1.f;

SOCKET serverSocket;

WSABUF s_wsabuf[1];
WSABUF r_wsabuf[1];

HANDLE			hIOCP;

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);

void Send_Packet(void* buf)
{
	SOCKETINFO* socketinfo = new SOCKETINFO;

	unsigned char packet_size = reinterpret_cast<unsigned char*>(buf)[0];
	socketinfo->packettype = EPacketType::RECV_PLAYER;
	memset(&socketinfo->overlapped, 0, sizeof(socketinfo->overlapped));
	memcpy(socketinfo->messagebuf, buf, packet_size);
	socketinfo->wsabuf[0].buf = reinterpret_cast<char*>(socketinfo->messagebuf);
	socketinfo->wsabuf[0].len = packet_size;

	WSASend(serverSocket, socketinfo->wsabuf, 1, 0, 0, &socketinfo->overlapped, 0);
}

void Send_Login_Packet(void *buf)
{
	client_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CLIENT_PACKET_LOGIN;
	memcpy_s(&packet.name, sizeof(packet.name), &buf, sizeof(packet.name));
	Send_Packet(&packet);
}

float Reduce_Energy(float UseEnergy)
{
	EnergyValue -= UseEnergy;
	return EnergyValue;
}

float Reduce_Health(float GetDamaged)
{
	HealthValue -= GetDamaged;
	return HealthValue;
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	DWORD r_flag = 0;
	WSARecv(serverSocket, r_wsabuf, 1, 0, &r_flag, over, recv_callback);
}

void process_packet(int p_id, unsigned char* packet);
void disconnect(int p_id);

void WorkerThread()
{
	char* buf = const_cast<char*>("BBaKKi");
	Send_Login_Packet(&buf);
	while (true)
	{
		DWORD num_byte;
		ULONG_PTR i_key;
		WSAOVERLAPPED* over;
		BOOL ret = GetQueuedCompletionStatus(hIOCP, &num_byte, &i_key, &over, INFINITE);
		int key = static_cast<int>(i_key);

		if (FALSE == ret)
		{
			int err = WSAGetLastError();
			printf_s("GQCS {%d} error ", err);
			disconnect(key);
			continue;
		}
		SOCKETINFO* socketinfo = reinterpret_cast<SOCKETINFO*> (over);
		switch (socketinfo->packettype)
		{
		case EPacketType::RECV_PLAYER:
			{
				unsigned char* ps = socketinfo->messagebuf;
				//int remain_data = num_byte + players[key].prev_recv;

				//while (remain_data > 0)
				{
					int packet_size = ps[0];
					//if (packet_size > remain_data) { break; }
					process_packet(key, ps);
					//remain_data -= packet_size;
					ps += packet_size;
				}
				//if (remain_data > 0) { memcpy(socketinfo->messagebuf, ps, remain_data); }
				//players[key].prev_recv = remain_data;
				//do_recv(key);
				break;
			}
		case EPacketType::SEND_PLAYER:
			{
				if (num_byte != socketinfo->wsabuf[0].len)
				{
					disconnect(key);
					delete socketinfo;
				}
				break;
			}
			case EPacketType::LOGIN_PLAYER:
			{
				break;
			}

			default:
				cout << "Unknown Packet Type" << endl;
				break;
		}
	}
}


void disconnect(int p_id)
{
	return;
}

void do_recv(int p_id)
{
	/*SESSION& pl = players[p_id];
	SOCKETINFO& rsocketinfo = pl.recv_over;
	memset(&(rsocketinfo.overlapped), 0, sizeof(rsocketinfo.overlapped));
	rsocketinfo.wsabuf[0].buf = reinterpret_cast<CHAR*>(rsocketinfo.messagebuf) + pl.prev_recv;
	rsocketinfo.wsabuf[0].len = MAX_BUFFER - pl.prev_recv;
	DWORD r_flag = 0;
	WSARecv(pl.socket, rsocketinfo.wsabuf, 1, 0, &r_flag, &rsocketinfo.overlapped, 0);*/
}

void process_packet(int p_id, unsigned char* packet)
{
	client_packet_login* p = reinterpret_cast<client_packet_login*>(packet);
	switch (p->type)
	{
		case CLIENT_PACKET_LOGIN:
		{

			break;
		}
		case C2S_PACKET_MOVE:
		{
			c2s_packet_move* move_packet = reinterpret_cast<c2s_packet_move*>(packet);
			break;
		}
		default:
			cout << "Unknown" << endl;
	}
}


int main(void)
{
	char* Server_IP = const_cast<char*>("127.0.0.1");

	HealthValue = 1.f;
	EnergyValue = 1.f;

	WSADATA WSAData;
	if(WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {printf_s("can't Start up\n");}

	serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	// Completion Port 按眉 积己
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(serverSocket), hIOCP, (DWORD)100000, 0);

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, Server_IP, &serverAddr.sin_addr);

	WSAConnect(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr), NULL, NULL, 0, 0);

	vector<thread> worker_threads;
	worker_threads.emplace_back(WorkerThread);
	for(auto& worker_thread : worker_threads) {worker_thread.join();}

	closesocket(serverSocket);
	WSACleanup();

	return 0;
}