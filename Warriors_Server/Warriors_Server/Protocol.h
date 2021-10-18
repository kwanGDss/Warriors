#pragma once

constexpr int MAX_NAME = 16;

#pragma pack (push, 1)

constexpr int MAX_BUFFER = 1024;
constexpr int SERVER_PORT = 8000;
constexpr int MAX_USER = 100;

constexpr int NOT_INGAME = -1;
constexpr int TO_SERVER = 0;
constexpr int TO_CLIENT = 1;
constexpr int CLIENT_LOGIN_ASK = 2;
constexpr int SERVER_LOGIN_ASK = 3;

constexpr int CLIENT_LOGOUT =			0;
constexpr int CLIENT_LOGIN =			1;
constexpr int CLIENT_CHANGE_CHARACTER = 2;
constexpr int CLIENT_REDUCE_STAMINA =	3;
constexpr int CLIENT_ATTACK =			4;
constexpr int CLIENT_START =			5;
constexpr int CLIENT_TICK =				6;
constexpr int CLIENT_GUARD =			7;
constexpr int CLIENT_PARRYING =			8;
constexpr int CLIENT_GROGGY =			9;
constexpr int CLIENT_GUARD_HIT =		10;
constexpr int CLIENT_BE_HIT =			11;

constexpr int SERVER_LOGIN_FAIL =		0;
constexpr int SERVER_LOGIN_OK =			1;
constexpr int SERVER_PLAYER_STATUS =	2;
constexpr int SERVER_ENEMY_STATUS =		3;
constexpr int SERVER_START =			4;
constexpr int SERVER_TICK =				5;
constexpr int SERVER_ATTACK =			6;

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

struct client_packet_change_character
{
	unsigned char	size;
	unsigned char	type;
	bool			change_character;
};

struct client_packet_start
{
	unsigned char	size;
	unsigned char	type;
	bool			character_type;
};

struct client_packet_be_hit
{
	unsigned char	size;
	unsigned char	type;
	bool			be_hit;
};

struct client_packet_guard
{
	unsigned char	size;
	unsigned char	type;
	bool			guard;
};

struct client_packet_groggy
{
	unsigned char	size;
	unsigned char	type;
	bool			groggy;
	bool			who;
};

struct client_packet_guard_hit
{
	unsigned char	size;
	unsigned char	type;
	bool			guard_hit;
	bool			who;
};

struct client_packet_reduce_stamina
{
	unsigned char	size;
	unsigned char	type;
	float			reduce_stamina;
};

struct client_packet_reduce_health
{
	unsigned char	size;
	unsigned char	type;
	float			reduce_health;
};

struct client_packet_parrying
{
	unsigned char	size;
	unsigned char	type;
	bool			parrying;
};

struct client_packet_tick
{
	unsigned char	size;
	unsigned char	type;
	bool			be_hit;
	bool			guard_hit;
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
	int				enemy_id;
};

struct server_packet_login_fail
{
	unsigned char	size;
	unsigned char	type;
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

struct server_packet_start
{
	unsigned char	size;
	unsigned char	type;
	bool			enemy_character_type;
};

struct server_packet_attack
{
	unsigned char	size;
	unsigned char	type;
	bool			enemy_be_hit;
};

struct server_packet_tick
{
	unsigned char	size;
	unsigned char	type;
	float			player_hp;
	float			player_stamina;
	float			enemy_x;
	float			enemy_y;
	float			enemy_hp;
	bool			player_be_hit;
	bool			player_guard_hit;
	bool			enemy_guard;
	bool			enemy_parrying;
	bool			enemy_groggy;
	bool			enemy_guard_hit;
};
#pragma pack (pop)