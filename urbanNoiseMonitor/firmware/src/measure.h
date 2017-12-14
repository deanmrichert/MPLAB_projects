/* 
 * File:   measure.h
 * Author: Dean Richert
 *
 * Created on October 6, 2017, 1:43 PM
 */

#ifndef MEASURE_H
#define	MEASURE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "app.h"
#include "driver/i2s/drv_i2s.h"
#include "helper_fcns.h"
#include <stdio.h>

APP_STATES measure(void);

#ifdef	__cplusplus
}
#endif

#endif	/* MEASURE_H */

