![](https://img-blog.csdnimg.cn/20210118221347395.gif)

<font color=#888888 size=5 face="华文楷体">
来源：微信公众号「编程学习基地」
</font>

### server

测试服务器，ubuntu18.04

~~~c
$ cat /proc/version
Linux version 4.4.0-87-generic (buildd@lcy01-31) (gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4) ) #110-Ubuntu SMP Tue Jul 18 12:55:35 UTC 2017
~~~

编译运行

~~~c
make
./server
~~~

默认绑定本地ip，默认端口为8000，可通过选项设置默认端口,例如

~~~c
./server -p 8888
~~~

### client

编译运行

~~~c
make
./client
~~~

默认绑定ip为0.0.0.0，默认端口为8000，可通过选项设置默认端口,例如

~~~c
./client -s 0.0.0.0 -p 8000
~~~

`注:client只支持群聊不支持私聊，私聊请看client-qt,可以被动回复添加好友信息.`

### client-qt

操作系统:win10；qt版本: 5.9.9；

qmake编译，直接双击 `.pro` 文件，即可编译，无需任何配置

`注意在 common.h 处修改宏 SERVER_ADDR 为你启动服务器的地址`

界面相对简陋，主要界面如下

+ 登录界面

![登录界面](https://img-blog.csdnimg.cn/f36bb2c05e6047f2b1213c3d3c2c29cf.png)


+ 注册界面

![注册界面](https://img-blog.csdnimg.cn/c54a115dbcde41048198c8234092e068.png)


+ 聊天界面

![聊天界面](https://img-blog.csdnimg.cn/7a6d203c6b9748a49c8aef4ef68340e2.jpg)


+ 添加好友界面
![添加好友界面](https://img-blog.csdnimg.cn/dab3bb85e38f4a63a29766b2ef74d9c8.png)



支持的功能

+ 注册账号
+ 登录账号
+ 添加好友
+ 群聊
![群聊](https://img-blog.csdnimg.cn/addb8ab2aad6457483374f31b0443bcb.png)

+ 私聊

![私聊](https://img-blog.csdnimg.cn/ae10b2edf19f45f789aaf711a85c9a5b.png)


后续UI美化以及功能增加持续更新，关注微信公众号「编程学习基地」最快咨询..