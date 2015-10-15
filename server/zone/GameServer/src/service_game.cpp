#include "login.pb.h"

#include "logic.h"

#include "send_util.h"
#include "public.h"
#include "service_game.h"
#include "util/yac_common.h"
#include "ini_parse.h"
#include "server_env.h"
#include "player_data_mgr.h"


using namespace util;
using namespace protocol;

GameData * game_create(void)
{
	GameData * inst = (GameData *)skynet_malloc(sizeof(*inst));
	inst->reserved = 0;

	LOG_DEBUG(0, "game_create");

	return inst;
}

//释放相关资源
void game_release(GameData *p_game_data)
{
	skynet_free(p_game_data);
}


static int _cb(struct skynet_context * ctx, void * ud, int type, int session,
		uint32_t source, const void * msg, size_t sz)
{
	GameData *p_game_data = (GameData *)ud;
	if(p_game_data)
	{

	}

	PkgHead pkg_head = *(PkgHead*)msg;
	const char* msg_body = msg + sizeof(PkgHead);

	switch (type)
	{
		case PTYPE_TEXT:
			Game::getInstance().cmdDispatch(ctx, pkg_head, msg_body);
			break;
		default:
			LOG_ERROR(ctx, "unknow type:%d", type);
	}

	return 0;
}


//初始化game
int game_init(GameData *p_game_data, struct skynet_context * ctx, char * parm)
{
	int ret = 0;

	skynet_callback(ctx, p_game_data, _cb);
	skynet_command(ctx, "REG", ".game");

	LOG_DEBUG(0, "game_init");

	if (!ServerEnv::getInstance().init())
	{
		INIT_LOG_ERROR("ServerEnv init failed");
		ret = -1;
	}

	ret = PlayerDataMgr::getInstance().Init();

	return ret;

}

void Game::cmdDispatch(struct skynet_context * ctx, const PkgHead& pkg_head, const char* msg_body)
{
	switch(pkg_head.cmd)
	{
	case CMD_LOGIN_REQ:
		break;
	}
}


