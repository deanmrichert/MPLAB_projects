#ifndef _APP_H
#define _APP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"
#include "system_config.h"
#include "driver/driver_common.h"
#include "driver/flash/drv_flash.h"
#include "peripheral/ports/plib_ports.h"

#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif 

#define DEBUG_MODE true
#define TIMER_FREQ 40000
#define RESPONSE_TIMEOUT 10
#define MAX_STR_LEN 1000
#define APPEUI "0004A30B001A820C"
#define APPKEY "38FF213CE7A4BE549896D050B9706111"
#define SAMPLE_T 10 
#define DATA_BUFFER_LEN 100
#define NUM_FILTERS 10
#define FILTER_ORDER 6
#define MEASUREMENT_HISTORY_HORIZON 100000
#define FLASH_MEM_ADDR 0x9D008000 
#define LEQ_PER_BAND_dB 40 
    
typedef enum
{
	APP_STATE_WAIT_FOR_BUTTON_PRESS,
    APP_STATE_INIT,
    APP_STATE_CALIBRATE,
	APP_STATE_JOIN,
    APP_STATE_IDLE,
    APP_STATE_MEASURE,
    APP_STATE_PROCESS,
    APP_STATE_TRANSMIT,
    APP_STATE_ERROR
} APP_STATES;

typedef enum {
    NO_ERROR,
    MEASUREMENT_BUFFER_FULL,
    RX_LEN_EXCEEDED_MAX,
    COULDNT_JOIN_NETWORK,
    TIMEOUT_WHILE_WAITING_FOR_RESPONSE,
    CONVERSION_DIDNT_COMPLETE,
    ERROR_WRITING_TO_FLASH,
    ERROR_ERASING_PAGE,
    I2S_RX_OVRFLW
} Error;

typedef enum {
    FACTORY_RESET,
    SET_ALL_PARAMS_AND_SAVE,
    JOIN_WITH_CURRENT_CONFIG // note: unsaveable parameters reset to factory default
} JoinProcedure;

typedef enum
{
    APP_FLASH_START,	
    APP_FLASH_ERASE,		
    APP_FLASH_WRITE,
    APP_FLASH_VERIFY,
    APP_FLASH_DONE,
    APP_FLASH_ERROR,
} APP_FLASH_STATES;

typedef struct{
    const float b[3]; // numerator coefficients
    const float a[3]; // denominator coefficients
    float y[2]; // filter output ring buffer
    float x[2]; // filter input ring buffer
} BiQuad;

typedef struct{
    BiQuad *sos; 
    float gain;
    float Leq;
} Filter;

typedef struct
{
    APP_STATES state;
    DRV_HANDLE handleFlashDrv;
	APP_FLASH_STATES flashStates;
	uint32_t    flashAddress;
    uint32_t    flashEndAddress;
	uint32_t    * bufferAddress;
    DRV_HANDLE timer_40k;
    DRV_HANDLE usart_debugger;
    DRV_HANDLE usart_RN2903;
    uint8_t rx_buf[MAX_STR_LEN];
    Error error;
    JoinProcedure joinProcedure;
    volatile unsigned int periods_elapsed;
    volatile uint32_t data_buf[DATA_BUFFER_LEN];
    volatile unsigned int data_in_idx;
    unsigned int data_out_idx;
    volatile bool collectData;
    volatile int errorRV;
    float sensor_bias;
    int measurement_history[MEASUREMENT_HISTORY_HORIZON];
    unsigned int measurement_history_idx;
    Filter *filterBank;
} APP_DATA;

void APP_Initialize ( void );
void APP_Tasks( void );

extern APP_DATA appData;

#endif /* _APP_H */

#ifdef __cplusplus
}
#endif