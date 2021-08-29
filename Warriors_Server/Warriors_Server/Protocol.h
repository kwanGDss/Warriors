#pragma once

constexpr int MAX_NAME = 10;

#pragma pack (push, 1)

constexpr int MAX_BUFFER = 4096;
constexpr int SERVER_PORT = 8000;
constexpr int MAX_USER = 100;

constexpr int BOARD_WIDTH = 400;
constexpr int BOARD_HEIGHT = 400;
constexpr int VIEW_DISTANCE = 5;
constexpr int WIDTH_INDEX = BOARD_WIDTH / VIEW_DISTANCE;

constexpr unsigned char CLIENT_PACKET_LOGIN =		1;
constexpr unsigned char C2S_PACKET_MOVE =			2;
constexpr unsigned char S2C_PACKET_LOGIN_INFO =		3;
constexpr unsigned char SERVER_PACKET_PC_LOGIN =	4;
constexpr unsigned char S2C_PACKET_PC_MOVE =		5;
constexpr unsigned char S2C_PACKET_PC_LOGOUT =		6;
constexpr unsigned char S2S_PACKET_PC_ENTER_VP=		7;
constexpr unsigned char S2S_PACKET_PC_MOVE_VP =		8;
constexpr unsigned char S2S_PACKET_PC_OUT_VP =		9;

enum class OP_TYPE { OP_RECV = 0, OP_SEND = 1, OP_ACCEPT = 2, OP_DO_MOVE = 3};
enum class S_STATE { STATE_FREE = 0, STATE_CONNECTED = 1, STATE_INGAME = 2 };
enum class V_STATE { STATE_NEW = 0, STATE_MOVE = 1, STATE_DELETE = 2 };

struct client_packet_login
{
	unsigned char	size;
	unsigned char	type;
	char			name[MAX_NAME];
};

struct c2s_packet_move
{
	unsigned char	size;
	unsigned char	type;
	char			dir; // 0 UP 1 RIGHT 2 DOWN 3 LEFT
	int				move_time;
};

struct server_packet_login
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	short			x, y;
	short			hp, level;
};

struct s2c_packet_pc_login
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	char			name[MAX_NAME];
	short			x, y;
	char			o_type;
};

struct s2c_packet_pc_move
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	short			x, y;
	int				move_time;
};

struct s2c_packet_pc_logout
{
	unsigned char	size;
	unsigned char	type;
	int				id;
};

struct s2s_packet_pc_enter_vp
{
	unsigned char	size;
	unsigned char	type;
	int				id;
};

struct s2s_packet_pc_move_vp
{
	unsigned char	size;
	unsigned char	type;
	int				id;
	short			x, y;
	int				move_time;
};

struct s2s_packet_pc_out_vp
{
	unsigned char	size;
	unsigned char	type;
	int				id;
};

#pragma pack (pop)