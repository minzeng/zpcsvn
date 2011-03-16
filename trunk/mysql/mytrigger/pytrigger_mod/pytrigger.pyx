
import Threading

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

 


cdef object datatrans( mytrigger.MY_DATA* d ):
    
    cdef int _int
    cdef time_t t
    
    # can't supported, i don't know the storage format
    if d.type == mytrigger.MYSQL_TYPE_DECIMAL :
        return 
    if d.type == mytrigger.MYSQL_TYPE_NEWDECIMAL :
        return
    
    
    # intiger type
    if d.type == mytrigger.MYSQL_TYPE_TINY :
        _int = (<char*>d.data)[0]
        return _int
    if d.type == mytrigger.MYSQL_TYPE_SHORT :
        return (<short int*>d.data)[0]
    if d.type == mytrigger.MYSQL_TYPE_LONG :
        return (<int*>d.data)[0]
    if d.type == mytrigger.MYSQL_TYPE_INT24 :
        _int = (<int*>d.data)[0] & 0x00ffffff
        return _int
    if d.type == mytrigger.MYSQL_TYPE_LONGLONG :
        return (<long long*>d.data)[0]
    
    
    # float type
    if d.type == mytrigger.MYSQL_TYPE_FLOAT :
        return (<float*>d.data)[0]
    if d.type == mytrigger.MYSQL_TYPE_DOUBLE :
        return (<double*>d.data)[0]
    
    
    # null
    if d.type == mytrigger.MYSQL_TYPE_NULL :
        return
    
    
    # datetime
    if d.type == mytrigger.MYSQL_TYPE_TIMESTAMP :
        t = (<time_t *>d.data)[0]
        return t
    if d.type == mytrigger.MYSQL_TYPE_DATE :
        m = (<char *>d.data)[:d.length]
        return datatime.datatime.strptime(m,'%Y:%m:%d')
    if d.type == mytrigger.MYSQL_TYPE_TIME :
        m = (<char *>d.data)[:d.length]
        return datatime.datatime.strptime(m,'%H:%M:%S')
    if d.type == mytrigger.MYSQL_TYPE_DATETIME :
        m = (<char *>d.data)[:d.length]
        return datatime.datatime.strptime(m,'%Y-%m-%d %H:%M:%S')
    if d.type == mytrigger.MYSQL_TYPE_YEAR :
        return # not supported
    if d.type == mytrigger.MYSQL_TYPE_NEWDATE :
        return # not supported
    
    
    # varchar
    if d.type == mytrigger.MYSQL_TYPE_VARCHAR :
        m = (<char *>d.data)[:d.length]
        return m
    if d.type == mytrigger.MYSQL_TYPE_TINY_BLOB :
        m = (<char *>d.data)[:d.length]
        return m
    if d.type == mytrigger.MYSQL_TYPE_MEDIUM_BLOB :
        m = (<char *>d.data)[:d.length]
        return m
    if d.type == mytrigger.MYSQL_TYPE_LONG_BLOB :
        m = (<char *>d.data)[:d.length]
        return m
    if d.type == mytrigger.MYSQL_TYPE_BLOB :
        m = (<char *>d.data)[:d.length]
        return m
    if d.type == mytrigger.MYSQL_TYPE_VAR_STRING :
        m = (<char *>d.data)[:d.length]
        return m
    if d.type == mytrigger.MYSQL_TYPE_STRING :
        m = (<char *>d.data)[:d.length]
        return m
    
    # binary
    if d.type == mytrigger.MYSQL_TYPE_BIT :
        return
        
    # enum / set
    if d.type == mytrigger.MYSQL_TYPE_ENUM :
        return
    if d.type == mytrigger.MYSQL_TYPE_SET :
        return
    
    
    return



cdef class Trigger( object ):
    
    cdef mytrigger.TRIGGER* c_trigger
    
    def __cinit__( self ):
        
        sefl.c_trigger = TRIGGER( )
        
        return
    
    def __init__( self ):
        pass
        
    def getdata( self ):
        
        cdef mytrigger.TRIGGER_DATA* d
        
        d = mytrigger.TAILQ_GET( queue )
        
        cdef int j
        
        rl = []
        rn = []
        
        for j from 0 <= j < d.filednum :
            rl.append( datatrans( d.row_list+j ) )
        for j from 0 <= j < d.filednum :
            rn.append( datatrans( d.row_list_update+j ) )
        
        mytrigger.TRIGGER_DATA_FREE( d )
        
        return rl, rn
        
    def _process( self ):
    
        while( True ):
            self.process( self.getdata() )
        
    def loop( self ):
    
        t = Threading.thread( Target = self._process )
        
        while( True ):
            mytrigger.loop( self.queue )
        
        t.join()
        
        return



