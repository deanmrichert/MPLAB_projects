#include "measure.h"

APP_STATES measure(void)
{
    if (DEBUG_MODE){sendMsg(appData.usart_debugger, "\n*************************\r\n***** MEASURE STATE *****\r\n*************************\r\n");}
    
    uint32_t i;
    int raw_measurement;
    char i2s_data_24bit[3];
    uint32_t totalBytesRead = 0;
    uint32_t totalNotRead = 0;
    //uint32_t numBytesRead;
    //uint32_t totalMeasurements;
    
    char cmd[MAX_STR_LEN] = "\0";
    int tx_count;
    
    for (i = 0; i < NUM_FILTERS; i++){
        appData.filterBank[i].Leq = 0.0;
    }
    
    appData.measurement_history_idx = 0;
    
    myI2S_init(); 
    while(totalBytesRead < 2*SAMPLE_T*TIMER_FREQ){
        if (SPI2STAT & (1 << 6)){
            SPI2STATCLR = 1 << 6;
            appData.error = I2S_RX_OVRFLW;
        }
        if (SPI2STAT & 1){
            raw_measurement = (int) (SPI2BUF >> 8);
            totalBytesRead++;
            if (totalBytesRead % 2){
                updateFilterBank((float) (raw_measurement - appData.sensor_bias));
                appData.measurement_history[appData.measurement_history_idx] = raw_measurement;
                appData.measurement_history_idx = (appData.measurement_history_idx + 1) % MEASUREMENT_HISTORY_HORIZON;
            }
        }
    }
    for (i = 0; i < NUM_FILTERS; i++){
        appData.filterBank[i].Leq = 10*log10(appData.filterBank[i].gain * appData.filterBank[i].Leq / (totalBytesRead/2));
    }
    
    
    /*
    totalMeasurements = 0;
    totalBytesRead = 0;
    do
    {
        //totalBytesRead = 0;
        //while (totalBytesRead < 3){
        //DRV_I2S_BaudSet(appData.i2s_adafruit,2560000,40000);
            numBytesRead = DRV_I2S_Read(appData.i2s_adafruit, &i2s_data_24bit[totalBytesRead],3-totalBytesRead);
            if (numBytesRead == DRV_I2S_READ_ERROR){
                //appData.error = ERROR_READING_I2S;
                //return APP_STATE_ERROR;
            }
            else {
                totalBytesRead += numBytesRead;
            }
        //}
        
        if (totalMeasurements % 2){
            raw_measurement = 65536*i2s_data_24bit[0] + 256*i2s_data_24bit[1] + i2s_data_24bit[2];
            appData.measurement_history[appData.measurement_history_idx] = raw_measurement;
            appData.measurement_history_idx = (appData.measurement_history_idx + 1) % MEASUREMENT_HISTORY_HORIZON;
            updateFilterBank((float) raw_measurement);
        }
        
        totalMeasurements++;
    } while( totalMeasurements < 10000); //2*SAMPLE_T*TIMER_FREQ );
    */
    /*
    for (i=0;i<50;i++){
        cmd[0] = '\0';
        sprintf(cmd, "%d\r\n", appData.measurement_history[i]);
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
    */
    
    if (!appData.error){
        return APP_STATE_PROCESS;
    }
    else{
        return APP_STATE_ERROR;
    }
}
