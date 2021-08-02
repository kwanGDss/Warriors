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
	//OP_TYPE			m_op;
	SOCKET			client_socket;
};

SOCKETINFO*		a_over;

// 패킷 처리 함수 포인터
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

	// 소켓 등록 및 서버 정보 설정
	bool Initialize();
	// 서버 시작
	virtual void StartServer();
	// 작업 스레드 생성
	virtual bool CreateWorkerThread();	
	// 작업 스레드
	virtual void WorkerThread();
	// 클라이언트에게 송신
	virtual void Send(stSOCKETINFO * pSocket);
	// 클라이언트 수신 대기
	virtual void Recv(stSOCKETINFO * pSocket);		

protected:
	stSOCKETINFO * SocketInfo;		// 소켓 정보
	SOCKET			ListenSocket;	// 서버 리슨 소켓
	HANDLE			hIOCP;			// IOCP 객체 핸들
	bool			bAccept;		// 요청 동작 플래그
	bool			bWorkerThread;	// 작업 스레드 동작 플래그
	HANDLE *		hWorkerHandle;	// 작업 스레드 핸들		
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
	// 작업 스레드 생성
	virtual bool CreateWorkerThread() override;
	// 작업 스레드
	virtual void WorkerThread() override;
	// 클라이언트에게 송신
	static void Send(stSOCKETINFO * pSocket);	

	// 몬스터 스레드
	void CreateMonsterManagementThread();
	void MonsterManagementThread();

private:
	//static cCharactersInfo	CharactersInfo;	// 접속한 클라이언트의 정보를 저장	
	static map<int, SOCKET> SessionSocket;	// 세션별 소켓 저장
	static float			HitPoint;		// 타격 데미지
	static CRITICAL_SECTION	csPlayers;		// CharactersInfo 임계영역

	HANDLE*					MonsterHandle;	// 몬스터 스레드 핸들러

	// 회원가입
	static void SignUp(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// DB에 로그인
	static void Login(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// 캐릭터 초기 등록
	static void EnrollCharacter(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// 캐릭터 위치 동기화
	static void SyncCharacters(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// 캐릭터 로그아웃 처리
	static void LogoutCharacter(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// 캐릭터 피격 처리
	static void HitCharacter(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// 채팅 수신 후 클라이언트들에게 송신
	static void BroadcastChat(stringstream & RecvStream, stSOCKETINFO * pSocket);
	// 몬스터 피격 처리
	static void HitMonster(stringstream & RecvStream, stSOCKETINFO * pSocket);

	// 브로드캐스트 함수
	static void Broadcast(stringstream & SendStream);	
	// 다른 클라이언트들에게 새 플레이어 입장 정보 보냄
	//static void BroadcastNewPlayer(cCharacter & player);
	// 캐릭터 정보를 버퍼에 기록
	static void WriteCharactersInfoToSocket(stSOCKETINFO * pSocket);		
	
	// 몬스터 정보 초기화
	void InitializeMonsterSet();
};

IocpBase::IocpBase()
{
	// 멤버 변수 초기화
	bWorkerThread = true;
	bAccept = true;
}


IocpBase::~IocpBase()
{
	// winsock 의 사용을 끝낸다
	WSACleanup();
	// 다 사용한 객체를 삭제
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
	// winsock 2.2 버전으로 초기화
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nResult != 0)
	{
		printf_s("Winsock Init Error\n");
		return false;
	}

	// 소켓 생성
	ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (ListenSocket == INVALID_SOCKET)
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
	nResult = ::bind(ListenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	
	if (nResult == SOCKET_ERROR)
	{
		printf_s("Bind Error\n");
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}

	// 수신 대기열 생성
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
	// 클라이언트 정보
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	DWORD recvBytes;
	DWORD flags;

	// Completion Port 객체 생성
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), hIOCP, (DWORD)100000, 0);

	// Worker Thread 생성
	if (!CreateWorkerThread()) return;	

	printf_s("Server Start...\n");

	// 클라이언트 접속을 받음
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

	// stSOCKETINFO 데이터 초기화
	ZeroMemory(&(pSocket->overlapped), sizeof(OVERLAPPED));
	ZeroMemory(pSocket->messageBuffer, MAX_BUFFER);
	pSocket->dataBuf.len = MAX_BUFFER;
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->recvBytes = 0;
	pSocket->sendBytes = 0;

	dwFlags = 0;

	// 클라이언트로부터 다시 응답을 받기 위해 WSARecv 를 호출해줌
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

		// 중첩 소켓을 지정하고 완료시 실행될 함수를 넘겨줌
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
			printf_s("[ERROR] IO Pending 실패 : %d", WSAGetLastError());
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

// static 변수 초기화
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

	// DB 접속
	/*if (Conn.Connect(DB_ADDRESS, DB_ID, DB_PW, DB_SCHEMA, DB_PORT))
	{
		printf_s("[INFO] DB 접속 성공\n");
	}
	else {
		printf_s("[ERROR] DB 접속 실패\n");
	}*/

	// 패킷 함수 포인터에 함수 지정
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
	// winsock 의 사용을 끝낸다
	WSACleanup();
	// 다 사용한 객체를 삭제
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

	// DB 연결 종료
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
	// 시스템 정보 가져옴
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	printf_s("CPU 갯수 : %d\n", sysInfo.dwNumberOfProcessors);
	// 적절한 작업 스레드의 갯수는 (CPU * 2) + 1
	nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

	// thread handler 선언
	//hWorkerHandle = new HANDLE[nThreadCnt];
	// thread 생성
	/*for (int i = 0; i < nThreadCnt; i++)
	{
		hWorkerHandle[i] = (HANDLE *)_beginthreadex(
			NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId
		);
		if (hWorkerHandle[i] == NULL)
		{
			printf_s("[ERROR] Worker Thread 생성 실패\n");
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
		printf_s("[ERROR] WSASend 실패 : ", WSAGetLastError());
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
		printf_s("[ERROR] Monster Thread 생성 실패\n");
		return;
	}
	ResumeThread(MonsterHandle);

	printf_s("[INFO] Monster Thread 시작...\n");
}

void MainIocp::MonsterManagementThread()
{
	// 몬스터 초기화
	InitializeMonsterSet();
	int count = 0;	
	// 로직 시작
	while (true)
	{
		/*for (auto & kvp : MonstersInfo.monsters)
		{
			auto & monster = kvp.second;
			for (auto & player : CharactersInfo.players)
			{
				// 플레이어나 몬스터가 죽어있을 땐 무시
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
		// 0.5초마다 클라이언트에게 몬스터 정보 전송
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
	// 몬스터 초기화	
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
	// 함수 호출 성공 여부
	BOOL	bResult;
	int		nResult;
	// Overlapped I/O 작업에서 전송된 데이터 크기
	DWORD	recvBytes;
	DWORD	sendBytes;
	// Completion Key를 받을 포인터 변수
	stSOCKETINFO *	pCompletionKey;
	// I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터	
	stSOCKETINFO *	pSocketInfo;
	DWORD	dwFlags = 0;


	while (bWorkerThread)
	{
		/**
		 * 이 함수로 인해 쓰레드들은 WaitingThread Queue 에 대기상태로 들어가게 됨
		 * 완료된 Overlapped I/O 작업이 발생하면 IOCP Queue 에서 완료된 작업을 가져와
		 * 뒷처리를 함
		 */
		bResult = GetQueuedCompletionStatus(hIOCP,
			&recvBytes,				// 실제로 전송된 바이트
			(PULONG_PTR)&pCompletionKey,	// completion key
			(LPOVERLAPPED *)&pSocketInfo,			// overlapped I/O 객체
			INFINITE				// 대기할 시간
		);

		if (!bResult && recvBytes == 0)
		{
			printf_s("[INFO] socket(%d) 접속 끊김\n", pSocketInfo->socket);
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
			// 패킷 종류
			int PacketType;
			// 클라이언트 정보 역직렬화
			stringstream RecvStream;

			RecvStream << pSocketInfo->dataBuf.buf;
			RecvStream >> PacketType;

			// 패킷 처리
			//if (fnProcess[PacketType].funcProcessPacket != nullptr)
			{
				//fnProcess[PacketType].funcProcessPacket(RecvStream, pSocketInfo);
			}
			//else
			{
				printf_s("[ERROR] 정의 되지 않은 패킷 : %d\n", PacketType);
			}
		}
		catch (const std::exception& e)
		{
			printf_s("[ERROR] 알 수 없는 예외 발생 : %s\n", e.what());
		}

		// 클라이언트 대기
		Recv(pSocketInfo);
	}
}

void MainIocp::SignUp(stringstream & RecvStream, stSOCKETINFO * pSocket)
{
	string Id;
	string Pw;

	RecvStream >> Id;
	RecvStream >> Pw;

	printf_s("[INFO] 회원가입 시도 {%s}/{%s}\n", Id, Pw);

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

	printf_s("[INFO] 로그인 시도 {%s}/{%s}\n", Id, Pw);

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

	printf_s("[INFO][%d]캐릭터 등록 - X : [%f], Y : [%f], Z : [%f], Yaw : [%f], Alive : [%d], Health : [%f]\n",
		info.SessionId, info.X, info.Y, info.Z, info.Yaw, info.IsAlive, info.HealthValue);

	EnterCriticalSection(&csPlayers);

	cCharacter* pinfo = &CharactersInfo.players[info.SessionId];

	// 캐릭터의 위치를 저장						
	pinfo->SessionId = info.SessionId;
	pinfo->X = info.X;
	pinfo->Y = info.Y;
	pinfo->Z = info.Z;

	// 캐릭터의 회전값을 저장
	pinfo->Yaw = info.Yaw;
	pinfo->Pitch = info.Pitch;
	pinfo->Roll = info.Roll;

	// 캐릭터의 속도를 저장
	pinfo->VX = info.VX;
	pinfo->VY = info.VY;
	pinfo->VZ = info.VZ;

	// 캐릭터 속성
	pinfo->IsAlive = info.IsAlive;
	pinfo->HealthValue = info.HealthValue;
	pinfo->IsAttacking = info.IsAttacking;

	LeaveCriticalSection(&csPlayers);

	SessionSocket[info.SessionId] = pSocket->socket;

	printf_s("[INFO] 클라이언트 수 : %d\n", SessionSocket.size());

	//Send(pSocket);
	BroadcastNewPlayer(info);*/
}

void MainIocp::SyncCharacters(stringstream& RecvStream, stSOCKETINFO* pSocket)
{
	/*cCharacter info;
	RecvStream >> info;

	// 	 	printf_s("[INFO][%d]정보 수신 - %d\n",
	// 	 		info.SessionId, info.IsAttacking);	
	EnterCriticalSection(&csPlayers);

	cCharacter * pinfo = &CharactersInfo.players[info.SessionId];

	// 캐릭터의 위치를 저장						
	pinfo->SessionId = info.SessionId;
	pinfo->X = info.X;
	pinfo->Y = info.Y;
	pinfo->Z = info.Z;

	// 캐릭터의 회전값을 저장
	pinfo->Yaw = info.Yaw;
	pinfo->Pitch = info.Pitch;
	pinfo->Roll = info.Roll;

	// 캐릭터의 속도를 저장
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
	printf_s("[INFO] (%d)로그아웃 요청 수신\n", SessionId);
	EnterCriticalSection(&csPlayers);
	CharactersInfo.players[SessionId].IsAlive = false;
	LeaveCriticalSection(&csPlayers);
	SessionSocket.erase(SessionId);
	printf_s("[INFO] 클라이언트 수 : %d\n", SessionSocket.size());
	WriteCharactersInfoToSocket(pSocket);*/
}

void MainIocp::HitCharacter(stringstream & RecvStream, stSOCKETINFO * pSocket)
{
	// 피격 처리된 세션 아이디
	/*int DamagedSessionId;
	RecvStream >> DamagedSessionId;
	printf_s("[INFO] %d 데미지 받음 \n", DamagedSessionId);
	EnterCriticalSection(&csPlayers);
	CharactersInfo.players[DamagedSessionId].HealthValue -= HitPoint;
	if (CharactersInfo.players[DamagedSessionId].HealthValue < 0)
	{
		// 캐릭터 사망처리
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
	// 몬스터 피격 처리
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

	// 다른 플레이어에게 브로드캐스트
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

	// 직렬화	
	//SendStream << EPacketType::RECV_PLAYER << endl;
	//SendStream << CharactersInfo << endl;

	// !!! 중요 !!! data.buf 에다 직접 데이터를 쓰면 쓰레기값이 전달될 수 있음
	CopyMemory(pSocket->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->dataBuf.len = SendStream.str().length();
}

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
	unsigned int threadId;
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
	int nResult;
	// 클라이언트 정보
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	DWORD recvBytes;
	DWORD flags;

	// Completion Port 객체 생성
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), hIOCP, (DWORD)100000, 0);

	// Worker Thread 생성
	if (!CalculateWorkerThread()) return;	

	printf_s("Server Start...\n");

	// 클라이언트 접속을 받음
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

		// 중첩 소켓을 지정하고 완료시 실행될 함수를 넘겨줌
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
			printf_s("[ERROR] IO Pending 실패 : %d", WSAGetLastError());
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


	// 함수 호출 성공 여부
	BOOL	bResult;
	int		nResult;
	// Overlapped I/O 작업에서 전송된 데이터 크기
	DWORD	recvBytes;
	DWORD	sendBytes;
	// Completion Key를 받을 포인터 변수
	stSOCKETINFO *	pCompletionKey;
	// I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터	
	stSOCKETINFO *	pSocketInfo;
	DWORD	dwFlags = 0;


	//while (bWorkerThread)
	{
		/**
		 * 이 함수로 인해 쓰레드들은 WaitingThread Queue 에 대기상태로 들어가게 됨
		 * 완료된 Overlapped I/O 작업이 발생하면 IOCP Queue 에서 완료된 작업을 가져와
		 * 뒷처리를 함
		 */
		bResult = GetQueuedCompletionStatus(hIOCP,
			&recvBytes,				// 실제로 전송된 바이트
			(PULONG_PTR)&pCompletionKey,	// completion key
			(LPOVERLAPPED *)&pSocketInfo,			// overlapped I/O 객체
			INFINITE				// 대기할 시간
		);

		if (!bResult && recvBytes == 0)
		{
			printf_s("[INFO] socket(%d) 접속 끊김\n", pSocketInfo->socket);
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
			// 패킷 종류
			int PacketType;
			// 클라이언트 정보 역직렬화
			stringstream RecvStream;

			RecvStream << pSocketInfo->dataBuf.buf;
			RecvStream >> PacketType;

			// 패킷 처리
			//if (fnProcess[PacketType].funcProcessPacket != nullptr)
			{
				//fnProcess[PacketType].funcProcessPacket(RecvStream, pSocketInfo);
			}
			//else
			{
				printf_s("[ERROR] 정의 되지 않은 패킷 : %d\n", PacketType);
			}
		}
		catch (const std::exception& e)
		{
			printf_s("[ERROR] 알 수 없는 예외 발생 : %s\n", e.what());
		}

		// 클라이언트 대기
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