#include "errorHandler.h"

APP_STATES errorHandler(void)
{
    char msg[MAX_STR_LEN] = "\0";
    if (DEBUG_MODE){sendMsg(appData.usart_debugger, "\n***********************\r\n***** ERROR STATE *****\r\n***********************\r\n");}
    if (DEBUG_MODE){sprintf(msg, "Error code: %d\r\n", appData.error); sendMsg(appData.usart_debugger,msg);}
    
    if (appData.error == COULDNT_JOIN_NETWORK){
        // handle errpr 
    }
    if (appData.error == MEASUREMENT_BUFFER_FULL){
        // handle error
    }
    if (appData.error == CONVERSION_DIDNT_COMPLETE){
        // handle error
    }
    else{
        // handle error
    }
    
    while(1){}
    
}
