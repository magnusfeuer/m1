/*
 * gpio.c
 *
 */

#include "../include/lpc21xx.h"
#include "../include/types.h"
#include "../include/gpio.h"


void GPIO_SetPin(UINT8 pin, BOOL level )
{
    UINT32 bit = (1 << GPIO_PIN(pin));
    if (level)
	GPIOn(GPIO_PORT(pin))->IOSET = bit;
    else
	GPIOn(GPIO_PORT(pin))->IOCLR = bit;
}

BOOL GPIO_GetPin(UINT8 pin)
{
    UINT32 bit = (1 << GPIO_PIN(pin));
    return (GPIOn(GPIO_PORT(pin))->IOPIN & bit) != 0;
}

void GPIO_SetPinDir(UINT8 pin, BOOL dir)
{
    UINT32 bit = (1 << GPIO_PIN(pin));
    if (dir)
	GPIOn(GPIO_PORT(pin))->IODIR |= bit;
    else
	GPIOn(GPIO_PORT(pin))->IODIR &= ~bit;
}

void GPIO_SetP0Function(UINT8 pin,UINT8 func)
{
    UINT8 n = GPIO_PIN(pin);
    func &= 0x03;
    if (GPIO_PORT(pin) != 0)
	return;
    if (n<16) {
	// P0.00 -> P0.15 use PINSEL0
	PINSEL0 = PINSEL0 & ~(0x00000003<<(n*2));
	PINSEL0 = PINSEL0 | (((UINT32)func)<<(n*2));
    }
    else {
	// P0.16 -> P0.31 use PINSEL1
	n -= 16;
	PINSEL1 = PINSEL1 & ~(0x00000003<<(n*2));
	PINSEL1 = PINSEL1 | (((UINT8)func)<<(n*2));
    }
}
