extern "C"
{
#include "socket_server.h"
};

int main(int argc, char* argv[])
{

	struct socket_server * ss = socket_server_create();
	int32_t conn_id = socket_server_connect(ss, 0, "127.0.0.1", 8888);

	char buff[1024];
	//发送数据
	socket_server_send(ss, conn_id, buff, 1024);

	socket_server_release(ss);
	return 0;
}
