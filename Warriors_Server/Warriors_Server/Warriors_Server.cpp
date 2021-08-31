#include <WS2tcpip.h>
#include <MSWSock.h>
#include <map>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <unordered_set>
#include "Protocol.h"
#include "Common.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

constexpr char	NUMOFTHREAD		= 8;
int				nThreadCnt		= 0;
HANDLE			hIOCP;
int				nResult			= 0;	

// IOCP 소켓 구조체

struct SOCKETINFO
{
	WSAOVERLAPPED	overlapped;
	WSABUF			wsabuf[1];
	unsigned char	messagebuf[MAX_BUFFER];
	EPacketType		packettype;
	SOCKET			client_socket;
};

struct SESSION
{
	int						id					= 0;
	SOCKETINFO				recv_over;
	unsigned char			prev_recv			= 0;
	SOCKET					socket				= 0;

	S_STATE					state				= S_STATE::STATE_FREE;			//0 : Free, 1 : Connected, 2 : 
	mutex					lock;
	char					name[MAX_NAME]		= {0,};
	short					x = 0, y = 0;
};

SESSION players[MAX_USER]; // Data Race!
SOCKET listenSocket;

void disconnect(int p_id);
void do_recv(int p_id);
int get_new_player_id();
void do_accept(SOCKET s_socket, SOCKETINFO* a_over);

bool Initialize()
{
	WSADATA wsaData;
	int nResult;
	// winsock 2.2 버전으로 초기화
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nResult != 0)
	{
		printf_s("Winsock Init Error\n");
		return false;
	}

	// 소켓 생성
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (listenSocket == INVALID_SOCKET)
	{
		printf_s("Socket Create Error\n");
		return false;
	}

	// 서버 정보 설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 소켓 설정
	// boost bind 와 구별짓기 위해 ::bind 사용
	nResult = ::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	
	if (nResult == SOCKET_ERROR)
	{
		printf_s("Bind Error\n");
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	// 수신 대기열 생성
	nResult = listen(listenSocket, SOMAXCONN);
	if (nResult == SOCKET_ERROR)
	{
		printf_s("Listen Error\n");
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	return true;
}

bool CalculateWorkerThread()
{
	// 시스템 정보 가져옴
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	printf_s("CPU 갯수 : {%d}\n", sysInfo.dwNumberOfProcessors);
	// 적절한 작업 스레드의 갯수는 (CPU * 2) + 1
	nThreadCnt = sysInfo.dwNumberOfProcessors * 2;
	if(!nThreadCnt) {return false;}
	printf_s("Worker Thread Start...\n");
	return true;
}

void StartServer()
{
	// 클라이언트 정보
	int addrLen = sizeof(SOCKADDR_IN);
	SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (clientSocket == SOCKET_ERROR) { return; }

	// Completion Port 객체 생성
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), hIOCP, (DWORD)100000, 0);

	// Worker Thread 생성
	if (!CalculateWorkerThread()) return;
	
	printf_s("Server Start...\n");

	SOCKETINFO a_over;
	a_over.packettype = EPacketType::LOGIN_PLAYER;

	do_accept(listenSocket, &a_over);
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
			disconnect(key);
			continue;
		}

		SOCKETINFO* socketinfo = reinterpret_cast<SOCKETINFO*> (over);
		client_packet_login *cl = reinterpret_cast<client_packet_login*>(socketinfo);
		switch (cl->type)
		{
		case EPacketType::SEND_PLAYER:
			{
				unsigned char* ps = socketinfo->messagebuf;
				int remain_data = num_byte + players[key].prev_recv;

				while (remain_data > 0)
				{
					int packet_size = ps[0];
					if (packet_size > remain_data) { break; }
					process_packet(key, ps);
					remain_data -= packet_size;
					ps += packet_size;
				}
				if (remain_data > 0) { memcpy(socketinfo->messagebuf, ps, remain_data); }
				players[key].prev_recv = remain_data;
				do_recv(key);
				break;
			}
		case EPacketType::RECV_PLAYER:
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
				client_packet_login* ps = reinterpret_cast<client_packet_login*> (socketinfo->messagebuf);
				SOCKET socket = socketinfo->client_socket;
				int p_id = get_new_player_id();
				if (-1 == p_id)
				{
					closesocket(socket);
					do_accept(listenSocket, socketinfo);
					continue;
				}


				SESSION& n_s = players[p_id];

				n_s.lock.lock();
				n_s.state = S_STATE::STATE_CONNECTED;
				n_s.id = p_id;
				n_s.prev_recv = 0;
				n_s.recv_over.packettype = EPacketType::SIGNUP_PLAYER;
				n_s.socket = socket;
				n_s.x = rand() % 10;
				n_s.y = rand() % 10;
				strcpy_s(n_s.name, ps->name);
				n_s.lock.unlock();

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), hIOCP, p_id, 0);

				do_recv(p_id);
				do_accept(listenSocket, socketinfo);
	
				cout << "New player [" << p_id << "] !" << endl;
				break;
			}

		case EPacketType::SIGNUP_PLAYER:
			{
				unsigned char* ps = socketinfo->messagebuf;
				client_packet_login* packet = reinterpret_cast<client_packet_login*>(ps);
				players[key].lock.lock();
				strcpy_s(players[key].name, packet->name);
				cout << packet->name << endl;
				players[key].x = rand() % 10;
				players[key].y = rand() % 10;
				/*sc_packet_login_ok packet;
				packet.size = sizeof(packet);
				packet.type = SC_LOGIN_OK;
				packet.id = p_id;
				packet.x = players[p_id].m_x;
				packet.y = players[p_id].m_y;
				packet.HP = players[p_id].hp;
				packet.LEVEL = players[p_id].level;
				packet.EXP = players[p_id].exp;

				send_packet(p_id, &packet, SC_LOGIN_OK);*/
				players[key].lock.unlock();
				break;
			}

			default:
				cout << "Unknown Packet Type" << endl;
				exit(-1);
		}
	}
}

void send_packet(int p_id, void* buf)
{
	SOCKETINFO* socketinfo = new SOCKETINFO;

	unsigned char packet_size = reinterpret_cast<unsigned char*>(buf)[0];
	socketinfo->packettype = EPacketType::SEND_PLAYER;
	memset(&socketinfo->overlapped, 0, sizeof(socketinfo->overlapped));
	memcpy(socketinfo->messagebuf, buf, packet_size);
	socketinfo->wsabuf[0].buf = reinterpret_cast<char*>(socketinfo->messagebuf);
	socketinfo->wsabuf[0].len = packet_size;

	WSASend(players[p_id].socket, socketinfo->wsabuf, 1, 0, 0, &socketinfo->overlapped, 0);
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

int get_new_player_id()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (listenSocket == i) { continue; }
		players[i].lock.lock();
		if (S_STATE::STATE_FREE == players[i].state)
		{
			
			players[i].state = S_STATE::STATE_CONNECTED;
			players[i].lock.unlock();
			return i;
		}
		players[i].lock.unlock();
	}
	return -1;
}

void SignUp(int p_id)
{

}
/*
void send_login_info(int p_id)
{
	server_packet_login packet;
	packet.hp = 100;
	packet.id = p_id;
	packet.level = 3;
	packet.size = sizeof(packet);
	packet.type = S2C_PACKET_LOGIN_INFO;
	packet.x = players[p_id].x;
	packet.y = players[p_id].y;
	send_packet(p_id, &packet);
}

void send_move_packet(int c_id, int p_id)
{
	s2c_packet_pc_move packet;
	packet.id = p_id;
	packet.size = sizeof(packet);
	packet.type = S2C_PACKET_PC_MOVE;
	packet.x = players[p_id].x;
	packet.y = players[p_id].y;
	send_packet(c_id, &packet);
}

void send_pc_login(int c_id, int p_id)
{
	s2c_packet_pc_login packet;
	packet.id = p_id;
	packet.size = sizeof(packet);
	packet.type = SERVER_PACKET_PC_LOGIN;
	packet.x = players[p_id].x;
	packet.y = players[p_id].y;
	strcpy_s(packet.name, players[p_id].name);
	packet.o_type = 0;
	send_packet(c_id, &packet);
}

void send_pc_logout(int c_id, int p_id)
{
	s2c_packet_pc_logout packet;
	packet.id = p_id;
	packet.size = sizeof(packet);
	packet.type = S2C_PACKET_PC_LOGOUT;
	send_packet(c_id, &packet);
}
*/
void disconnect(int p_id)
{
	players[p_id].lock.lock();
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
	}
}

void do_recv(int p_id)
{
	SESSION& pl = players[p_id];
	SOCKETINFO& rsocketinfo = pl.recv_over;
	memset(&(rsocketinfo.overlapped), 0, sizeof(rsocketinfo.overlapped));
	rsocketinfo.wsabuf[0].buf = reinterpret_cast<CHAR*>(rsocketinfo.messagebuf) + pl.prev_recv;
	rsocketinfo.wsabuf[0].len = MAX_BUFFER - pl.prev_recv;
	DWORD r_flag = 0;

	WSARecv(pl.socket, rsocketinfo.wsabuf, 1, 0, &r_flag, &rsocketinfo.overlapped, 0);
}

void process_packet(int p_id, unsigned char* packet)
{
	client_packet_login* p = reinterpret_cast<client_packet_login*>(packet);
	switch (p->type)
	{
		//case CLIENT_PACKET_LOGIN:
		{
			players[p_id].lock.lock();
			strcpy_s(players[p_id].name, p->name);
			players[p_id].x = rand() % BOARD_WIDTH;
			players[p_id].y = rand() % BOARD_HEIGHT;
			//send_login_info(p_id);
			players[p_id].state = S_STATE::STATE_INGAME;
			players[p_id].lock.unlock();

			for (auto& pl : players)
			{
				if (pl.id == p_id) { continue; }
				if (pl.state != S_STATE::STATE_INGAME) {continue; }
				//send_pc_login(p_id, pl.id);
				//send_pc_login(pl.id, p_id);
			}
			break;
		}
		//case C2S_PACKET_MOVE:
		{
			c2s_packet_move* move_packet = reinterpret_cast<c2s_packet_move*>(packet);
			break;
		}
		default:
			cout << "Unknown" << endl;
	}
}

int main()
{
	/*if (Initialize())
	{
		StartServer();
		vector<thread> worker_threads;
		while(!nThreadCnt){};
		for(int i = 0; i < 1; ++i)
		{
			worker_threads.emplace_back(WorkerThread);
		}
		for(auto& worker_thread : worker_threads) {worker_thread.join();}

		closesocket(listenSocket);
		WSACleanup();
	}
    return 0;*/

	WSADATA wsaData;
	int nResult;
	// winsock 2.2 버전으로 초기화
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nResult != 0)
	{
		printf_s("Winsock Init Error\n");
		exit(-1);
	}

	// 소켓 생성
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (listenSocket == INVALID_SOCKET)
	{
		printf_s("Socket Create Error\n");
		exit(-1);
	}

	// 서버 정보 설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 소켓 설정
	// boost bind 와 구별짓기 위해 ::bind 사용
	nResult = ::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));

	if (nResult == SOCKET_ERROR)
	{
		printf_s("Bind Error\n");
		closesocket(listenSocket);
		WSACleanup();
		return -1;
	}

	// 수신 대기열 생성
	nResult = listen(listenSocket, SOMAXCONN);
	if (nResult == SOCKET_ERROR)
	{
		printf_s("Listen Error\n");
		closesocket(listenSocket);
		WSACleanup();
		return -1;
	}

	// Completion Port 객체 생성
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), hIOCP, (DWORD)100000, 0);

	SOCKETINFO a_over;
	a_over.packettype = EPacketType::LOGIN_PLAYER;
	//do_accept(listenSocket, &a_over);

	// 클라이언트 정보
	int addrLen = sizeof(SOCKADDR_IN) + 16;
	DWORD num_byte;
	SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(clientSocket == SOCKET_ERROR) {return -1;}

	memset(&a_over.overlapped, 0, sizeof(a_over.overlapped));
	a_over.client_socket = clientSocket;

	BOOL result = AcceptEx(listenSocket, clientSocket, a_over.messagebuf, 0, addrLen, addrLen, &num_byte, &a_over.overlapped);

	if (FALSE == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			cout << "Accept error" << endl;
			exit(-1);
		}
	}

	printf_s("Server Start...\n");

	// Worker Thread 생성
	if (!CalculateWorkerThread()) return 0;

	vector<thread> worker_threads;
	while (!nThreadCnt) {};
	for (int i = 0; i < 1; ++i)
	{
		worker_threads.emplace_back(WorkerThread);
	}
	for (auto& worker_thread : worker_threads) { worker_thread.join(); }

	closesocket(listenSocket);
	WSACleanup();
	return 0;
}