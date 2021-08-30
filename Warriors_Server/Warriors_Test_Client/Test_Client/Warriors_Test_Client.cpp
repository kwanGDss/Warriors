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

void WorkerThread()
{
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
			//disconnect(key);
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
					//disconnect(key);
					delete socketinfo;
				}
				break;
			}
			case EPacketType::LOGIN_PLAYER:
			{
				char* ps = reinterpret_cast<char*> (socketinfo->messagebuf);
				SOCKET socket = socketinfo->client_socket;
				//int p_id = get_new_player_id();
				//if (-1 == p_id)
				{
					closesocket(socket);
					//do_accept(listenSocket, socketinfo);
					continue;
				}


				/*SESSION& n_s = players[p_id];

				n_s.lock.lock();
				n_s.state = S_STATE::STATE_CONNECTED;
				n_s.id = p_id;
				n_s.prev_recv = 0;
				n_s.socket = socket;
				n_s.x = rand() % 10;
				n_s.y = rand() % 10;
				memcpy_s(n_s.name, sizeof(ps), ps, sizeof(ps));
				n_s.lock.unlock();*/

				//CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), hIOCP, p_id, 0);
				
				//printf_s("%s", n_s.name);

				//do_recv(p_id);
				//do_accept(listenSocket, socketinfo);
	
				cout << "New player [" << ps << "] !" << endl;
				break;
			}

			default:
				cout << "Unknown Packet Type" << endl;
				exit(-1);
		}
	}
}

void do_accept(SOCKET s_socket, SOCKETINFO* a_over)
{
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&a_over->overlapped, 0, sizeof(a_over->overlapped));
	DWORD num_byte;
	int addr_size = sizeof(SOCKADDR_IN) + 16;
	a_over->client_socket = c_socket;
	BOOL ret = AcceptEx(s_socket, c_socket, a_over->messagebuf, 0, addr_size, addr_size, &num_byte, &a_over->overlapped);
	if (FALSE == ret)
	{
		int err = WSAGetLastError();
		if (WSA_IO_PENDING != err)
		{
			printf_s("Accept : {%d}", err);
			exit(-1);
		}
	}
}

void disconnect(int p_id)
{
	/*players[p_id].lock.lock();
	players[p_id].state = S_STATE::STATE_CONNECTED;
	closesocket(players[p_id].socket);
	players[p_id].state = S_STATE::STATE_FREE;
	players[p_id].lock.unlock();
	for (auto& cl : players)
	{
		cl.lock.lock();
		if (S_STATE::STATE_INGAME != cl.state) 
		{
			cl.lock.unlock();
			continue; 
		}
		//send_pc_logout(cl.id, p_id);
		cl.lock.unlock();
	}*/
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
			/*players[p_id].lock.lock();
			strcpy_s(players[p_id].name, p->name);
			players[p_id].x = rand() % BOARD_WIDTH;
			players[p_id].y = rand() % BOARD_HEIGHT;
			send_login_info(p_id);
			players[p_id].state = S_STATE::STATE_INGAME;
			players[p_id].lock.unlock();

			for (auto& pl : players)
			{
				if (pl.id == p_id) { continue; }
				if (pl.state != S_STATE::STATE_INGAME) {continue; }
				send_pc_login(p_id, pl.id);
				send_pc_login(pl.id, p_id);
			}*/
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