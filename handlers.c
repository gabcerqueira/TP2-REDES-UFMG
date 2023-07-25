#include "handlers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>


//1 pra close connection
//2 pra list users
//3 send to
//4 pra send all
//0 pra default
int handleCommand(char *msg)
{

    if (strstr(msg, "close connection") != NULL)
    { // select file

        return 1;
        
    }
    else if (strstr(msg, "list users") != NULL)
    { //send file
        return 2;
        
    }
    else if (strstr(msg, "send to") != NULL)
    { // exit
        
        return 3;
    }
    else if (strstr(msg, "send all") != NULL)
    { // exit
        
        return 4;
    }
    
    else
    {
        return 0;
    }
}



