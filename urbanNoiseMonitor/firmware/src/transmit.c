#include "transmit.h"

APP_STATES transmit(void)
{
    if (DEBUG_MODE){sendMsg(appData.usart_debugger, "\n**************************\r\n***** TRANSMIT STATE *****\r\n**************************\r\n");}
    
    uint32_t i;
    char cmd[MAX_STR_LEN] = "mac tx uncnf 1 ";
    int tx_count;
    
    // add the payload to the tx command
    for (i = 0; i < NUM_FILTERS; i++){
        sprintf(cmd + strlen(cmd), "%lX", *(unsigned long*) &appData.filterBank[i].Leq);
    }
    sprintf(cmd + strlen(cmd), "\r\n");
    
    // send the command
    sendMsg(appData.usart_RN2903,cmd);
    
    for (i=0;i<NUM_FILTERS;i++){
        cmd[0] = '\0';
        sprintf(cmd, "%f\r\n", appData.filterBank[i].Leq);
        tx_count = 0;
        while (1) 
        {
            if(!DRV_USART_TransmitBufferIsFull(appData.usart_debugger))
            {
                DRV_USART_WriteByte(appData.usart_debugger, cmd[tx_count++]);
            }
            if (cmd[tx_count] == '\0')
            {
                break;
            }
        }
    }
    
    
    
    if (!appData.error){
        return APP_STATE_IDLE;
    }
    else{
        return APP_STATE_ERROR;
    }
}