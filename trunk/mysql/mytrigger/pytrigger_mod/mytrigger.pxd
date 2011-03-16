cdef extern from "mtmod.h" :
    
    struct MY_DATA :
        void* data
        int type
        int length
        
    struct TRIGGER_DATA :
        int ioperate_type
        char *logfile
        unsigned long log_pos
        int filednum
        char* dbname
        char* tbname
        MY_DATA* row_list
        MY_DATA* row_list_update
        int b_islast
    
    cdef enum CheeseState :
        MYSQL_TYPE_DECIMAL
        MYSQL_TYPE_TINY
        MYSQL_TYPE_SHORT
        MYSQL_TYPE_LONG
        MYSQL_TYPE_FLOAT
        MYSQL_TYPE_DOUBLE
        MYSQL_TYPE_NULL
        MYSQL_TYPE_TIMESTAMP
        MYSQL_TYPE_LONGLONG
        MYSQL_TYPE_INT24
        MYSQL_TYPE_DATE
        MYSQL_TYPE_TIME
        MYSQL_TYPE_DATETIME
        MYSQL_TYPE_YEAR
        MYSQL_TYPE_NEWDATE
        MYSQL_TYPE_VARCHAR
        MYSQL_TYPE_BIT
        MYSQL_TYPE_NEWDECIMAL=246
        MYSQL_TYPE_ENUM=247
        MYSQL_TYPE_SET=248
        MYSQL_TYPE_TINY_BLOB=249
        MYSQL_TYPE_MEDIUM_BLOB=250
        MYSQL_TYPE_LONG_BLOB=251
        MYSQL_TYPE_BLOB=252
        MYSQL_TYPE_VAR_STRING=253
        MYSQL_TYPE_STRING=254
        MYSQL_TYPE_GEOMETRY=255
        
    
    
    struct TRIGGER :
        pass
        
    TRIGGER* TRIGGER_init( void )
    int TRIGGER_deinit( TRIGGER* )
    TRIGGER_DATA* TRIGGER_getdata( TRIGGER* )
    int TRIGGER_freedata( TRIGGER*, TRIGGER_DATA* )
    int TRIGGER_cleardata( TRIGGER* )
    