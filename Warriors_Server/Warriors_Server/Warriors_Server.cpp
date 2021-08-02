#include <WS2tcpip.h>
#include <MSWSock.h>
#include <map>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <unordered_set>
#include "Protocol.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

constexpr char	NUMOFTHREAD = 8;
int				nThreadCnt = 0;
HANDLE			hIOCP;

// IOCP ���� ����ü
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
	//OP_TYPE			m_op;
	SOCKET			client_socket;
};

SOCKETINFO*		a_over;

// ��Ŷ ó�� �Լ� ������
struct FuncProcess
{
	void(*funcProcessPacket)(stringstream & RecvStream, stSOCKETINFO * pSocket);
	FuncProcess()
	{
		funcProcessPacket = nullptr;
	}
};

class IocpBase
{
public:
	IocpBase();
	virtual ~IocpBase();

	// ���� ��� �� ���� ���� ����
	bool Initialize();
	// ���� ����
	virtual void StartServer();
	// �۾� ������ ����
	virtual bool CreateWorkerThread();	
	// �۾� ������
	virtual void WorkerThread();
	// Ŭ���̾�Ʈ���� �۽�
	virtual void Send(stSOCKETINFO * pSocket);
	// Ŭ���̾�Ʈ ���� ���
	virtual void Recv(stSOCKETINFO * pSocket);		

protected:
	stSOCKETINFO * SocketInfo;		// ���� ����
	SOCKET			ListenSocket;	// ���� ���� ����
	HANDLE			hIOCP;			// IOCP ��ü �ڵ�
	bool			bAccept;		// ��û ���� �÷���
	bool			bWorkerThread;	// �۾� ������ ���� �÷���
	HANDLE *		hWorkerHandle;	// �۾� ������ �ڵ�		
};

struct SESSION
{
	int						id					= 0;
	SOCKETINFO					m_recv_over;
	unsigned char			m_prev_recv			= 0;
	SOCKET					m_s					= 0;

	//S_STATE					m_state				= S_STATE::STATE_FREE;			//0 : Free, 1 : Connected, 2 : 
	mutex					m_lock;
	//char					name[MAX_NAME]		= {0,};
	short					m_x = 0, m_y = 0;
	int						last_move_time		= 0;
	unordered_set <int>		m_viewlist;
};

//SESSION players[MAX_USER]; // Data Race!
SOCKET listenSocket;

class MainIocp : public IocpBase
{
public:
	MainIocp();
	virtual ~MainIocp();
	
	virtual void StartServer() override;
	// �۾� ������ ����
	virtual bool CreateWorkerThread() override;
	// �۾� ������
	virtual void WorkerThread() override;
	// Ŭ���̾�Ʈ���� �۽�
	static void Send(stSOCKETINFO * pSocket);	

	// ���� ������
	void CreateMonsterManagementThread();
	void MonsterManagementThread();

private:
	//static cCharactersInfo	CharactersInfo;	// ������ Ŭ���̾�Ʈ�� ������ ����	
	static map<int, SOCKET> SessionSocket;	// ���Ǻ� ���� ����
	static float			HitPoint;		// Ÿ�� ������
	static CRITICAL_SECTION	csPlayers;		// CharactersInfo �Ӱ迵��

	HANDLE*					MonsterHandle;	// ���� ������ �ڵ鷯

	// ȸ������
	static void SignUp(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// DB�� �α���
	static void Login(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// ĳ���� �ʱ� ���
	static void EnrollCharacter(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// ĳ���� ��ġ ����ȭ
	static void SyncCharacters(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// ĳ���� �α׾ƿ� ó��
	static void LogoutCharacter(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// ĳ���� �ǰ� ó��
	static void HitCharacter(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// ä�� ���� �� Ŭ���̾�Ʈ�鿡�� �۽�
	static void BroadcastChat(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// ���� �ǰ� ó��
	static void HitMonster(stringstream & RecvStream, stSOCKETINFO * pSocket);

	// ��ε�ĳ��Ʈ �Լ�
	static void Broadcast(stringstream & SendStream);	
	// �ٸ� Ŭ���̾�Ʈ�鿡�� �� �÷��̾� ���� ���� ����
	//static void BroadcastNewPlayer(cCharacter & player);
	// ĳ���� ������ ���ۿ� ���
	static void WriteCharactersInfoToSocket(stSOCKETINFO * pSocket);		
	
	// ���� ���� �ʱ�ȭ
	void InitializeMonsterSet();
};

IocpBase::IocpBase()
{
	// ��� ���� �ʱ�ȭ
	bWorkerThread = true;
	bAccept = true;
}


IocpBase::~IocpBase()
{
	// winsock �� ����� ������
	WSACleanup();
	// �� ����� ��ü�� ����
	if (SocketInfo)
	{
		delete[] SocketInfo;
		SocketInfo = NULL;
	}

	if (hWorkerHandle)
	{
		delete[] hWorkerHandle;
		hWorkerHandle = NULL;
	}
}

bool IocpBase::Initialize()
{
	WSADATA wsaData;
	int nResult;
	// winsock 2.2 �������� �ʱ�ȭ
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nResult != 0)
	{
		printf_s("Winsock Init Error\n");
		return false;
	}

	// ���� ����
	ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (ListenSocket == INVALID_SOCKET)
	{
		printf_s("Socket Create Error\n");
		return false;
	}

	// ���� ���� ����
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// ���� ����
	// boost bind �� �������� ���� ::bind ���
	nResult = ::bind(ListenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	
	if (nResult == SOCKET_ERROR)
	{
		printf_s("Bind Error\n");
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}

	// ���� ��⿭ ����
	nResult = listen(ListenSocket, SOMAXCONN);
	if (nResult == SOCKET_ERROR)
	{
		printf_s("Listen Error\n");
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}

	return true;
}

void IocpBase::StartServer()
{
	int nResult;
	// Ŭ���̾�Ʈ ����
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	DWORD recvBytes;
	DWORD flags;

	// Completion Port ��ü ����
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), hIOCP, (DWORD)100000, 0);

	// Worker Thread ����
	if (!CreateWorkerThread()) return;	

	printf_s("Server Start...\n");

	// Ŭ���̾�Ʈ ������ ����
	//while (bAccept)
	{
		SOCKETINFO* a_over;
		//a_over.m_op = OP_TYPE::OP_ACCEPT;
		memset(&a_over->overlapped, 0, sizeof(a_over->overlapped));
		DWORD num_byte;
		int addr_size = sizeof(SOCKADDR_IN) + 16;
		a_over->client_socket = clientSocket;
		BOOL ret = AcceptEx(ListenSocket, clientSocket, a_over->wsabuf, 0, addr_size, addr_size, &num_byte, &a_over->m_over);

		if (FALSE == ret)
		{
			int err = WSAGetLastError();
			if (WSA_IO_PENDING != err)
			{
				printf_s("Accept \d Error\n", err);
				return;
			}
		}
	}

}

bool IocpBase::CreateWorkerThread()
{
	return false;
}

void IocpBase::Send(stSOCKETINFO * pSocket)
{
	int nResult;
	DWORD	sendBytes;
	DWORD	dwFlags = 0;

	nResult = WSASend(
		pSocket->socket,
		&(pSocket->dataBuf),
		1,
		&sendBytes,
		dwFlags,
		NULL,
		NULL
	);

	if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		printf_s("WSASend Error : ", WSAGetLastError());
	}

}

void IocpBase::Recv(stSOCKETINFO * pSocket)
{
	int nResult;
	// DWORD	sendBytes;
	DWORD	dwFlags = 0;

	// stSOCKETINFO ������ �ʱ�ȭ
	ZeroMemory(&(pSocket->overlapped), sizeof(OVERLAPPED));
	ZeroMemory(pSocket->messageBuffer, MAX_BUFFER);
	pSocket->dataBuf.len = MAX_BUFFER;
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->recvBytes = 0;
	pSocket->sendBytes = 0;

	dwFlags = 0;

	// Ŭ���̾�Ʈ�κ��� �ٽ� ������ �ޱ� ���� WSARecv �� ȣ������
	nResult = WSARecv(
		pSocket->socket,
		&(pSocket->dataBuf),
		1,
		(LPDWORD)&pSocket,
		&dwFlags,
		(LPWSAOVERLAPPED)&(pSocket->overlapped),
		NULL
	);

	if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		printf_s("WSARecv Error : ", WSAGetLastError());
	}
}

void IocpBase::WorkerThread()
{
	/*SocketInfo = new stSOCKETINFO();
		SocketInfo->socket = clientSocket;
		SocketInfo->recvBytes = 0;
		SocketInfo->sendBytes = 0;
		SocketInfo->dataBuf.len = MAX_BUFFER;
		SocketInfo->dataBuf.buf = SocketInfo->messageBuffer;
		flags = 0;*/

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);

	/*	hIOCP = CreateIoCompletionPort(
			(HANDLE)clientSocket, hIOCP, (DWORD)SocketInfo, 0
		);

		// ��ø ������ �����ϰ� �Ϸ�� ����� �Լ��� �Ѱ���
		nResult = WSARecv(
			SocketInfo->socket,
			&SocketInfo->dataBuf,
			1,
			&recvBytes,
			&flags,
			&(SocketInfo->overlapped),
			NULL
		);

		if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			printf_s("[ERROR] IO Pending ���� : %d", WSAGetLastError());
			return;
		}
		*/
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
			printf_s("GQCS %d error ", err);
			//disconnect(key);
			continue;
		}
		SOCKETINFO* socketinfo = reinterpret_cast<SOCKETINFO*>(over);
		/*switch (socketinfo->m_op)
		{
			case OP_TYPE::OP_RECV:
			{
				unsigned char* ps = SOCKETINFO->m_netbuf;
				int remain_data = num_byte + players[key].m_prev_recv;

				while (remain_data > 0)
				{
					int packet_size = ps[0];
					if (packet_size > remain_data) { break; }
					process_packet(key, ps);
					remain_data -= packet_size;
					ps += packet_size;
				}
				if (remain_data > 0) { memcpy(SOCKETINFO->m_netbuf, ps, remain_data); }
				players[key].m_prev_recv = remain_data;
				do_recv(key);
				break;
			}
			case OP_TYPE::OP_SEND:
			{
				if (num_byte != SOCKETINFO->m_wsabuf[0].len)
				{
					disconnect(key);
					delete SOCKETINFO;
				}
				break;
			}
			case OP_TYPE::OP_ACCEPT:
			{
				SOCKET c_socket = SOCKETINFO->c_socket;
				int p_id = get_new_player_id();
				if (-1 == p_id)
				{
					closesocket(c_socket);
					do_accept(listenSocket, SOCKETINFO);
					continue;
				}

				SESSION& n_s = players[p_id];

				n_s.m_lock.lock();
				n_s.m_state = S_STATE::STATE_CONNECTED;
				n_s.id = p_id;
				n_s.m_prev_recv = 0;
				n_s.m_recv_over.m_op = OP_TYPE::OP_RECV;
				n_s.m_s = c_socket;
				n_s.m_x = 3;
				n_s.m_y = 3;
				n_s.name[0] = 0;
				n_s.m_lock.unlock();

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, p_id, 0);

				do_recv(p_id);
				do_accept(listenSocket, SOCKETINFO);
	
				cout << "New Client [" << p_id << "] !" << endl;
				break;
			}

			default:
				cout << "Unknown Packet Type" << endl;
				exit(-1);
		}*/

	}
}

// static ���� �ʱ�ȭ
float				MainIocp::HitPoint = 0.1f;
map<int, SOCKET>	MainIocp::SessionSocket;
//cCharactersInfo		MainIocp::CharactersInfo;
//DBConnector			MainIocp::Conn;
CRITICAL_SECTION	MainIocp::csPlayers;
//MonsterSet			MainIocp::MonstersInfo;

unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	MainIocp* pOverlappedEvent = (MainIocp*)p;
	pOverlappedEvent->WorkerThread();
	return 0;
}

unsigned int WINAPI CallMonsterThread(LPVOID p)
{
	MainIocp* pOverlappedEvent = (MainIocp*)p;
	pOverlappedEvent->MonsterManagementThread();
	return 0;
}

MainIocp::MainIocp()
{
	InitializeCriticalSection(&csPlayers);

	// DB ����
	/*if (Conn.Connect(DB_ADDRESS, DB_ID, DB_PW, DB_SCHEMA, DB_PORT))
	{
		printf_s("[INFO] DB ���� ����\n");
	}
	else {
		printf_s("[ERROR] DB ���� ����\n");
	}*/

	// ��Ŷ �Լ� �����Ϳ� �Լ� ����
	/*fnProcess[EPacketType::SIGNUP].funcProcessPacket = SignUp;
	fnProcess[EPacketType::LOGIN].funcProcessPacket = Login;
	fnProcess[EPacketType::ENROLL_PLAYER].funcProcessPacket = EnrollCharacter;
	fnProcess[EPacketType::SEND_PLAYER].funcProcessPacket = SyncCharacters;
	fnProcess[EPacketType::HIT_PLAYER].funcProcessPacket = HitCharacter;
	fnProcess[EPacketType::CHAT].funcProcessPacket = BroadcastChat;
	fnProcess[EPacketType::LOGOUT_PLAYER].funcProcessPacket = LogoutCharacter;
	fnProcess[EPacketType::HIT_MONSTER].funcProcessPacket = HitMonster;*/
}


MainIocp::~MainIocp()
{
	// winsock �� ����� ������
	WSACleanup();
	// �� ����� ��ü�� ����
	if (SocketInfo)
	{
		delete[] SocketInfo;
		SocketInfo = NULL;
	}

	if (hWorkerHandle)
	{
		delete[] hWorkerHandle;
		hWorkerHandle = NULL;
	}

	// DB ���� ����
	//Conn.Close();
}

void MainIocp::StartServer()
{
	//CreateMonsterManagementThread();
	IocpBase::StartServer();
}

bool MainIocp::CreateWorkerThread()
{
	unsigned int threadId;
	// �ý��� ���� ������
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	printf_s("CPU ���� : %d\n", sysInfo.dwNumberOfProcessors);
	// ������ �۾� �������� ������ (CPU * 2) + 1
	nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

	// thread handler ����
	//hWorkerHandle = new HANDLE[nThreadCnt];
	// thread ����
	/*for (int i = 0; i < nThreadCnt; i++)
	{
		hWorkerHandle[i] = (HANDLE *)_beginthreadex(
			NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId
		);
		if (hWorkerHandle[i] == NULL)
		{
			printf_s("[ERROR] Worker Thread ���� ����\n");
			return false;
		}
		ResumeThread(hWorkerHandle[i]);
	}*/
	printf_s("Worker Thread Start...\n");
	return true;
}

void MainIocp::Send(stSOCKETINFO * pSocket)
{
	int nResult;
	DWORD	sendBytes;
	DWORD	dwFlags = 0;

	nResult = WSASend(
		pSocket->socket,
		&(pSocket->dataBuf),
		1,
		&sendBytes,
		dwFlags,
		NULL,
		NULL
	);

	if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		printf_s("[ERROR] WSASend ���� : ", WSAGetLastError());
	}


}

void MainIocp::CreateMonsterManagementThread()
{
	unsigned int threadId;

	MonsterHandle = (HANDLE *)_beginthreadex(
		NULL, 0, &CallMonsterThread, this, CREATE_SUSPENDED, &threadId
	);
	if (MonsterHandle == NULL)
	{
		printf_s("[ERROR] Monster Thread ���� ����\n");
		return;
	}
	ResumeThread(MonsterHandle);

	printf_s("[INFO] Monster Thread ����...\n");
}

void MainIocp::MonsterManagementThread()
{
	// ���� �ʱ�ȭ
	InitializeMonsterSet();
	int count = 0;	
	// ���� ����
	while (true)
	{
		/*for (auto & kvp : MonstersInfo.monsters)
		{
			auto & monster = kvp.second;
			for (auto & player : CharactersInfo.players)
			{
				// �÷��̾ ���Ͱ� �׾����� �� ����
				if (!player.second.IsAlive || !monster.IsAlive())
					continue;

				if (monster.IsPlayerInHitRange(player.second) && !monster.bIsAttacking)
				{
					monster.HitPlayer(player.second);
					continue;
				}

				if (monster.IsPlayerInTraceRange(player.second) && !monster.bIsAttacking)
				{
					monster.MoveTo(player.second);
					continue;
				}
			}
		}*/

		count++;
		// 0.5�ʸ��� Ŭ���̾�Ʈ���� ���� ���� ����
		if (count > 15)
		{			
			stringstream SendStream;
			//SendStream << EPacketType::SYNC_MONSTER << endl;
			//SendStream << MonstersInfo << endl;

			count = 0;
			Broadcast(SendStream);
		}
		
		Sleep(33);
	}
}

void MainIocp::WriteCharactersInfoToSocket(stSOCKETINFO* pSocket)
{
}

void MainIocp::InitializeMonsterSet()
{
	// ���� �ʱ�ȭ	
	/*Monster mFields;

	mFields.X = -5746;
	mFields.Y = 3736;
	mFields.Z = 7362;
	mFields.Health = 100.0f;
	mFields.Id = 1;
	mFields.MovePoint = 10.f;
	MonstersInfo.monsters[mFields.Id] = mFields;

	mFields.X = -5136;
	mFields.Y = 1026;
	mFields.Z = 7712;
	mFields.Id = 2;
	MonstersInfo.monsters[mFields.Id] = mFields;

	mFields.X = -3266;
	mFields.Y = 286;
	mFields.Z = 8232;
	mFields.Id = 3;
	MonstersInfo.monsters[mFields.Id] = mFields;

	mFields.X = -156;
	mFields.Y = 326;
	mFields.Z = 8352;
	mFields.Id = 4;
	MonstersInfo.monsters[mFields.Id] = mFields;*/
}

void MainIocp::WorkerThread()
{
	// �Լ� ȣ�� ���� ����
	BOOL	bResult;
	int		nResult;
	// Overlapped I/O �۾����� ���۵� ������ ũ��
	DWORD	recvBytes;
	DWORD	sendBytes;
	// Completion Key�� ���� ������ ����
	stSOCKETINFO *	pCompletionKey;
	// I/O �۾��� ���� ��û�� Overlapped ����ü�� ���� ������	
	stSOCKETINFO *	pSocketInfo;
	DWORD	dwFlags = 0;


	while (bWorkerThread)
	{
		/**
		 * �� �Լ��� ���� ��������� WaitingThread Queue �� �����·� ���� ��
		 * �Ϸ�� Overlapped I/O �۾��� �߻��ϸ� IOCP Queue ���� �Ϸ�� �۾��� ������
		 * ��ó���� ��
		 */
		bResult = GetQueuedCompletionStatus(hIOCP,
			&recvBytes,				// ������ ���۵� ����Ʈ
			(PULONG_PTR)&pCompletionKey,	// completion key
			(LPOVERLAPPED *)&pSocketInfo,			// overlapped I/O ��ü
			INFINITE				// ����� �ð�
		);

		if (!bResult && recvBytes == 0)
		{
			printf_s("[INFO] socket(%d) ���� ����\n", pSocketInfo->socket);
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
			continue;
		}

		pSocketInfo->dataBuf.len = recvBytes;

		if (recvBytes == 0)
		{
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
			continue;
		}

		try
		{
			// ��Ŷ ����
			int PacketType;
			// Ŭ���̾�Ʈ ���� ������ȭ
			stringstream RecvStream;

			RecvStream << pSocketInfo->dataBuf.buf;
			RecvStream >> PacketType;

			// ��Ŷ ó��
			//if (fnProcess[PacketType].funcProcessPacket != nullptr)
			{
				//fnProcess[PacketType].funcProcessPacket(RecvStream, pSocketInfo);
			}
			//else
			{
				printf_s("[ERROR] ���� ���� ���� ��Ŷ : %d\n", PacketType);
			}
		}
		catch (const std::exception& e)
		{
			printf_s("[ERROR] �� �� ���� ���� �߻� : %s\n", e.what());
		}

		// Ŭ���̾�Ʈ ���
		Recv(pSocketInfo);
	}
}

void MainIocp::SignUp(stringstream & RecvStream, stSOCKETINFO * pSocket)
{
	string Id;
	string Pw;

	RecvStream >> Id;
	RecvStream >> Pw;

	printf_s("[INFO] ȸ������ �õ� {%s}/{%s}\n", Id, Pw);

	stringstream SendStream;
	//SendStream << EPacketType::SIGNUP << endl;
	//SendStream << Conn.SignUpAccount(Id, Pw) << endl;

	CopyMemory(pSocket->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->dataBuf.len = SendStream.str().length();

	Send(pSocket);
}

void MainIocp::Login(stringstream & RecvStream, stSOCKETINFO * pSocket)
{
	string Id;
	string Pw;

	RecvStream >> Id;
	RecvStream >> Pw;

	printf_s("[INFO] �α��� �õ� {%s}/{%s}\n", Id, Pw);

	stringstream SendStream;
	//SendStream << EPacketType::LOGIN << endl;
	//SendStream << Conn.SearchAccount(Id, Pw) << endl;

	CopyMemory(pSocket->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->dataBuf.len = SendStream.str().length();

	Send(pSocket);
}

void MainIocp::EnrollCharacter(stringstream & RecvStream, stSOCKETINFO * pSocket)
{
	/*cCharacter info;
	RecvStream >> info;

	printf_s("[INFO][%d]ĳ���� ��� - X : [%f], Y : [%f], Z : [%f], Yaw : [%f], Alive : [%d], Health : [%f]\n",
		info.SessionId, info.X, info.Y, info.Z, info.Yaw, info.IsAlive, info.HealthValue);

	EnterCriticalSection(&csPlayers);

	cCharacter* pinfo = &CharactersInfo.players[info.SessionId];

	// ĳ������ ��ġ�� ����						
	pinfo->SessionId = info.SessionId;
	pinfo->X = info.X;
	pinfo->Y = info.Y;
	pinfo->Z = info.Z;

	// ĳ������ ȸ������ ����
	pinfo->Yaw = info.Yaw;
	pinfo->Pitch = info.Pitch;
	pinfo->Roll = info.Roll;

	// ĳ������ �ӵ��� ����
	pinfo->VX = info.VX;
	pinfo->VY = info.VY;
	pinfo->VZ = info.VZ;

	// ĳ���� �Ӽ�
	pinfo->IsAlive = info.IsAlive;
	pinfo->HealthValue = info.HealthValue;
	pinfo->IsAttacking = info.IsAttacking;

	LeaveCriticalSection(&csPlayers);

	SessionSocket[info.SessionId] = pSocket->socket;

	printf_s("[INFO] Ŭ���̾�Ʈ �� : %d\n", SessionSocket.size());

	//Send(pSocket);
	BroadcastNewPlayer(info);*/
}

void MainIocp::SyncCharacters(stringstream& RecvStream, stSOCKETINFO* pSocket)
{
	/*cCharacter info;
	RecvStream >> info;

	// 	 	printf_s("[INFO][%d]���� ���� - %d\n",
	// 	 		info.SessionId, info.IsAttacking);	
	EnterCriticalSection(&csPlayers);

	cCharacter * pinfo = &CharactersInfo.players[info.SessionId];

	// ĳ������ ��ġ�� ����						
	pinfo->SessionId = info.SessionId;
	pinfo->X = info.X;
	pinfo->Y = info.Y;
	pinfo->Z = info.Z;

	// ĳ������ ȸ������ ����
	pinfo->Yaw = info.Yaw;
	pinfo->Pitch = info.Pitch;
	pinfo->Roll = info.Roll;

	// ĳ������ �ӵ��� ����
	pinfo->VX = info.VX;
	pinfo->VY = info.VY;
	pinfo->VZ = info.VZ;

	pinfo->IsAttacking = info.IsAttacking;

	LeaveCriticalSection(&csPlayers);

	WriteCharactersInfoToSocket(pSocket);
	Send(pSocket);*/
}

void MainIocp::LogoutCharacter(stringstream& RecvStream, stSOCKETINFO* pSocket)
{
	/*int SessionId;
	RecvStream >> SessionId;
	printf_s("[INFO] (%d)�α׾ƿ� ��û ����\n", SessionId);
	EnterCriticalSection(&csPlayers);
	CharactersInfo.players[SessionId].IsAlive = false;
	LeaveCriticalSection(&csPlayers);
	SessionSocket.erase(SessionId);
	printf_s("[INFO] Ŭ���̾�Ʈ �� : %d\n", SessionSocket.size());
	WriteCharactersInfoToSocket(pSocket);*/
}

void MainIocp::HitCharacter(stringstream & RecvStream, stSOCKETINFO * pSocket)
{
	// �ǰ� ó���� ���� ���̵�
	/*int DamagedSessionId;
	RecvStream >> DamagedSessionId;
	printf_s("[INFO] %d ������ ���� \n", DamagedSessionId);
	EnterCriticalSection(&csPlayers);
	CharactersInfo.players[DamagedSessionId].HealthValue -= HitPoint;
	if (CharactersInfo.players[DamagedSessionId].HealthValue < 0)
	{
		// ĳ���� ���ó��
		CharactersInfo.players[DamagedSessionId].IsAlive = false;
	}
	LeaveCriticalSection(&csPlayers);
	WriteCharactersInfoToSocket(pSocket);
	Send(pSocket);*/
}

void MainIocp::BroadcastChat(stringstream& RecvStream, stSOCKETINFO* pSocket)
{
	/*stSOCKETINFO* client = new stSOCKETINFO;

	int SessionId;
	string Temp;
	string Chat;

	RecvStream >> SessionId;
	getline(RecvStream, Temp);
	Chat += to_string(SessionId) + "_:_";
	while (RecvStream >> Temp)
	{
		Chat += Temp + "_";
	}
	Chat += '\0';

	printf_s("[CHAT] %s\n", Chat);

	stringstream SendStream;
	SendStream << EPacketType::CHAT << endl;
	SendStream << Chat;

	Broadcast(SendStream);*/
}

void MainIocp::HitMonster(stringstream& RecvStream, stSOCKETINFO* pSocket)
{
	// ���� �ǰ� ó��
	/*int MonsterId;
	RecvStream >> MonsterId;
	MonstersInfo.monsters[MonsterId].Damaged(30.f);

	if (!MonstersInfo.monsters[MonsterId].IsAlive())
	{
		stringstream SendStream;
		SendStream << EPacketType::DESTROY_MONSTER << endl;
		SendStream << MonstersInfo.monsters[MonsterId] << endl;

		Broadcast(SendStream);

		MonstersInfo.monsters.erase(MonsterId);
	}

	// �ٸ� �÷��̾�� ��ε�ĳ��Ʈ
	stringstream SendStream;
	SendStream << EPacketType::HIT_MONSTER << endl;
	SendStream << MonstersInfo << endl;

	Broadcast(SendStream);*/
}

void MainIocp::Broadcast(stringstream& SendStream)
{
	stSOCKETINFO* client = new stSOCKETINFO;
	for (const auto& kvp : SessionSocket)
	{
		client->socket = kvp.second;
		CopyMemory(client->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
		client->dataBuf.buf = client->messageBuffer;
		client->dataBuf.len = SendStream.str().length();

		Send(client);
	}
}



/*void MainIocp::BroadcastNewPlayer(cCharacter & player)
{
	stringstream SendStream;
	SendStream << EPacketType::ENTER_NEW_PLAYER << endl;
	SendStream << player << endl;

	Broadcast(SendStream);
}*/

void MainIocp::WriteCharactersInfoToSocket(stSOCKETINFO * pSocket)
{
	stringstream SendStream;

	// ����ȭ	
	//SendStream << EPacketType::RECV_PLAYER << endl;
	//SendStream << CharactersInfo << endl;

	// !!! �߿� !!! data.buf ���� ���� �����͸� ���� �����Ⱚ�� ���޵� �� ����
	CopyMemory(pSocket->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->dataBuf.len = SendStream.str().length();
}

bool Initialize()
{
	WSADATA wsaData;
	int nResult;
	// winsock 2.2 �������� �ʱ�ȭ
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nResult != 0)
	{
		printf_s("Winsock Init Error\n");
		return false;
	}

	// ���� ����
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (listenSocket == INVALID_SOCKET)
	{
		printf_s("Socket Create Error\n");
		return false;
	}

	// ���� ���� ����
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// ���� ����
	// boost bind �� �������� ���� ::bind ���
	nResult = ::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	
	if (nResult == SOCKET_ERROR)
	{
		printf_s("Bind Error\n");
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	// ���� ��⿭ ����
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
	unsigned int threadId;
	// �ý��� ���� ������
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	printf_s("CPU ���� : {%d}\n", sysInfo.dwNumberOfProcessors);
	// ������ �۾� �������� ������ (CPU * 2) + 1
	nThreadCnt = sysInfo.dwNumberOfProcessors * 2;
	if(!nThreadCnt) {return false;}
	printf_s("Worker Thread Start...\n");
	return true;
}

void StartServer()
{
	int nResult;
	// Ŭ���̾�Ʈ ����
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	DWORD recvBytes;
	DWORD flags;

	// Completion Port ��ü ����
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), hIOCP, (DWORD)100000, 0);

	// Worker Thread ����
	if (!CalculateWorkerThread()) return;	

	printf_s("Server Start...\n");

	// Ŭ���̾�Ʈ ������ ����
	//while (bAccept)
	{
		//a_over.m_op = OP_TYPE::OP_ACCEPT;
		memset(&a_over->overlapped, 0, sizeof(a_over->overlapped));
		DWORD num_byte;
		int addr_size = sizeof(SOCKADDR_IN) + 16;
		a_over->client_socket = clientSocket;
		BOOL ret = AcceptEx(listenSocket, clientSocket, a_over->messagebuf, 0, addr_size, addr_size, &num_byte, &a_over->overlapped);

		if (FALSE == ret)
		{
			int err = WSAGetLastError();
			if (WSA_IO_PENDING != err)
			{
				printf_s("Accept {\d} Error\n", err);
				return;
			}
		}
	}
}

void WorkerThread()
{
	/*SocketInfo = new stSOCKETINFO();
		SocketInfo->socket = clientSocket;
		SocketInfo->recvBytes = 0;
		SocketInfo->sendBytes = 0;
		SocketInfo->dataBuf.len = MAX_BUFFER;
		SocketInfo->dataBuf.buf = SocketInfo->messageBuffer;
		flags = 0;*/

	/*SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);*/

	/*	hIOCP = CreateIoCompletionPort(
			(HANDLE)clientSocket, hIOCP, (DWORD)SocketInfo, 0
		);

		// ��ø ������ �����ϰ� �Ϸ�� ����� �Լ��� �Ѱ���
		nResult = WSARecv(
			SocketInfo->socket,
			&SocketInfo->dataBuf,
			1,
			&recvBytes,
			&flags,
			&(SocketInfo->overlapped),
			NULL
		);

		if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			printf_s("[ERROR] IO Pending ���� : %d", WSAGetLastError());
			return;
		}
		*/
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
		SOCKETINFO* SOCKETINFO = reinterpret_cast<SOCKETINFO*> (over);
		/*switch (SOCKETINFO->m_op)
		{
			case OP_TYPE::OP_RECV:
			{
				unsigned char* ps = SOCKETINFO->m_netbuf;
				int remain_data = num_byte + players[key].m_prev_recv;

				while (remain_data > 0)
				{
					int packet_size = ps[0];
					if (packet_size > remain_data) { break; }
					process_packet(key, ps);
					remain_data -= packet_size;
					ps += packet_size;
				}
				if (remain_data > 0) { memcpy(SOCKETINFO->m_netbuf, ps, remain_data); }
				players[key].m_prev_recv = remain_data;
				do_recv(key);
				break;
			}
			case OP_TYPE::OP_SEND:
			{
				if (num_byte != SOCKETINFO->m_wsabuf[0].len)
				{
					disconnect(key);
					delete SOCKETINFO;
				}
				break;
			}
			case OP_TYPE::OP_ACCEPT:
			{
				SOCKET c_socket = SOCKETINFO->c_socket;
				int p_id = get_new_player_id();
				if (-1 == p_id)
				{
					closesocket(c_socket);
					do_accept(listenSocket, SOCKETINFO);
					continue;
				}

				SESSION& n_s = players[p_id];

				n_s.m_lock.lock();
				n_s.m_state = S_STATE::STATE_CONNECTED;
				n_s.id = p_id;
				n_s.m_prev_recv = 0;
				n_s.m_recv_over.m_op = OP_TYPE::OP_RECV;
				n_s.m_s = c_socket;
				n_s.m_x = 3;
				n_s.m_y = 3;
				n_s.name[0] = 0;
				n_s.m_lock.unlock();

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, p_id, 0);

				do_recv(p_id);
				do_accept(listenSocket, SOCKETINFO);
	
				cout << "New Client [" << p_id << "] !" << endl;
				break;
			}

			default:
				cout << "Unknown Packet Type" << endl;
				exit(-1);
		}*/

	}


	// �Լ� ȣ�� ���� ����
	BOOL	bResult;
	int		nResult;
	// Overlapped I/O �۾����� ���۵� ������ ũ��
	DWORD	recvBytes;
	DWORD	sendBytes;
	// Completion Key�� ���� ������ ����
	stSOCKETINFO *	pCompletionKey;
	// I/O �۾��� ���� ��û�� Overlapped ����ü�� ���� ������	
	stSOCKETINFO *	pSocketInfo;
	DWORD	dwFlags = 0;


	//while (bWorkerThread)
	{
		/**
		 * �� �Լ��� ���� ��������� WaitingThread Queue �� �����·� ���� ��
		 * �Ϸ�� Overlapped I/O �۾��� �߻��ϸ� IOCP Queue ���� �Ϸ�� �۾��� ������
		 * ��ó���� ��
		 */
		bResult = GetQueuedCompletionStatus(hIOCP,
			&recvBytes,				// ������ ���۵� ����Ʈ
			(PULONG_PTR)&pCompletionKey,	// completion key
			(LPOVERLAPPED *)&pSocketInfo,			// overlapped I/O ��ü
			INFINITE				// ����� �ð�
		);

		if (!bResult && recvBytes == 0)
		{
			printf_s("[INFO] socket(%d) ���� ����\n", pSocketInfo->socket);
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
			//continue;
		}

		pSocketInfo->dataBuf.len = recvBytes;

		if (recvBytes == 0)
		{
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
			//continue;
		}

		try
		{
			// ��Ŷ ����
			int PacketType;
			// Ŭ���̾�Ʈ ���� ������ȭ
			stringstream RecvStream;

			RecvStream << pSocketInfo->dataBuf.buf;
			RecvStream >> PacketType;

			// ��Ŷ ó��
			//if (fnProcess[PacketType].funcProcessPacket != nullptr)
			{
				//fnProcess[PacketType].funcProcessPacket(RecvStream, pSocketInfo);
			}
			//else
			{
				printf_s("[ERROR] ���� ���� ���� ��Ŷ : %d\n", PacketType);
			}
		}
		catch (const std::exception& e)
		{
			printf_s("[ERROR] �� �� ���� ���� �߻� : %s\n", e.what());
		}

		// Ŭ���̾�Ʈ ���
		//Recv(pSocketInfo);
	}
}


int main()
{
	//MainIocp iocp_server;
	if (Initialize())
	{
		StartServer();
		vector<thread> worker_threads;
		while(!nThreadCnt){};
		for(int i = 0; i < nThreadCnt; ++i)
		{
			worker_threads.emplace_back(WorkerThread);
		}
		for(auto& worker_thread : worker_threads) {worker_thread.join();}

		closesocket(listenSocket);
		WSACleanup();
	}
    return 0;
}