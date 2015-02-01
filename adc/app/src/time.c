
#include "../include/cfg.h"
#include "../include/types.h"
#include "../include/lpc21xx.h"
#include "../include/time.h"

void TIME_WaitNop(UINT32 n)
{
    while (n) {
	asm("nop");
	n--;
    }
}

/*
 * Wait n micro seconds
 */
void TIME_Wait(UINT32 us)
{
    UINT32 t = T0TC + us;
    while (!((t-T0TC)&0x80000000));
}

/*
 * Check timeout in microseconds
 */
BOOL TIME_IsTimeout(UINT32 timer,UINT32 tmo)
{
    UINT32 t = T0TC;
    timer += tmo;
    return ((timer-t)&0x80000000)?TRUE:FALSE;
}

/*
 * Get tick in us
 */

UINT32 TIME_GetTick(void)
{
    return T0TC;
}

void TIME_Enable(void)
{
    T0TCR = 1;
}

void TIME_Disable(void)
{
    T0TCR = 0;
}


/*
 *   Timer T0TC will wrap after 4294 seconds (71 minutes)
 *   So no timer should ever be set to value greater than 
 */
void TIME_Create(void) 
{
    T0PR = PCLK / (1000000) - 1;  // tick one every us
    T0TC = 0;                     // reset
    T0TCR = 1;                    // enable
}
