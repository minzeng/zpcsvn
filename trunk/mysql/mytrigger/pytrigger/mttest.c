#include "pymt.h"



int main( int argc, char *argv[] )
{
    char errmsg[1024];
    
    init_proc( errmsg, argv[1] );
    
    return 0;
}