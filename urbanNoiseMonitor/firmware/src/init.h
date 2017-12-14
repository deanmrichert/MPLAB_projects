/* 
 * File:   init.h
 * Author: Dean Richert
 *
 * Created on October 5, 2017, 4:34 PM
 */

#ifndef INIT_H
#define	INIT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "app.h"
#include "helper_fcns.h"
#include "system_config.h"
#include <stdio.h>

APP_STATES init(void);    
    
#ifdef	__cplusplus
}
#endif

#endif	/* INIT_H */

