#pragma once

constexpr int MAX_NAME = 16;

#pragma pack (push, 1)

struct Player
{
	char id[16];
	float HP = 1.f, STAMINA = 1.f;
	short x_locate, y_locate;
};

constexpr int MAX_BUFFER = 4096;
constexpr int SERVER_PORT = 8000;
constexpr int MAX_USER = 100;

constexpr int BOARD_WIDTH = 400;
constexpr int BOARD_HEIGHT = 400;
constexpr int VIEW_DISTANCE = 5;
constexpr int WIDTH_INDEX = BOARD_WIDTH / VIEW_DISTANCE;

constexpr int NOT_INGAME = -1;
constexpr int TO_SERVER = 0;
constexpr int TO_CLIENT = 1;
constexpr int LOGIN_ASK = 2;

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
	int				move_time;
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

struct server_packet_login
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	short			x, y;
	short			hp, stamina;
};

struct server_packet_login_ok
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	char			name[MAX_NAME];
	short			x, y;
	char			o_type;
};

struct server_packet_move
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	short			x, y;
	int				move_time;
};

struct server_packet_logout
{
	unsigned char	size;
	unsigned char	type;
	int				id;
};

struct server_packet_players_status
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	float			stamina;
	float			health;
};

#pragma pack (pop)