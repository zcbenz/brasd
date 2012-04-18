bras图形客户端0.1.2
===================

brasd是一个图形端的bras拨号程序，可以自动照料好网关和路由表。

安装方法
--------

对于Ubuntu用户，可以从网络进行安装：

    sudo add-apt-repository ppa:zcbenz/brasd
    sudo apt-get update
    sudo apt-get install brasd

或者直接安装附件中的deb包。

对于其他发行版的用户，需要自行编译源码进行安装：

    tar zxvf brasd-0.1.2.tar.gz
    cd brasd-0.1.2
    ./configure --prefix /usr
    make
    sudo make install
    sudo make install-service # 安装系统服务

注意：
如果你的发行版的系统脚本没有储存在/etc/init.d/，请不要执行make install-service，请将源码树根目录下的brasd.init文件复制到你的系统
脚本目录中。

程序依赖
--------

libevent 和 gtkmm

更新日志
--------

* bras-client可以记住多个账号和密码
* brasd支持多个客户端
* brasd启动時会结束正在运行的实例
* brasd使用/etc/brasd来进行设置
* 路由表分离到单独的文件中
* 代码更加健壮

已知问题
--------

连接到bras后，如果中途切换了网络，可能会导致网络不正常，这时请重新连接一次
网络。这是由于在和bras服务器的连接断开后，xl2tpd不会马上结束，而是重试多次
后才会结束，此时会导致切换后的网络的路由表被破坏。

如何回报问题
------------

1. 首先以debug模式启动brasd：
   sudo brasd -D
   如果brasd输出 "Cannot init server, brasd already runs?" 后退出，
   请多试几次

2. 然后在另一个终端下记下此时的路由表：
   route -n

3. 进行拨号

4. 记下此时的路由表：
   route -n

5. 请把上面所有命令的输出记录下来，贴在回报问题的帖子中

6. 你可以在虎踞龙盘BBS的Linux版块回报问题，但我还是建议在本项目的Issues里进行回报：
   https://github.com/zcbenz/brasd/issues

脚本化
------

对bras的控制通过sockets进行，也就是说你可以通过脚本来控制bras的状态。

查看bras状态：

    echo "STAT" | netcat 127.0.0.1 10086

设置用户名和密码：

    echo "SET username password" | netcat 127.0.0.1 10086

连接bras：

    echo "CONNECT" | netcat 127.0.0.1 10086

断开bras：

    echo "DISCONNECT" | netcat 127.0.0.1 10086

出于安全考量，默认情况下你无法远程控制bras，你可以在/etc/brasd中加上下面
的设置来打开网络控制：

    internet on

这样将上面的127.0.0.1修改为远程机器的密码即可远程控制bras。
