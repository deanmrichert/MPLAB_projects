/* 
 * File:   idle.h
 * Author: Dean Richert
 *
 * Created on October 6, 2017, 1:27 PM
 */

#ifndef PROCESS_H
#define	PROCESS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "app.h"
#include "helper_fcns.h"
    
APP_STATES process(void);

#ifdef	__cplusplus
}
#endif

#endif	/* IDLE_H */