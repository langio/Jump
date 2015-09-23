#include "login.pb.h"
#include "comm_def.h"

using namespace protocol;

extern "C"
{
#include "socket_server.h"
};

int main(int argc, char* argv[])
{

	struct socket_server * ss = socket_server_create();
	int32_t conn_id = socket_server_connect(ss, 100, "127.0.0.1", 8888);

	char buff[1024];

	login_req req;

	req.set_id(10);
	req.add_item(100);

	int32_t byte_size = req.ByteSize();
	if (!req.SerializeToArray(buff, byte_size))
	{

	}

	while(1)
	{
		//发送数据
		if(-1 == socket_server_send(ss, conn_id, buff, byte_size))
		{
			cout << "socket_server_send" << endl;
		}

		sleep(1);
	}

	socket_server_release(ss);
	return 0;
}
