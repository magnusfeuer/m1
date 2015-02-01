/*
 *
 */
#ifndef _GPIO_H_
#define _GPIO_H_

#include "types.h"

#define GPIO_IN           0
#define GPIO_OUT          1

// PORT 0 bits for IODIR | IOSET | IOCLR
#define P0(x)  (1 << (x))
#define P1(x)  (1 << (x))

/* Define pin number to be used with GPIO functions
 * SetPin, GetPin, SetPinDir
 *   2 bit port number (0,1,2,3)  5 bit pin number=0..31
 */
#define GPIO(port,pin)   ((UINT8)((((port)&0x3)<<5) | ((pin) & 0x1F)))
#define GPIO_PIN(gpin)   ((gpin) & 0x1F)
#define GPIO_PORT(gpin)  (((gpin)>>5) & 0x3)
// functions

void GPIO_SetPin(UINT8  pin, BOOL level);
BOOL GPIO_GetPin(UINT8  pin);
void GPIO_SetPinDir(UINT8 pin, BOOL dir );
void GPIO_SetP0Function(UINT8 byPin,UINT8 func);

#endif

