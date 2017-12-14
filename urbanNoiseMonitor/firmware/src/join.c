#include "join.h"

APP_STATES join(void)
{
    if (DEBUG_MODE){sendMsg(appData.usart_debugger, "\n**********************\r\n***** JOIN STATE *****\r\n**********************\r\n");}
    
    char msg[MAX_STR_LEN];
    int i;

    switch (appData.joinProcedure){
            default: 
            break;
            case FACTORY_RESET:
                sendMsg(appData.usart_RN2903,"sys factoryRESET\r\n");
                appData.joinProcedure = SET_ALL_PARAMS_AND_SAVE;
                return APP_STATE_JOIN;
            case SET_ALL_PARAMS_AND_SAVE:
                sendMsg(appData.usart_RN2903,"sys get hweui\r\n");
                strcpy(msg,"mac set deveui ");
                strcat(msg,(char*) appData.rx_buf);
                sendMsg(appData.usart_RN2903,msg);
                strcpy(msg,"mac set appkey ");
                strcat(msg,APPKEY);
                strcat(msg,"\r\n");
                sendMsg(appData.usart_RN2903,msg);
                strcpy(msg,"mac set appeui ");
                strcat(msg,APPEUI);
                strcat(msg,"\r\n");
                sendMsg(appData.usart_RN2903,msg);
                sendMsg(appData.usart_RN2903,"mac set devaddr 00000000\r\n");
				sendMsg(appData.usart_RN2903,"mac set nwkskey 00000000000000000000000000000000\r\n");
				sendMsg(appData.usart_RN2903,"mac set appskey 00000000000000000000000000000000\r\n");
				sendMsg(appData.usart_RN2903,"mac set adr off\r\n");
				sendMsg(appData.usart_RN2903,"mac set rx2 8 923300000\r\n");
                for (i = 0 ; i < 64 ; i++){ 
                    sprintf(msg,"mac set ch drrange %d 0 3\r\n",i);
                    sendMsg(appData.usart_RN2903,msg);
                    if ( i < 8 ){
                        sprintf(msg,"mac set ch status %d on\r\n",i);
                    }
                    else {
                        sprintf(msg,"mac set ch status %d off\r\n",i);
                    }
                    sendMsg(appData.usart_RN2903,msg);
                }
                sendMsg(appData.usart_RN2903,"mac save\r\n");
                sendMsg(appData.usart_RN2903,"sys reset\r\n");
                sendMsg(appData.usart_RN2903,"mac set pwridx 5\r\n");
				sendMsg(appData.usart_RN2903,"mac set dr 3\r\n");
				sendMsg(appData.usart_RN2903,"mac set bat 127\r\n");
				sendMsg(appData.usart_RN2903,"mac set retx 2\r\n");
				sendMsg(appData.usart_RN2903,"mac set linkchk 0\r\n");
				sendMsg(appData.usart_RN2903,"mac set rxdelay1 1000\r\n");
				sendMsg(appData.usart_RN2903,"mac set ar off\r\n");
				sendMsg(appData.usart_RN2903,"mac set sync 34\r\n");
				sendMsg(appData.usart_RN2903,"radio set mod lora\r\n");
				sendMsg(appData.usart_RN2903,"radio set pwr 2\r\n");
				sendMsg(appData.usart_RN2903,"radio set sf sf10\r\n");
				sendMsg(appData.usart_RN2903,"radio set crc off\r\n");
				sendMsg(appData.usart_RN2903,"radio set iqi on\r\n");
				sendMsg(appData.usart_RN2903,"radio set cr 4/5\r\n");
				sendMsg(appData.usart_RN2903,"radio set wdt 15000\r\n");
				sendMsg(appData.usart_RN2903,"radio set sync 34\r\n");
				sendMsg(appData.usart_RN2903,"radio set bw 500\r\n");
                sendMsg(appData.usart_RN2903,"mac join otaa\r\n");
            break;
            case JOIN_WITH_CURRENT_CONFIG: 
                sendMsg(appData.usart_RN2903,"sys reset\r\n");
                sendMsg(appData.usart_RN2903,"mac set dr 3\r\n");
                sendMsg(appData.usart_RN2903,"mac join otaa\r\n");
            break;
    }
    
    if (!joined()){
        appData.error = COULDNT_JOIN_NETWORK;
    }
    
    if (!appData.error){
        return APP_STATE_IDLE;
    }
    else{
        return APP_STATE_ERROR;
    }      
}
