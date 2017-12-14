#include "calibrate.h"

APP_STATES calibrate(void)
{
    if (DEBUG_MODE){sendMsg(appData.usart_debugger, "\n***************************\r\n***** CALIBRATE STATE *****\r\n***************************\r\n");}
    
    uint32_t i;
    int raw_measurement;
    int totalBytesRead;
    char cmd[MAX_STR_LEN] = "\0";
    int tx_count;
    int bandLimits[] = {22,44,88,177,354,707,1414,2828,5657,11314,20000};
    
    appData.sensor_bias = 0.0;
    
    totalBytesRead = 0;
    myI2S_init(); 
    while(totalBytesRead < 2*SAMPLE_T*TIMER_FREQ){
        if (SPI2STAT & 1){
            raw_measurement = (int) (SPI2BUF >> 8);
            totalBytesRead++;
            if (totalBytesRead % 2){
                appData.sensor_bias += raw_measurement;
            }
        }
    }
    appData.sensor_bias /= totalBytesRead/2;
    saveToFlash(appData.sensor_bias,0);
    
    
    for (i = 0; i < NUM_FILTERS; i++){
        appData.filterBank[i].Leq = 0.0;
    }
    
    totalBytesRead = 0;
    myI2S_init(); 
    while(totalBytesRead < 2*SAMPLE_T*TIMER_FREQ){
        if (SPI2STAT & 1){
            raw_measurement = (int) (SPI2BUF >> 8);
            totalBytesRead++;
            if (totalBytesRead % 2){
                updateFilterBank((float) (raw_measurement - appData.sensor_bias));
            }
        }
    }
    for (i = 0; i < NUM_FILTERS; i++){
        appData.filterBank[i].gain = pow(10,LEQ_PER_BAND_dB/10) * (totalBytesRead/2) / appData.filterBank[i].Leq;
        saveToFlash(appData.filterBank[i].gain,i+1);
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    //appData.data_in_idx = 0;
    //appData.data_out_idx = 0;
    //appData.sensor_bias = 0.0;
    /*
    // compute sensor bias
    ADC_initialize();
    TimerSetup();
    appData.collectData = true;
    appData.periods_elapsed = 0;
    while(1){
        if (appData.data_in_idx - appData.data_out_idx > DATA_BUFFER_LEN){
            appData.error = MEASUREMENT_BUFFER_FULL;
            break;
        }
        if (!measurementsStopped && appData.periods_elapsed >= TIMER_FREQ*SAMPLE_T){ // stop taking measurements
            appData.collectData = false;
            DRV_TMR_Stop(appData.timer_40k);
            ADC_close();
            measurementsStopped = true;
        }
        if (appData.data_in_idx > appData.data_out_idx){
            appData.sensor_bias += (float) appData.data_buf[appData.data_out_idx % DATA_BUFFER_LEN];
            appData.data_out_idx++;
        }
        else if (measurementsStopped){
            appData.sensor_bias /= (float)(appData.data_out_idx);
            saveToFlash(appData.sensor_bias,0);
            break;
        }
    }
    
    // calibrate filter bank gains
    appData.data_in_idx = 0;
    appData.data_out_idx = 0;
    measurementsStopped = false;
    
    for (i = 0; i < NUM_FILTERS; i++){
        appData.filterBank[i].Leq = 0.0;
    }
    
    ADC_initialize();
    TimerSetup();
    appData.collectData = true;
    appData.periods_elapsed = 0;
    while (1){
        if (appData.data_in_idx - appData.data_out_idx > DATA_BUFFER_LEN){
            appData.error = MEASUREMENT_BUFFER_FULL;
            break;
        }
        if (!measurementsStopped && appData.periods_elapsed >= TIMER_FREQ*SAMPLE_T){ // stop taking measurements
            appData.collectData = false;
            DRV_TMR_Stop(appData.timer_40k);
            ADC_close();
            measurementsStopped = true;
        }
        if (appData.data_in_idx > appData.data_out_idx){
            raw_measurement = (float) appData.data_buf[appData.data_out_idx % DATA_BUFFER_LEN];
            updateFilterBank(raw_measurement);
            appData.data_out_idx++;
        }
        else if (measurementsStopped){
            for (i = 0; i < NUM_FILTERS; i++){
                appData.filterBank[i].gain = pow(10,LEQ_PER_BAND_dB/10) * appData.data_out_idx / appData.filterBank[i].Leq;
                saveToFlash(appData.filterBank[i].gain,i+1);
            }
            break;
        }
    } 
    */
    
    cmd[0] = '\0';
    sprintf(cmd, "sensor bias: %e\r\n", appData.sensor_bias);
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
    
    cmd[0] = '\0';
    for (i = 0; i < NUM_FILTERS; i++){
        sprintf(cmd, "gain for %d to %d band: %e\r\n", bandLimits[i], bandLimits[i+1], appData.filterBank[i].gain);
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
        return APP_STATE_JOIN;
    }
    else{
        return APP_STATE_ERROR;
    }
}
