/* 
 * File:   helper_fcns.h
 * Author: Dean Richert
 *
 * Created on October 5, 2017, 2:55 PM
 */

#ifndef HELPER_FCNS_H
#define	HELPER_FCNS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "app.h"
#include <string.h>
#include <math.h>

void pause_s(unsigned int);
void sendMsg(DRV_HANDLE, const char*);
void TimerSetup(void);
void TimerCallback(uintptr_t, uint32_t);
bool joined(void);
void updateBiQuad(BiQuad*, float, bool);
void updateFilterBank(float);
void ADC_initialize(void);
void ADC_close(void);
void saveToFlash(float, uint8_t);
void myI2S_init(void);

#ifdef	__cplusplus
}
#endif

#endif	/* HELPER_FCNS_H */

