#pragma once

#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "..\..\Warriors_Server\Common.h"
#include "..\..\Warriors_Server\Protocol.h"

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")


// IOCP 소켓 구조체
struct stSOCKETINFO
{
	WSAOVERLAPPED	overlapped;
	WSABUF			dataBuf;
	SOCKET			socket;
	char			messageBuffer[MAX_BUFFER];
	int				recvBytes;
	int				sendBytes;
};

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
	packet.name = "BBaKKi";
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

int main(void)
{
	char* Server_IP = const_cast<char*>("127.0.0.1");

	HealthValue = 1.f;
	EnergyValue = 1.f;

	WSADATA WSAData;
	if(WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {printf_s("can't Start up\n");}

	serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, Server_IP, &serverAddr.sin_addr);

	WSAConnect(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr), NULL, NULL, 0, 0);

	char* buf = const_cast<char*>("BBaKKi");
	Send_Login_Packet(buf);

	closesocket(serverSocket);
	WSACleanup();

	return 0;
}