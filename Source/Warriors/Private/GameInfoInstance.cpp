// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInfoInstance.h"



UGameInfoInstance::UGameInfoInstance()
{
    char* Server_IP = TCHAR_TO_ANSI(*IPAddress);

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

}

UGameInfoInstance::~UGameInfoInstance()
{
	closesocket(serverSocket);
	WSACleanup();
}

void UGameInfoInstance::Send_Login_Packet()
{
	char* buf = TCHAR_TO_ANSI(*PlayerName);
	unsigned char packet_size = reinterpret_cast<unsigned char*>(buf)[0];
	s_over->packettype = EPacketType::LOGIN_PLAYER;
	memset(&s_over->overlapped, 0, sizeof(s_over->overlapped));
	std::memcpy(s_over->messagebuf, buf, packet_size);
	s_over->wsabuf[0].buf = reinterpret_cast<char*>(s_over->messagebuf);
	s_over->wsabuf[0].len = packet_size;

	WSASend(serverSocket, s_over->wsabuf, 1, 0, 0, &s_over->overlapped, 0);
}

void UGameInfoInstance::Send_Packet()
{
	//unsigned char packet_size = reinterpret_cast<unsigned char*>(buf)[0];
	s_over->packettype = EPacketType::LOGIN_PLAYER;
	memset(&s_over->overlapped, 0, sizeof(s_over->overlapped));
	//std::memcpy(s_over->messagebuf, buf, packet_size);
	s_over->wsabuf[0].buf = reinterpret_cast<char*>(s_over->messagebuf);
	//s_over->wsabuf[0].len = packet_size;

	WSASend(serverSocket, s_over->wsabuf, 1, 0, 0, &s_over->overlapped, 0);
}

void recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	DWORD r_flag = 0;
	//WSARecv(serverSocket, r_wsabuf, 1, 0, &r_flag, over, recv_callback);
}