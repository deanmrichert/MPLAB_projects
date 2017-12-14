#include "idle.h"

APP_STATES idle(void)
{
    if (DEBUG_MODE){sendMsg(appData.usart_debugger, "\n**********************\r\n***** IDLE STATE *****\r\n**********************\r\n");}
    
    // todo: enter low power mode
    
    pause_s(5);
    
    if (!appData.error){
        return APP_STATE_MEASURE;
    }
    else{
        return APP_STATE_ERROR;
    }
}