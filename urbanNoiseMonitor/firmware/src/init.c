#include "init.h"

APP_STATES init(void)
{
    bool appInitialized = true;
    
    if (appData.timer_40k == DRV_HANDLE_INVALID)
    {
        appData.timer_40k = DRV_TMR_Open(APP_TMR_DRV, DRV_IO_INTENT_EXCLUSIVE);
        appInitialized &= ( DRV_HANDLE_INVALID != appData.timer_40k );
    }
   
    RN2903_MCLROff();
    pause_s(1);
    
    if (appData.usart_debugger == DRV_HANDLE_INVALID)
    {
        appData.usart_debugger = DRV_USART_Open(APP_DRV_USART_DEBUGGER, DRV_IO_INTENT_READWRITE|DRV_IO_INTENT_NONBLOCKING);
        appInitialized &= ( DRV_HANDLE_INVALID != appData.usart_debugger );
    }
    
    if (DEBUG_MODE){sendMsg(appData.usart_debugger, "\n**********************\r\n***** INIT STATE *****\r\n**********************\r\n");}
    
    if (appData.usart_RN2903 == DRV_HANDLE_INVALID)
    {
        appData.usart_RN2903 = DRV_USART_Open(APP_DRV_USART_RN2903, DRV_IO_INTENT_READWRITE|DRV_IO_INTENT_NONBLOCKING);
        appInitialized &= ( DRV_HANDLE_INVALID != appData.usart_RN2903 );
    }

    if (appData.handleFlashDrv == DRV_HANDLE_INVALID)
    {
        appData.handleFlashDrv = DRV_FLASH_Open(APP_FLASH_INDEX, DRV_IO_INTENT_EXCLUSIVE);
        appInitialized &= ( DRV_HANDLE_INVALID != appData.handleFlashDrv );
    }

    RN2903_MCLROn();
    waitForResponse(appData.usart_RN2903);
    
    if (appInitialized)
    {
        return APP_STATE_WAIT_FOR_BUTTON_PRESS;
    }
    else
    {
        return APP_STATE_ERROR;
    }
}