## 消息格式

|消息长度 |消息内容       |
|--------|-----------|
|msg len |msg content|

* 消息长度：表示后面的消息内容的长度，big-edain消息长度字段是2字节或4字节，服务启动时由gate的启动参数决定是2字节还是4字节，_S_是2字节，_L_是4字节
* 消息内容：实际发送的消息内容
* 消息内容最多16M（3个字节），skynet内部传递消息时，把消息类型编码到了长度字段（sz），占1个字节

## gate参数说明

* header：消息的长度，S表示消息长度是2字节，L表示消息长度是4字节
* watchdog：要将消息转发到的服务的名字
* binding：要绑定的监听的地址
* client_tag：
* max：最大连接数
* buffer

##头文件顺序
* protobuf产生的头文件放在最上面

## shmkey分配
* 使用十进制  service(2位)+zone_id(4位)+key序号(3位)
* service_dispatcher 1
* service_game 2