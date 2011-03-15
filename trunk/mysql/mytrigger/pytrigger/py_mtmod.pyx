cimport mtmod
import datatime
from time cimport *

#int init_proc(char *err_msg);
#int deinit_proc(char *err_msg);
#int i_proc(struct TRIGGER_DATA* data);
#int d_proc(struct TRIGGER_DATA* data);
#int u_proc(struct TRIGGER_DATA* data);


# MYSQL_TYPE_TINY         TINYINT                char
# MYSQL_TYPE_SHORT        SMALLINT               short int
# MYSQL_TYPE_LONG         INT                    int
# MYSQL_TYPE_LONGLONG     BIGINT                 long long int
# MYSQL_TYPE_FLOAT        FLOAT                  float
# MYSQL_TYPE_DOUBLE       DOUBLE                 double
# MYSQL_TYPE_TIME         TIME                   MYSQL_TIME
# MYSQL_TYPE_DATE         DATE                   MYSQL_TIME
# MYSQL_TYPE_DATETIME     DATETIME               MYSQL_TIME
# MYSQL_TYPE_TIMESTAMP    TIMESTAMP              MYSQL_TIME
# MYSQL_TYPE_STRING       CHAR
# MYSQL_TYPE_VAR_STRING   VARCHAR
# MYSQL_TYPE_TINY_BLOB    TINYBLOB/TINYTEXT
# MYSQL_TYPE_BLOB         BLOB/TEXT
# MYSQL_TYPE_MEDIUM_BLOB  MEDIUMBLOB/MEDIUMTEXT
# MYSQL_TYPE_LONG_BLOB    LONGBLOB/LONGTEXT

 


m = None

cdef object datatrans( mtmod.MY_DATA* d ):
    
    cdef int _int
    cdef time_t t
    
    # can't supported, i don't know the storage format
    if d.type == mtmod.MYSQL_TYPE_DECIMAL :
        return 
    if d.type == mtmod.MYSQL_TYPE_NEWDECIMAL :
        return
    
    
    # intiger type
    if d.type == mtmod.MYSQL_TYPE_TINY :
        _int = (<char*>d.data)[0]
        return _int
    if d.type == mtmod.MYSQL_TYPE_SHORT :
        return (<short int*>d.data)[0]
    if d.type == mtmod.MYSQL_TYPE_LONG :
        return (<int*>d.data)[0]
    if d.type == mtmod.MYSQL_TYPE_INT24 :
        _int = (<int*>d.data)[0] & 0x00ffffff
        return _int
    if d.type == mtmod.MYSQL_TYPE_LONGLONG :
        return (<long long*>d.data)[0]
    
    
    # float type
    if d.type == mtmod.MYSQL_TYPE_FLOAT :
        return (<float*>d.data)[0]
    if d.type == mtmod.MYSQL_TYPE_DOUBLE :
        return (<double*>d.data)[0]
    
    
    # null
    if d.type == mtmod.MYSQL_TYPE_NULL :
        return
    
    
    # datetime
    if d.type == mtmod.MYSQL_TYPE_TIMESTAMP :
        t = (<time_t *>d.data)[0]
        return t
    if d.type == mtmod.MYSQL_TYPE_DATE :
        m = (<char *>d.data)[:d.length]
        return datatime.datatime.strptime(m,'%Y:%m:%d')
    if d.type == mtmod.MYSQL_TYPE_TIME :
        m = (<char *>d.data)[:d.length]
        return datatime.datatime.strptime(m,'%H:%M:%S')
    if d.type == mtmod.MYSQL_TYPE_DATETIME :
        m = (<char *>d.data)[:d.length]
        return datatime.datatime.strptime(m,'%Y-%m-%d %H:%M:%S')
    if d.type == mtmod.MYSQL_TYPE_YEAR :
        return # not supported
    if d.type == mtmod.MYSQL_TYPE_NEWDATE :
        return # not supported
    
    
    # varchar
    if d.type == mtmod.MYSQL_TYPE_VARCHAR :
        m = (<char *>d.data)[:d.length]
        return m
    if d.type == mtmod.MYSQL_TYPE_TINY_BLOB :
        m = (<char *>d.data)[:d.length]
        return m
    if d.type == mtmod.MYSQL_TYPE_MEDIUM_BLOB :
        m = (<char *>d.data)[:d.length]
        return m
    if d.type == mtmod.MYSQL_TYPE_LONG_BLOB :
        m = (<char *>d.data)[:d.length]
        return m
    if d.type == mtmod.MYSQL_TYPE_BLOB :
        m = (<char *>d.data)[:d.length]
        return m
    if d.type == mtmod.MYSQL_TYPE_VAR_STRING :
        m = (<char *>d.data)[:d.length]
        return m
    if d.type == mtmod.MYSQL_TYPE_STRING :
        m = (<char *>d.data)[:d.length]
        return m
    
    # binary
    if d.type == mtmod.MYSQL_TYPE_BIT :
        return
        
    # enum / set
    if d.type == mtmod.MYSQL_TYPE_ENUM :
        return
    if d.type == mtmod.MYSQL_TYPE_SET :
        return
    
    
    return


cdef api int _init_proc (char *err_msg, char* modulename) except -1 :
    
    global m
    
    m = __import__( modulename )
    
    return 0
    
cdef api int _deinit_proc ( char* err_mgs ) except -1 :
    
    global m
    
    m = None
    
    return 0
    
    
cdef api int i_proc( mtmod.TRIGGER_DATA* data ) except -1:
    
    cdef int j
    
    rl = []
    
    for j from 0 <= j < data.filednum :
        rl.append( datatrans( data.row_list+j ) )

    m.process( 'INSERT',
               data.dbname,
               data.tbname,
               rl,
               None,
               data.logfile,
               data.log_pos,
    )
    
    return 0

cdef api int d_proc( mtmod.TRIGGER_DATA* data ) except -1:
    
    cdef int j
    
    rl = []
    
    for j from 0 <= j < data.filednum :
        rl.append( datatrans( data.row_list+j ) )

    m.process( 'DELETE',
               data.dbname,
               data.tbname,
               None,
               rl,
               data.logfile,
               data.log_pos,
    )
    
    return 0
    
cdef api int u_proc( mtmod.TRIGGER_DATA* data ) except -1:
    
    cdef int j
    
    rl = []
    rn = []
    
    for j from 0 <= j < data.filednum :
        rl.append( datatrans( data.row_list+j ) )
    for j from 0 <= j < data.filednum :
        rn.append( datatrans( data.row_list_update+j ) )

    m.process( 'UPDATE',
               data.dbname,
               data.tbname,
               rn,
               rl,
               data.logfile,
               data.log_pos,
    )
    
    return 0

