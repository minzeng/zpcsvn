MYSQL(1)                     MySQL Database System                    MYSQL(1)

NAME
    myreplicator - ��������MySQL���ƹ���

SYNOPSIS
    myreplicator [options] remote_binlog_name
    
DESCRIPTION
        myreplicator��һ��������mysql�����ݿ⸴�Ƴ��򣬵�����mysql������
    ���ܵ�ԭ����ͬ�����Ǵ�master��ȡbinlog������binlog�������slave��ִ��
    SQL��䡣������������mysql����ˣ�myreplicator������ص��ǿ���ʹһ��
    slaveָ����master��
        myreplicator�������ļ���ʽ��mysql�������ļ���ͬ�����Ǳ�׼��INI��ʽ��
    myreplicatorʹ�á�myreplicator����Ϊģ���ʶ��
    ���磺
        [myreplicator]
        #master configure
        host=10.210.74.143
        #slave configure
        slave-host=10.210.74.143
        #master.info
        relay-info=/tmp/re.info
    ������ؼ���һ����������relay-info����ʵʱ��¼��binlog��ִ��λ�ã�Ϊ
    myreplicator�˳���Ӷϵ����ִ���ṩ��λ�����ݡ����ĸ�ʽ��master.info
    �ļ��ĸ�ʽ��ͬ��
    
myreplicator OPTIONS
    ����������Ϣ��
        host=������������
        port=���������˿�
        user=�������û���
        password=�������û�����
        start-position=��������ʼλ�ã���relay-info�ļ�ʱ��Ч
    ����������Ϣ��
        slave-host=������������
        slave-port=���������˿�
        slave-user=�������û���
        slave-pass=�������û�����
    �������ã�
        relay-info=��ͬ��λ����Ϣ�ļ���ʵʱ��¼binlog��ִ��λ�ã�Ĭ��ֵΪ[./relay.info]
        defaults-file=��ָ�������ļ�
        daemon�����ػ����̵ķ�ʽ����
        self-server-id=��myreplicator������ݵķ�������ɫ��ע�ⲻ��Ϊ0��
        replication-ignore-db=�����˵�ָ���������
        ignore-server-id=�����˵�ָ��server-id����־��¼
        log-file=��myreplicator��־�ļ���Ĭ��ֵ[./myrpl.log]
        skip-slave-error������slaveд�����ݳ���ʱ�����ø�ѡ����Դ��󣬷������ֹͣ��
        
    ��ʾ��
        ����ѡ��Ҫ��Ϊ�����в���Ҫ��ǰ���"--"
        ����ѡ����Ⱥŵı�ʾ��Ҫ������������Ҫ
        
relay-info file format
    ���ļ���ʽ��master.info������ͬ��ֻ��������һ��self server id
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