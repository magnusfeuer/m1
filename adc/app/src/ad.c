/*
 *  Sample analog values
 */
#include "../include/types.h"
#include "../include/lpc21xx.h"
#include "../include/gpio.h"
#include "../include/ad.h"
#include "../include/cfg.h"

#define ADCR_PDN_MASK             0x00200000
#define AD_CHANNELS               3



/*
 * Initialize the LPC AD converter inputs
 */
void AD_Init(void)
{
    GPIO_SetP0Function(PIN_AIN0, FUN_AIN0);
    GPIO_SetP0Function(PIN_AIN1, FUN_AIN1);
    GPIO_SetP0Function(PIN_AIN2, FUN_AIN2);
    GPIO_SetP0Function(PIN_AIN3, FUN_AIN3);
}

void AD_PowerUp()
{
    /* No powerup needed since PDN is set when ever sample is read */
}

void AD_PowerDown()
{
    ADCR &= ~ADCR_PDN_MASK;
}

/*
 * Convert sample value to mV
 */
UINT16 AD_To_mV(UINT16 sample)
{
    UINT32 v;
    v = (sample * 3300) / 1024;
    return (UINT16) v;
}

/*
 * Start sample a channel 0-3
 */    

void AD_StartSample(UINT8 n)
{
    if (n >= AD_CHANNELS)
	n = 0;
    ADCR = 0x01200400 + (1<<n);  // start now
}

/*
 *  Get sample from ADC unit return FALSE if not ready
 */

BOOL AD_GetSample(UINT16* samp)
{
    UINT32 val = ADDR;

    if ((val & 0x80000000)==0)
	return FALSE;
    *samp = (val >> 6) & 0x03FF;
    return TRUE;
} 
