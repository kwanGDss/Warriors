#pragma once

constexpr int MAX_NAME = 16;

#pragma pack (push, 1)

constexpr int MAX_BUFFER = 1024;
constexpr int SERVER_PORT = 8000;
constexpr int MAX_USER = 100;

constexpr int BOARD_WIDTH = 400;
constexpr int BOARD_HEIGHT = 400;
constexpr int VIEW_DISTANCE = 5;
constexpr int WIDTH_INDEX = BOARD_WIDTH / VIEW_DISTANCE;

constexpr int NOT_INGAME = -1;
constexpr int TO_SERVER = 0;
constexpr int TO_CLIENT = 1;
constexpr int CLIENT_LOGIN_ASK = 2;
constexpr int SERVER_LOGIN_ASK = 3;

constexpr int CLIENT_LOGOUT =			0;
constexpr int CLIENT_LOGIN =			1;
constexpr int CLIENT_REDUCE_STAMINA =	2;
constexpr int CLIENT_ATTACK =			3;
constexpr int CLIENT_MOVE =				4;

constexpr int SERVER_LOGIN_FAIL =		0;
constexpr int SERVER_LOGIN_OK =			1;
constexpr int SERVER_PLAYER_STATUS =	2;
constexpr int SERVER_ENEMY_STATUS =		3;
constexpr int SERVER_PLAYER_MOVE =		4;

enum class OP_TYPE { OP_RECV = 0, OP_SEND = 1, OP_ACCEPT = 2, OP_DO_MOVE = 3};
enum class S_STATE { STATE_FREE = 0, STATE_CONNECTED = 1, STATE_INGAME = 2 };
enum class V_STATE { STATE_NEW = 0, STATE_MOVE = 1, STATE_DELETE = 2 };

enum class EPacketType
{
	SIGNUP_PLAYER,
	LOGIN_PLAYER,
	ENROLL_PLAYER,
	SEND_PLAYER,
	RECV_PLAYER,
	LOGOUT_PLAYER,
	HIT_PLAYER,
	DAMAGED_PLAYER,
	CHAT,
	ENTER_NEW_PLAYER
};

struct client_packet_login
{
	unsigned char	size;
	unsigned char	type;
	char			name[MAX_NAME];
};

struct client_packet_move
{
	unsigned char	size;
	unsigned char	type;
	char			dir; // 0 UP 1 RIGHT 2 DOWN 3 LEFT
};

struct client_packet_reduce_stamina
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	float			reduce_stamina;
};

struct client_packet_reduce_health
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	float			reduce_health;
};

struct client_packet_logout
{
	unsigned char	size;
	unsigned char	type;
};

struct server_packet_login
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	short			x, y;
	float			hp, stamina;
};

struct server_packet_login_ok
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	float			hp, stamina;
};

struct server_packet_login_fail
{
	unsigned char	size;
	unsigned char	type;
};

struct server_packet_move
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	int				x, y;
};

struct server_packet_logout
{
	unsigned char	size;
	unsigned char	type;
	int				id;
};

struct server_packet_player_status
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	float			stamina;
	float			health;
};

struct server_packet_enemy_status
{
	unsigned char	size;
	unsigned char	type;
	int				enemy_id;
	float			health;
};
#pragma pack (pop)