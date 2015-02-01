/*
 * Analog to Digital converter interface
 */
#ifndef _AD_H_
#define _AD_H_

#include "types.h"

extern void   AD_Init(void);

extern UINT16 AD_To_mV(UINT16 sample);
extern void   AD_StartSample(UINT8 n);
extern BOOL   AD_GetSample(UINT16* sample);
extern void   AD_PowerUp(void);
extern void   AD_PowerDown(void);

#endif
