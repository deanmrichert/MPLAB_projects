#include "helper_fcns.h"


// String to write into the flash.  Be aware that the double quotes adds a null byte at the end of the string.
    // So, writing "Hello World!" actually stores 13 bytes.
    uint8_t appFlashWriteString[4*(NUM_FILTERS+1)]; 
            
/* Buffer in the KSEG1 RAM to store the data */
    uint32_t appFlashRowBuff[DRV_FLASH_ROW_SIZE/sizeof(uint32_t)] __attribute__((coherent, aligned(16)));
    /* FlashReserveArea refers to memory starting from address 
       FLASH_DRV_START_ADDRESS. The attribute keep instructs the linker not to remove the section
       even if it is not refered anywhere in the code.
     */
    const uint8_t appFlashReserveArea[APP_FLASH_DRV_BYTE_COUNT] __attribute__ ((keep)) __attribute__((address(APP_FLASH_DRV_START_ADDRESS))) = {0xFF, };


void pause_s(unsigned int num_s){
    appData.periods_elapsed = 0;
    TimerSetup();
    while(appData.periods_elapsed < num_s*TIMER_FREQ);
    DRV_TMR_Stop(appData.timer_40k);
    appData.periods_elapsed = 0;
}

void waitForResponse(DRV_HANDLE handle){
    unsigned int rx_count = 0;
    appData.periods_elapsed = 0;
    TimerSetup();
    while (1){
        if(!DRV_USART_ReceiverBufferIsEmpty(handle))
        {
            appData.rx_buf[rx_count++] = DRV_USART_ReadByte(handle);
            if ((appData.rx_buf[rx_count-1] == '\n') && (rx_count > 1) && (appData.rx_buf[rx_count-2] == '\r'))
            {
                appData.rx_buf[rx_count] = '\0';
                break;
            }
        }
        if (rx_count == MAX_STR_LEN){
            appData.error = RX_LEN_EXCEEDED_MAX; 
            break;
        } 
        if (appData.periods_elapsed >= RESPONSE_TIMEOUT*TIMER_FREQ){
            appData.error = TIMEOUT_WHILE_WAITING_FOR_RESPONSE; 
            break;
        }
    }
    DRV_TMR_Stop(appData.timer_40k);
    appData.periods_elapsed = 0;
    if (DEBUG_MODE && !appData.error){sendMsg(appData.usart_debugger,appData.rx_buf);}
    return;
}

void sendMsg(DRV_HANDLE handle, const char* msg){
    if (DEBUG_MODE && handle == appData.usart_RN2903){sendMsg(appData.usart_debugger,msg);}
    unsigned int tx_count = 0;
    while (1) 
    {
        if(!DRV_USART_TransmitBufferIsFull(handle))
        {
            DRV_USART_WriteByte(handle, msg[tx_count++]);
        }
        if (msg[tx_count] == '\0')
        {
            break;
        }
    }
    if (handle == appData.usart_RN2903){
        waitForResponse(handle);
        if ((!strcmp(msg,"mac join otaa\r\n") || !strncmp(msg,"mac tx",6)) && !strcmp((char*) appData.rx_buf,"ok\r\n")){
            waitForResponse(handle);
        }
    }
}

void TimerSetup( void )
{
    DRV_TMR_AlarmRegister(
        appData.timer_40k, 
        APP_TMR_DRV_PERIOD, 
        APP_TMR_DRV_IS_PERIODIC,
        (uintptr_t)NULL, 
        TimerCallback);
    DRV_TMR_Start(appData.timer_40k);
}

void TimerCallback (  uintptr_t context, uint32_t alarmCount )
{
    appData.periods_elapsed++;
}

bool joined(void){
    sendMsg(appData.usart_RN2903,"mac get status\r\n");
    return appData.rx_buf[7] % 2; 
}

void updateBiQuad(BiQuad *filter, float u, bool first_sos){
    
    float y2;

    /*
    y2 = (*filter).b[0]*u + (*filter).b[1]*(*filter).x[1] + (*filter).b[0]*(*filter).x[0];
    y2 -= (*filter).a[1]*(*filter).y[1] + (*filter).a[2]*(*filter).y[0];
    y2 /= (*filter).a[0];
    
    (*filter).x[0] = (*filter).x[1];
    (*filter).x[1] = u;
    
    (*filter).y[0] = (*filter).y[1];
    (*filter).y[1] = y2;
    */
    
    // this implementation assumes that a[0] = 1 and b[0] = b[2]
    
    y2 = (*filter).b[1]*(*filter).x[1];
    
    if (first_sos){
        y2 += (*filter).b[0]*u + (*filter).b[0]*(*filter).x[0];
    }
    else{
        y2 += u + (*filter).x[0];
    }
    
    y2 -= (*filter).a[1]*(*filter).y[1] + (*filter).a[2]*(*filter).y[0];
    
    (*filter).x[0] = (*filter).x[1];
    (*filter).x[1] = u;
    
    (*filter).y[0] = (*filter).y[1];
    (*filter).y[1] = y2;
    
}

void updateFilterBank(float u){
    
    uint8_t i,j;
    
    for (i = 0; i < NUM_FILTERS; i++){
        updateBiQuad(&appData.filterBank[i].sos[0],u,true);
        for (j = 1; j < FILTER_ORDER; j++){
            updateBiQuad(&appData.filterBank[i].sos[j],appData.filterBank[i].sos[j-1].y[1],false);
        } 
        appData.filterBank[i].Leq += appData.filterBank[i].sos[j-1].y[1]*appData.filterBank[i].sos[j-1].y[1];
    }
}

void ADC_initialize(void)
{
/* initialize ADC calibration setting */
     ADC0CFG = DEVADC0;
     ADC1CFG = DEVADC1;
     ADC2CFG = DEVADC2;
     ADC3CFG = DEVADC3;
     ADC4CFG = DEVADC4;
     //ADC5CFG = DEVADC5;
     //ADC6CFG = DEVADC6;
     ADC7CFG = DEVADC7;
/* Configure ADCCON1 */
ADCCON1 = 0; // No ADCCON1 features are enabled including: Stop-in-Idle, turbo,
// CVD mode, Fractional mode and scan trigger source.
/* Configure ADCCON2 */
ADCCON2 = 0; // Since, we are using only the Class 1 inputs, no setting is
// required for ADCDIV
/* Initialize warm up time register */
ADCANCON = 0;
ADCANCONbits.WKUPCLKCNT = 5; // Wakeup exponent = 32 * TADx
/* Clock setting */
ADCCON3 = 0;
ADCCON3bits.ADCSEL = 0; // Select input clock source
ADCCON3bits.CONCLKDIV = 1; // Control clock frequency is half of input clock
ADCCON3bits.VREFSEL = 0; // Select AVDD and AVSS as reference source
/* Select ADC sample time and conversion clock */
ADC0TIMEbits.ADCDIV = 1; // ADC0 clock frequency is half of control clock = TAD0
ADC0TIMEbits.SAMC = 5; // ADC0 sampling time = 5 * TAD0
ADC0TIMEbits.SELRES = 3; // ADC0 resolution is 12 bits
ADC1TIMEbits.ADCDIV = 1; // ADC1 clock frequency is half of control clock = TAD1
ADC1TIMEbits.SAMC = 5; // ADC1 sampling time = 5 * TAD1
ADC1TIMEbits.SELRES = 3; // ADC1 resolution is 12 bits
ADC2TIMEbits.ADCDIV = 1; // ADC2 clock frequency is half of control clock = TAD2
ADC2TIMEbits.SAMC = 5; // ADC2 sampling time = 5 * TAD2
ADC2TIMEbits.SELRES = 3; // ADC2 resolution is 12 bits
/* Select analog input for ADC modules, no presync trigger, not sync sampling */
ADCTRGMODEbits.SH0ALT = 0; // ADC0 = AN0
ADCTRGMODEbits.SH1ALT = 0; // ADC1 = AN1
ADCTRGMODEbits.SH2ALT = 0; // ADC2 = AN2
/* Select ADC input mode */
ADCIMCON1bits.SIGN0 = 0; // unsigned data format
ADCIMCON1bits.DIFF0 = 0; // Single ended mode
ADCIMCON1bits.SIGN1 = 0; // unsigned data format
ADCIMCON1bits.DIFF1 = 0; // Single ended mode
ADCIMCON1bits.SIGN2 = 0; // unsigned data format
ADCIMCON1bits.DIFF2 = 0; // Single ended mode
/* Configure ADCGIRQENx */
ADCGIRQEN1 = 0; // No interrupts are used
ADCGIRQEN2 = 0;
/* Configure ADCCSSx */
ADCCSS1 = 0; // No scanning is used
ADCCSS2 = 0;
/* Configure ADCCMPCONx */
ADCCMPCON1 = 0; // No digital comparators are used. Setting the ADCCMPCONx
ADCCMPCON2 = 0; // register to '0' ensures that the comparator is disabled.
ADCCMPCON3 = 0; // Other registers are ?don't care?.
ADCCMPCON4 = 0;
ADCCMPCON5 = 0;
ADCCMPCON6 = 0;
    
    
    /* Configure ADCFLTRx */
ADCFLTR1 = 0; // No oversampling filters are used.
ADCFLTR2 = 0;
ADCFLTR3 = 0;
ADCFLTR4 = 0;
ADCFLTR5 = 0;
ADCFLTR6 = 0;
/* Set up the trigger sources */
ADCTRGSNSbits.LVL0 = 0; // Edge trigger
ADCTRGSNSbits.LVL1 = 0; // Edge trigger
ADCTRGSNSbits.LVL2 = 0; // Edge trigger
ADCTRG1bits.TRGSRC0 = 1; // Set AN0 to trigger from software.
ADCTRG1bits.TRGSRC1 = 1; // Set AN1 to trigger from software.
ADCTRG1bits.TRGSRC2 = 1; // Set AN2 to trigger from software.
/* Early interrupt */
ADCEIEN1 = 0; // No early interrupt
ADCEIEN2 = 0;
/* Turn the ADC on */
ADCCON1bits.ON = 1;
/* Wait for voltage reference to be stable */
while(!ADCCON2bits.BGVRRDY); // Wait until the reference voltage is ready
while(ADCCON2bits.REFFLT); // Wait if there is a fault with the reference voltage
/* Enable clock to analog circuit */
ADCANCONbits.ANEN0 = 1; // Enable the clock to analog bias
ADCANCONbits.ANEN1 = 1; // Enable the clock to analog bias
ADCANCONbits.ANEN2 = 1; // Enable the clock to analog bias
/* Wait for ADC to be ready */
while(!ADCANCONbits.WKRDY0); // Wait until ADC0 is ready
while(!ADCANCONbits.WKRDY1); // Wait until ADC1 is ready
while(!ADCANCONbits.WKRDY2); // Wait until ADC2 is ready
/* Enable the ADC module */
ADCCON3bits.DIGEN0 = 1; // Enable ADC0
ADCCON3bits.DIGEN1 = 1; // Enable ADC1
ADCCON3bits.DIGEN2 = 1; // Enable ADC2

//while (1) {
/* Trigger a conversion */
//ADCCON3bits.GSWTRG = 1;
/* Wait the conversions to complete */
//while (ADCDSTAT1bits.ARDY0 == 0);
/* fetch the result */
//result = ADCDATA0;
//while (ADCDSTAT1bits.ARDY1 == 0);
/* fetch the result */
//result = ADCDATA1;
//while (ADCDSTAT1bits.ARDY2 == 0);
/* fetch the result */
//result[2] = ADCDATA2;
/*
* Process results here
*
* Note: Loop time determines the sampling time since all inputs are Class 1.
* If the loop time is small and the next trigger happens before the completion
* of set sample time, the conversion will happen only after the sample time
* has elapsed.
*
*/
/*
    sprintf(cmd, "%d\r\n", result);
        
        tx_count = 0;
        while (cmd[tx_count] != '\0'){
            if(!DRV_USART_TransmitBufferIsFull(appData.usart_debugger))
            {
                DRV_USART_WriteByte(appData.usart_debugger, cmd[tx_count++]);
            }
        }

        pause_s(1);
*/

}

void ADC_close(void)
{
    ADCCON3bits.DIGEN1 = 0;
}

void saveToFlash(float data, uint8_t float_idx)
{
    
    memcpy((void *)(KVA0_TO_KVA1(appFlashWriteString+4*float_idx)),&data, 4);
    
    appData.flashStates = APP_FLASH_START;
    while (1){
        
    
    
	switch(appData.flashStates)
	{
		default:

		case APP_FLASH_START:
{   
            appData.flashAddress = (uint32_t)appFlashReserveArea;
            appData.flashEndAddress = appData.flashAddress + APP_FLASH_DRV_BYTE_COUNT;
            appData.bufferAddress = ((uint32_t *)appFlashWriteString);
            appData.flashStates = APP_FLASH_ERASE;
			break;

		}	
		case APP_FLASH_ERASE:
        {
            /*
                This state will Erase any and all pages which contain data between the Start Address
                and the End address.  Each erase operation erases one Page of flash.  So, this state 
                will perform more than one operation if required.
                Be aware that even the amount of flash to be used is small, the minimum erasure is still
                one full Page.
            */
            if(!DRV_FLASH_IsBusy(appData.handleFlashDrv))
            {
                if(appData.flashAddress < appData.flashEndAddress)
                {
                    DRV_FLASH_ErasePage(appData.handleFlashDrv
                                        , appData.flashAddress);
                    appData.flashAddress += DRV_FLASH_PAGE_SIZE;
                } else {
                    appData.flashAddress = (uint32_t)appFlashReserveArea;
                    appData.flashStates = APP_FLASH_WRITE;
                }
            }
			break;
		}	
		case APP_FLASH_WRITE:
        {
            /*
                This state will Write the given string one row at a time.  Since the string may take 
                multiple rows to write, it will continue performing write operations until the whole 
                string has been written.  Since the start address may not coinside with the Row start
                address, it pre loads the Row ram buffer with data from the flash.  This also allows
                the last write operation to stop before the end of a Row.
            */
            if(!DRV_FLASH_IsBusy(appData.handleFlashDrv))
            {
                if(appData.flashAddress < appData.flashEndAddress)
                {
                    uint32_t NextRowEndFlash = APP_FLASH_DRV_ROW_END_ADDRESS(appData.flashAddress);
                    uint32_t * NextRowStartFlash = (uint32_t *)APP_FLASH_DRV_ROW_START_ADDRESS(appData.flashAddress);
                    uint32_t LastBufferIndex = DRV_FLASH_ROW_SIZE/sizeof(uint32_t);
                    uint32_t FirstBufferIndex = 0;
                    uint32_t BytesToProgram = DRV_FLASH_ROW_SIZE;
                    // If next flash address is the starting address, then compensate for any row start offset.
                    // The starting address might not be aligned with the start of a row.  This math aligns
                    // the data into the row buffer correctly.
                    if(appData.flashAddress == (uint32_t)appFlashReserveArea)
                    {
                        FirstBufferIndex = APP_ROW_START_OFFSET/sizeof(uint32_t);
                    } 
                    // If the end of the next row is beyond the end of the flash, then
                    // reduce the amount of data copied into the row buffer by the amount
                    // that a full row write would have gone past the end of flash.
                    if(NextRowEndFlash > appData.flashEndAddress)
                    {
                        LastBufferIndex -= (NextRowEndFlash - appData.flashEndAddress)/sizeof(uint32_t);
                    }
                    /*  Initialize row buffer to Flash Contents */
                    memcpy((void *)(appFlashRowBuff), (const void *)(KVA0_TO_KVA1(NextRowStartFlash)), DRV_FLASH_ROW_SIZE);
                    //Copy next rows worth of the string into the row buffer.
                    BytesToProgram = (LastBufferIndex*sizeof(uint32_t) - FirstBufferIndex*sizeof(uint32_t));
                    memcpy((void *)(appFlashRowBuff+FirstBufferIndex), (const void *)(KVA0_TO_KVA1(appData.bufferAddress)), BytesToProgram);

                    DRV_FLASH_WriteRow(appData.handleFlashDrv
                                        , (uint32_t)(KVA0_TO_KVA1(appData.flashAddress)), appFlashRowBuff);
                    appData.flashAddress += BytesToProgram;
                    appData.bufferAddress += BytesToProgram/4;
                } else {
                    appData.flashStates = APP_FLASH_VERIFY;
                }
            }
			break;
        }
		case APP_FLASH_VERIFY:
{
            if(!DRV_FLASH_IsBusy(appData.handleFlashDrv))
            {
                /* Verify that data written to flash memory is valid (FlashReserveArea array read from kseg1) */
                if (!memcmp((const void *)(KVA0_TO_KVA1(appFlashWriteString)), (const void *)(KVA0_TO_KVA1(appFlashReserveArea)), APP_FLASH_DRV_BYTE_COUNT))
                {
                    appData.flashStates = APP_FLASH_DONE;
                }
                else
                {
                    appData.flashStates = APP_FLASH_ERROR;
                }
            }
			break;
        }
		case APP_FLASH_DONE:
		{
			return;
        }
        case APP_FLASH_ERROR:
		{
            appData.error = ERROR_WRITING_TO_FLASH;
            return;
        }
    }
}
}



void myI2S_init(void){

    /* The following code example will initialize the SPI2 Module in I2S Master mode. */
    /* It assumes that none of the SPI1 input pins are shared with an analog input. */
    
    unsigned int rData;
    IEC0CLR=0x03800000; // disable all interrupts
    SPI2CON = 0; // Stops and resets the SPI2.
    SPI2CON2 = 0; // Reset audio settings
    SPI2BRG=0; // Reset Baud rate register
    rData=SPI2BUF; // clears the receive buffer
    //IFS0CLR=0x03800000; // clear any existing event
    //IPC5CLR=0x1f000000; // clear the priority
    //IPC5SET=0x0d000000; // Set IPL = 3, Subpriority 1
    //IEC0SET=0x03800000; // Enable RX, TX and Error interrupts
    SPI2STATCLR=0x40; // clear the Overflow
    SPI2CON2=0x00000080; // I2S Mode, AUDEN = 1, AUDMON = 0
    SPI2BRG =0x0; //(to generate 2.56 MHz BCLK, PBCLK @ 100 MHz)
    SPI2CON =0x00808C60; // Master mode, SPI ON, CKP = 1, 16-bit audio channel
                         // data, 32 bits per frame
    // from here, the device is ready to receive and transmit data
    
    /* Note: A few of bits related to frame settings are not required to be set in the SPI1CON */
    /* register during audio mode operation. Please refer to the notes in the SPIxCON2 register.*/
}