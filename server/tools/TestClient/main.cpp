#include "login.pb.h"
#include "comm_def.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS

using namespace protocol;

extern "C"
{
#include "socket_server.h"
};


static void * _poll(void * ud)
{
	struct socket_server *ss = (struct socket_server *)ud;
	struct socket_message result;
	for (;;)
	{
		int type = socket_server_poll(ss, &result, NULL);
		// DO NOT use any ctrl command (socket_server_close , etc. ) in this thread.
		switch (type)
		{
		case SOCKET_EXIT:
			return NULL;
		case SOCKET_DATA:
			printf("message(%" PRIuPTR ") [id=%d] size=%d\n",result.opaque,result.id, result.ud);
			free(result.data);
			break;
		case SOCKET_CLOSE:
			printf("close(%" PRIuPTR ") [id=%d]\n",result.opaque,result.id);
			break;
		case SOCKET_OPEN:
			printf("open(%" PRIuPTR ") [id=%d] %s\n",result.opaque,result.id,result.data);
			break;
		case SOCKET_ERROR:
			printf("error(%" PRIuPTR ") [id=%d]\n",result.opaque,result.id);
			break;
		case SOCKET_ACCEPT:
			printf("accept(%" PRIuPTR ") [id=%d %s] from [%d]\n",result.opaque, result.ud, result.data, result.id);
			socket_server_start(ss, 300, result.ud);
			break;
		}
	}
}

int main(int argc, char* argv[])
{

	struct socket_server * ss = socket_server_create();
	int32_t conn_id = socket_server_connect(ss, 100, "127.0.0.1", 8888);

	pthread_t pid;
	pthread_create(&pid, NULL, _poll, (void*)ss);

	char buf[1024];

	login_req req;

	req.set_id(10);
	req.add_item(100);

	int32_t byte_size = req.ByteSize();
	if (!req.SerializeToArray(buf, byte_size))
	{

	}

	while (fgets(buf, sizeof(buf), stdin) != NULL)
	{
		if (strncmp(buf, "quit", 4) == 0)
			break;
		buf[strlen(buf)-1] = '\n';		// 去除\n
		char* sendbuf = (char*)malloc(sizeof(buf)+1);
		memcpy(sendbuf, buf, strlen(buf)+1);
		socket_server_send(ss, conn_id, sendbuf, strlen(sendbuf));
	}


	socket_server_exit(ss);
	pthread_join(pid, NULL);

	socket_server_release(ss);
	return 0;
}
