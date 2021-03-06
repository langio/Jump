#include "send_util.h"

bool sendToClinet(const PkgHead& pkg_head, const google::protobuf::Message& message)
{
	char buff[MAX_BUFF];

	char *p = buff;

	int32_t pkg_head_size = sizeof(pkg_head);
	int32_t pkg_body_size = message.ByteSize();

	int32_t pkg_total_size = pkg_head_size + pkg_body_size;

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
	p_pkg_head->body_len = pkg_body_size;
	p_pkg_head->pack();
	p += sizeof(PkgHead);

	//协议包体
	if (!message.SerializeToArray(p, pkg_body_size))
	{
		LOG_ERROR(0, "message.SerializeToArray failed");
		break;
	}

	int32_t ret = 0;

	ret = skynet_socket_send(0, pkg_head.client_fd, buff, send_len);

	if(-1 == ret)
	{
		LOG_ERROR(0, "skynet_socket_send failed");
		break;
	}

	LOG_DEBUG(0, "send_len:%d  pkg_body_len:%d", send_len, pkg_body_size);

	return true;

	__END_PROC__


	return false;
}
