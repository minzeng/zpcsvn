MYSQL(1)                     MySQL Database System                    MYSQL(1)

NAME
    myreplicator - 第三方的MySQL复制工具

SYNOPSIS
    myreplicator [options] remote_binlog_name
    
DESCRIPTION
        myreplicator是一个独立于mysql的数据库复制程序，但它和mysql自身复制
    功能的原理相同，都是从master读取binlog，解析binlog，最后在slave上执行
    SQL语句。由于它独立于mysql，因此，myreplicator的最大特点是可以使一个
    slave指向多个master。
        myreplicator的配置文件格式和mysql的配置文件相同，都是标准的INI格式。
    myreplicator使用“myreplicator”作为模块标识。
    例如：
        [myreplicator]
        #master configure
        host=10.210.74.143
        #slave configure
        slave-host=10.210.74.143
        #master.info
        relay-info=/tmp/re.info
    其中最关键的一个配置项是relay-info，它实时记录着binlog的执行位置，为
    myreplicator退出后从断点继续执行提供了位置依据。它的格式与master.info
    文件的格式相同。
    
myreplicator OPTIONS
    主库配置信息：
        host=：主库主机名
        port=：主库服务端口
        user=：主库用户名
        password=：主库用户密码
        start-position=：复制起始位置，无relay-info文件时有效
    辅库配置信息：
        slave-host=：辅库主机名
        slave-port=：辅库服务端口
        slave-user=：辅库用户名
        slave-pass=：辅库用户密码
    其他配置：
        relay-info=：同步位置信息文件，实时记录binlog的执行位置，默认值为[./relay.info]
        defaults-file=：指定配置文件
        daemon：以守护进程的方式启动
        self-server-id=：myreplicator自身扮演的服务器角色。注意不能为0。
        replication-ignore-db=：过滤掉指定库的数据
        ignore-server-id=：过滤掉指定server-id的日志记录
        log-file=：myreplicator日志文件，默认值[./myrpl.log]
        skip-slave-error；当向slave写入数据出错时，设置该选项将忽略错误，否则程序停止。
        
    提示：
        以上选项要作为命令行参数要在前面加"--"
        以上选项带等号的表示需要参数，否则不需要
        
relay-info file format
    此文件格式与master.info大致相同，只在最后多了一个self server id
    mysql-bin.000003 => master binlog name
    106              => binglog position
    10.210.74.143    => master host
    repl             => master user name 
    123qwe           => master user password
    3306             => master port
    128              => self server id

COPYRIGHT

REFERENCES

SEE ALSO

AUTHOR
    zhaopengcheng<pengcheng2@staff.sina.com.cn>