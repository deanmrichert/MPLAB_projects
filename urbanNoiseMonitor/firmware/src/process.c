#include "process.h"

APP_STATES process(void)
{
    if (DEBUG_MODE){sendMsg(appData.usart_debugger, "\n*************************\r\n***** PROCESS STATE *****\r\n*************************\r\n");}
    
    // todo: process data
    
    if (!appData.error){
        return APP_STATE_TRANSMIT;
    }
    else{
        return APP_STATE_ERROR;
    }
}
