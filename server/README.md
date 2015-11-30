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

## 系统中用到的表

* 注册信息表：key是 reg_账号
* 昵称表：key是 nick_昵称，value是1，用于确保昵称的唯一性
* uid表：key是 sole_uid_in_one_redis，每次获取1个uid，value自增1，系统中实际使用的uid是获取到的uid+1000000
* profile表：key是 user\_uid_zoneid



