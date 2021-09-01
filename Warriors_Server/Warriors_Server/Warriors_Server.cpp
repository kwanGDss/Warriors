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

void disconnect(int p_id);
void do_recv(int p_id);
int get_new_player_id();
void do_accept(SOCKET s_socket, SOCKETINFO* a_over);

PLAYERINFO players[MAX_USER]; // Data Race!
SOCKET listenSocket;
HANDLE h_iocp;

void send_packet(int p_id, void* buf, char packet_type)
{
	SOCKETINFO* s_info = new SOCKETINFO;

	unsigned char packet_size = reinterpret_cast<unsigned char*>(buf)[0];
	s_info->m_packet_type[0] = TO_CLIENT;
	s_info->m_packet_type[1] = packet_type;
	memset(&s_info->m_over, 0, sizeof(s_info->m_over));
	memcpy(s_info->m_buf, buf, packet_size);
	s_info->m_wsabuf[0].buf = reinterpret_cast<char*>(s_info->m_buf);
	s_info->m_wsabuf[0].len = packet_size;

	WSASend(players[p_id].m_socket, s_info->m_wsabuf, 1, 0, 0, &s_info->m_over, 0);
}

void send_packet_to_server(int p_id, void* buf, char packet_type)
{
	SOCKETINFO* s_info = new SOCKETINFO;

	unsigned char packet_size = reinterpret_cast<unsigned char*>(buf)[0];
	s_info->m_packet_type[0] = TO_CLIENT;
	s_info->m_packet_type[1] = packet_type;
	memset(&s_info->m_over, 0, sizeof(s_info->m_over));
	memcpy(s_info->m_buf, buf, packet_size);
	s_info->m_wsabuf[0].buf = reinterpret_cast<char*>(s_info->m_buf);
	s_info->m_wsabuf[0].len = packet_size;

	WSASend(players[p_id].m_socket, s_info->m_wsabuf, 1, 0, 0, &s_info->m_over, 0);
	WSASend(listenSocket, s_info->m_wsabuf, 1, 0, 0, &s_info->m_over, 0);
}

void send_login_ok(int p_id)
{
	server_packet_login_ok packet;
	packet.size = sizeof(packet);
	packet.type = SERVER_LOGIN_OK;
	packet.id = p_id;
	packet.hp = players[p_id].m_hp;
	packet.stamina = players[p_id].m_stamina;

	send_packet(p_id, &packet, SERVER_LOGIN_OK);
}

void send_login_fail(int p_id)
{
	server_packet_login_fail packet;
	packet.size = sizeof(packet);
	packet.type = SERVER_LOGIN_FAIL;

	send_packet(p_id, &packet, SERVER_LOGIN_FAIL);
}

void send_players_status(int dest_id, int sour_id)
{
	server_packet_players_status packet;
	packet.size = sizeof(packet);
	packet.type = SERVER_PLAYERS_STATUS;
	packet.id = sour_id;
	packet.stamina = players[dest_id].m_stamina;
	packet.health = players[dest_id].m_hp;

	send_packet(dest_id, &packet, SERVER_PLAYERS_STATUS);
}

void send_position_change(int p_id)
{
	server_packet_move packet;
	packet.size = sizeof(packet);
	packet.type = SERVER_PLAYER_MOVE;
	packet.id = p_id;
	packet.x = players[p_id].m_x;
	packet.y = players[p_id].m_y;

	send_packet(p_id, &packet, SERVER_PLAYER_MOVE);
}

void setting_viewport(int p_id)
{
}

void player_move(int p_id, char dir)
{
	short x = players[p_id].m_x;
	short y = players[p_id].m_y;
	switch (dir)
	{
		case 0:
			if (y > 0) y--; break;
		case 1:
			if (y < (/*WORLD_HEIGHT*/ - 1)) y++; break;
		case 2:
			if (x > 0) x--; break;
		case 3:
			if (x < /*WORLD_WIDTH*/ - 1) x++; break;
	}

	players[p_id].m_x = x;
	players[p_id].m_y = y;

	cout << "( " << x << " , " << y << " ) " << endl;

	send_position_change(p_id);
}

void do_recv(int p_id)
{
	PLAYERINFO& pl = players[p_id];
	SOCKETINFO& r_over = pl.m_recv_over;
	memset(&r_over.m_over, 0, sizeof(r_over.m_over));
	r_over.m_wsabuf[0].buf = reinterpret_cast<CHAR*>(r_over.m_buf) + pl.m_prev_recv;
	r_over.m_wsabuf[0].len = 4096 - pl.m_prev_recv;
	DWORD r_flag = 0;

	WSARecv(pl.m_socket, r_over.m_wsabuf, 1, 0, &r_flag, &r_over.m_over, 0);
}

void process_packet_login(int p_id, client_packet_login* packet)
{
	players[p_id].m_lock.lock();
	strcpy_s(players[p_id].m_name, packet->player_id);
	players[p_id].m_x = 3;
	players[p_id].m_y = 3;
	send_login_ok(p_id);
	players[p_id].m_lock.unlock();
}

void process_packet_move(int p_id, client_packet_move* packet)
{
	player_move(p_id, packet->dir);
}

//void process_packet_attack(int p_id, cs_packet_attack* packet)
{
	//
}

void process_packet_logout(int p_id, client_packet_logout* packet)
{
	//
}


int get_new_player_id()
{
	for (int i = 0; i < NPC_ID_START; ++i)
	{
		if (listenSocket == i) { continue; }
		players[i].m_lock.lock();
		if (NOT_INGAME == players[i].id)
		{
			players[i].id = i; 
			players[i].m_viewlist.clear();
			players[i].m_lock.unlock();
			return i;
		}
		players[i].m_lock.unlock();
	}
	return -1;
}

void do_accept(SOCKET s_socket, SOCKETINFO* a_over)
{
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&a_over->m_over, 0, sizeof(a_over->m_over));
	DWORD num_byte;
	int addr_size = sizeof(SOCKADDR_IN) + 16;
	a_over->m_clientsocket = c_socket;
	BOOL ret = AcceptEx(s_socket, c_socket, a_over->m_buf, 0, addr_size, addr_size, &num_byte, &a_over->m_over);
	if (FALSE == ret)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			cout << "Accept error" << endl;
			exit(-1);
		}
	}
}

void disconnect(int p_id)
{
	
	players[p_id].m_lock.lock();
	unordered_set<int> old_vl = players[p_id].m_viewlist;
	players[p_id].id = NOT_INGAME;
	closesocket(players[p_id].m_socket);
	players[p_id].m_lock.unlock();
	for (auto& cl : old_vl)
	{
		if (true == is_npc(cl)) { continue; }
		if(cl == p_id) {continue;}
		players[cl].m_lock.lock();
		if (NOT_INGAME == players[cl].id)
		{
			players[cl].m_lock.unlock();
			continue; 
		}
		send_remove_object(players[cl].id, p_id);
		players[cl].m_lock.unlock();
	}
}

void earn_exp(int p_id, int exp)
{
	PLAYERINFO &player = players[p_id];
	switch(player.level)
	{
	case 1:
		if((player.exp + exp) > 100) 
		{
			player.level = 2;
			player.exp = player.exp + exp - 100;
		}
		else
		{
			player.exp += exp;
		}
		break;
	case 2:
		if((player.exp + exp) > 200) 
		{
			player.level = 3;
			player.exp = player.exp + exp - 200;
		}
		else
		{
			player.exp += exp;
		}
		break;
	case 3:
		if((player.exp + exp) > 400) 
		{
			player.level = 4;
			player.exp = player.exp + exp - 400;
		}
		else
		{
			player.exp += exp;
		}
		break;
	case 4:
		if((player.exp + exp) > 800) 
		{
			player.level = 5;
			player.exp = player.exp + exp - 800;
		}
		else
		{
			player.exp += exp;
		}
		break;
	case 5:
		if((player.exp + exp) > 1600) 
		{
			player.level = 6;
			player.exp = player.exp + exp - 1600;
		}
		else
		{
			player.exp += exp;
		}
		break;
	}
}

void go_start_location(int p_id)
{
	//
}

void player_dead(int p_id)
{
	players[p_id].exp /= 2;
	players[p_id].hp = 30;

	go_start_location(p_id);
}

void do_random_move_npc(int id)
{
	unordered_set <int> old_vl;
	for (int i = 0; i < NPC_ID_START; ++i)
	{
		if (NOT_INGAME == players[i].id) { continue; }
		if (true == can_see(id, players[i].id)) {old_vl.insert(players[i].id);}
	}
	int x = players[id].m_x;
	int y = players[id].m_y;
	switch (rand() % 4)
	{
	case 0: if(x > 0) x--; break;
	case 1: if(x < (WORLD_WIDTH - 1)) x++; break;
	case 2: if(y > 0) y--; break;
	case 3: if(y < (WORLD_HEIGHT - 1)) y++; break;
	}
	players[id].m_x = x;
	players[id].m_y = y;

	unordered_set <int> new_vl;
	for (int i = 0; i < NPC_ID_START; ++i)
	{
		if (NOT_INGAME == players[i].id) { continue; }
		if (true == can_see(id, players[i].id)) {new_vl.insert(players[i].id);}
	}

	for (auto pl : old_vl)
	{
		if (0 == new_vl.count(pl))
		{
			send_remove_object(pl, id);
		}
	}

	for(auto pl : new_vl)
	{
		if (0 == old_vl.count(pl)) 
		{
			send_add_object(pl, id, OBJ_TYPE::OBJ_NPC);
		}
		send_position_vp(pl, id);
	}
}

void worker()
{
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);

	while (true)
	{
		DWORD num_byte;
		ULONG_PTR i_key;
		WSAOVERLAPPED* over;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_byte, &i_key, &over, INFINITE);
		int key = static_cast<int>(i_key);

		if (FALSE == ret)
		{
			cout << "GQCS error! " << endl;
			disconnect(key);
			continue;
		}
		SOCKETINFO* ex_over = reinterpret_cast<SOCKETINFO*> (over);
		cs_packet_login* pack = reinterpret_cast<cs_packet_login*>(ex_over->m_buf);
		switch (ex_over->m_packet_type[0])
		{
		case TO_SERVER:
			{
				unsigned char* ps = ex_over->m_buf;
				int remain_data = num_byte + players[key].m_prev_recv;

				while (remain_data > 0)
				{
					int packet_size = ps[0];
					if (packet_size > remain_data) { break; }
					
					switch(pack->type)
					{
					case CS_LOGIN:
						{
							process_packet_login(key, reinterpret_cast<cs_packet_login*>(ps));
							break;
						}
					case CS_MOVE:
						{
							process_packet_move(key, reinterpret_cast<cs_packet_move*>(ps));
							break;
						}
					case CS_ATTACK:
						{
							process_packet_attack(key, reinterpret_cast<cs_packet_attack*>(ps));
							break;
						}	
					case CS_CHAT:
						{
							process_packet_chat(key, reinterpret_cast<cs_packet_chat*>(ps));
							break;
						}
					case CS_LOGOUT:
						{
							process_packet_logout(key, reinterpret_cast<cs_packet_logout*>(ps));
							break;
						}
					case CS_TELEPORT:
						{
							process_packet_teleport(key, reinterpret_cast<cs_packet_teleport*>(ps));
							break;
						}
					}
					remain_data -= packet_size;
					ps += packet_size;
				}
				if (remain_data > 0) { memcpy(ex_over->m_buf, ps, remain_data); }
				players[key].m_prev_recv = remain_data;
				do_recv(key);
				break;
			}
			case TO_CLIENT:
			{
				if (num_byte != ex_over->m_wsabuf[0].len) {disconnect(key);}
				switch(pack->type)
				{
				case SC_ADD_OBJECT:
				{
					sc_packet_add_object * add_obj_packet = reinterpret_cast<sc_packet_add_object *>(pack);
					players[key].m_viewlist.insert(add_obj_packet->id);
					break;
				}
					
				case SC_REMOVE_OBJECT:
				{
					sc_packet_remove_object * remove_obj_packet = reinterpret_cast<sc_packet_remove_object *>(pack);
					players[key].m_viewlist.erase(remove_obj_packet->id);
					break;
				}
					
				}
				delete ex_over;
				break;
			}
			case LOGIN_ASK:
			{
				SOCKET c_socket = ex_over->m_clientsocket;
				int p_id = get_new_player_id();
				if (-1 == p_id)
				{
					closesocket(c_socket);
					do_accept(listenSocket, ex_over);
					continue;
				}

				PLAYERINFO& n_s = players[p_id];

				n_s.m_lock.lock();
				n_s.id = p_id;
				n_s.m_prev_recv = 0;
				n_s.m_recv_over.m_packet_type[0] = TO_SERVER;
				n_s.m_recv_over.m_packet_type[1] = CS_LOGIN;
				n_s.m_socket = c_socket;
				n_s.m_x = 3;
				n_s.m_y = 3;
				n_s.m_namebuf[0] = 0;
				n_s.m_lock.unlock();

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, p_id, 0);

				do_recv(p_id);
				do_accept(listenSocket, ex_over);
	
				cout << "New Client [" << p_id << "] !" << endl;

				//PostQueuedCompletionStatus(h_iocp, 1, key, &ex_over->m_over);
				break;
			}
			default:
				cout << "Unknown Packet Type" << endl;
				exit(-1);
		}
	}
}

int main(void)
{
	InitWinSock();

	WSADATA WSAData;
	int ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if(!ret){ }
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(!listenSocket) {cout << "ListenSocket error" << endl;}
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	ret = bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	if(!ret) {}
	ret = listen(listenSocket, SOMAXCONN);
	if(!ret) {}

	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), h_iocp, (DWORD)100000, 0);

	SOCKETINFO socketinfo;
	socketinfo.m_packet_type[0] = LOGIN_ASK;

	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&socketinfo.m_over, 0, sizeof(socketinfo.m_over));
	DWORD num_byte;
	int addr_size = sizeof(SOCKADDR_IN) + 16;
	socketinfo.m_clientsocket = c_socket;
	BOOL result = AcceptEx(listenSocket, c_socket, socketinfo.m_buf, 0, addr_size, addr_size, &num_byte, &socketinfo.m_over);
	if (FALSE == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			cout << "Accept error" << endl;
			exit(-1);
		}
	}

	/*for(auto& player: players)
	{
		player.m_x = rand() % WORLD_WIDTH;
		player.m_y = rand() % WORLD_HEIGHT;
	}*/

	cout << "done!" << endl;

	vector <thread> worker_threads;
	for (int i = 0; i < 4; ++i)
	{
		worker_threads.emplace_back(worker);
	}

	for (auto& th : worker_threads) {th.join();}

	closesocket(listenSocket);
	WSACleanup();
	return 0;
}