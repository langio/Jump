#include "skynet_socket.h"
#include "pkg_head.pb.h"
#include <google/protobuf/message.h>

int sendToClinet(int socket_id, const pkg_head& head, const google::protobuf::Message& message)
{
	return 0;
}

