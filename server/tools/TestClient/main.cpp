#include "login.pb.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS

#include "comm_def.h"
#include "public.h"

using namespace protocol;

extern "C"
{
#include "socket_server.h"
};

#undef LOG_ERROR
#define LOG_ERROR(ctx, msg, args...) printf("[ERROR] %s:%d|" msg "\n", __FILE__, __LINE__, ##args)

const int32_t MAX_BUFF = 102400;
struct socket_server * ss = NULL;

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

void send_msg(const PkgHead& pkg_head, const google::protobuf::Message& message)
{
	char buff[MAX_BUFF];

	char *p = buff;

	int32_t pkg_head_size = sizeof(pkg_head);
	int32_t pkg_msg_size = message.ByteSize();

	int32_t pkg_total_size = pkg_head_size + pkg_msg_size;

	__BEGIN_PROC__

	//消息头
	int32_t *header = (int32_t *)p;
	*header = htonl(pkg_total_size);
	p += sizeof(*header);

	int32_t send_len = sizeof(*header) + pkg_total_size;

	if(send_len > MAX_BUFF)
	{
		LOG_ERROR(0, "there is no enough buff. buff size:%d, real size:%d", MAX_BUFF, send_len);
		break;
	}


	//协议包头
	PkgHead *p_pkg_head = (PkgHead *)p;
	*p_pkg_head = pkg_head;
	p_pkg_head->body_len = pkg_msg_size;
	p_pkg_head->pack();
	p += sizeof(PkgHead);

	//协议包体
	if (!message.SerializeToArray(p, pkg_msg_size))
	{
		LOG_ERROR(0, "message.SerializeToArray failed");
		break;
	}

	char* sendbuf = (char*)malloc(send_len + 1);
	memcpy(sendbuf, buff, send_len + 1);

	socket_server_send(ss, pkg_head.client_fd, sendbuf, send_len);

	__END_PROC__
}

int main(int argc, char* argv[])
{

	ss = socket_server_create();
	int32_t conn_id = socket_server_connect(ss, 100, "127.0.0.1", 8888);

	pthread_t pid;
	pthread_create(&pid, NULL, _poll, (void*)ss);

	//char buf[1024] = {0};

	login_req req;

	req.set_uid(10);
//	for(int32_t i=0; i<10; ++i)
//	{
//		req.add_item(100+i);
//	}

//	int32_t byte_size = req.ByteSize();
//	if (!req.SerializeToArray(buf+sizeof(byte_size), byte_size))
//	{
//
//	}
//	int32_t *header = (int32_t*)buf;
//
//	int32_t data_len = byte_size;
//
//	printf("data_len:%d\n", data_len);
//
//	int32_t send_len = data_len + sizeof(byte_size);
//
//	*header = htonl(data_len);

	PkgHead pkg_head;

	pkg_head.cmd = CMD_LOGIN_REQ;
	pkg_head.client_fd = conn_id;

	int32_t counter = 0;
	//while (fgets(buf, sizeof(buf), stdin) != NULL)
	while(1)
	{
//		char* sendbuf = (char*)malloc(send_len + 1);
//		memcpy(sendbuf, buf, send_len + 1);
//
//		socket_server_send(ss, conn_id, sendbuf, send_len);

		send_msg(pkg_head, req);
		++counter;

		printf("counter:%d\n", counter);

		sleep(1);

		break;

	}


	socket_server_exit(ss);
	pthread_join(pid, NULL);

	socket_server_release(ss);
	return 0;
}
