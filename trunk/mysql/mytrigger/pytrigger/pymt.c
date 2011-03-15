#include "pymt.h"
#include <Python.h>

int init_proc (char *err_msg, char* modulename)
{
    
    if ( !Py_IsInitialized() )
    {
        printf("( protocol ) Init Python Evn\r\n");
        Py_InitializeEx(0);
        printf("init OK\r\n");
    }
    
    import_py_mtmod();
    
    printf("IMPORT OK");
    
    return _init_proc( err_msg, modulename );
}


int deinit_proc ( char* err_mgs )
{
    int r;
    
    r = _deinit_proc( err_mgs );
    Py_Finalize();
    
    return r;
}

