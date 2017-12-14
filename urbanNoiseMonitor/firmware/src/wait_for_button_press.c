#include "wait_for_button_press.h"

APP_STATES wait_for_button_press(void)
{
    if (DEBUG_MODE){sendMsg(appData.usart_debugger, "\n*********************************\r\n***** WAIT_FOR_BUTTON_PRESS *****\r\n*********************************\r\n");}
    
    while(SWITCH_S4StateGet());
    appData.periods_elapsed = 0;
    TimerSetup();
    while(appData.periods_elapsed < 3*TIMER_FREQ){
        if (SWITCH_S4StateGet() && appData.periods_elapsed > 0.5*TIMER_FREQ){
            DRV_TMR_Stop(appData.timer_40k);
            appData.periods_elapsed = 0;
            return APP_STATE_JOIN;
        }
    }
    DRV_TMR_Stop(appData.timer_40k);
    appData.periods_elapsed = 0;
    
    if (!appData.error){
        return APP_STATE_CALIBRATE;   
    }
    else{
        return APP_STATE_ERROR;
    }
}
