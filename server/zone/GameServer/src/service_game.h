#ifndef __SERVICE_GAME_H_
#define __SERVICE_GAME_H_

typedef struct game_data
{
	int reserved;
}GameData;

extern "C"
{

//#include "skynet.h"
#include "skynet_server.h"

	GameData * game_create(void);
	void game_release(GameData *p_game_data);
	static int _cb(struct skynet_context * ctx, void * ud, int type, int session,
			uint32_t source, const void * msg, size_t sz);
	int game_init(GameData *p_game_data, struct skynet_context * ctx, char * parm);
};

class Game
{
public:
	static Game& getInstance()
	{
		static Game _instance;
		return _instance;
	};

	void cmdDispatch(struct skynet_context * ctx, const PkgHead& pkg_head, const char* msg_body);


private:
	Game(){}
	Game(const Game&);
	Game& operator=(const Game&);
};

#endif

