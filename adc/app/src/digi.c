
#include "../include/types.h"
#include "../include/lpc21xx.h"
#include "../include/gpio.h"
#include "../include/digi.h"
#include "../include/cfg.h"

void timer1_isr(void) IRQ_FUNC;

UINT32 cap_counter[4];

void timer1_isr(void) IRQ_FUNC
{
    UINT32 src = T1IR;
    if (src & 0x10) { 
	cap_counter[0]++;
	// T1CR0 contain the TC value
    }
    if (src & 0x20) {
	cap_counter[1]++;
	// T1CR1 contain the TC value
    }
    if (src & 0x40) {
	cap_counter[2]++;
	// T1CR2 contain the TC value	
    }
    if (src & 0x80) { 
	cap_counter[3]++;
	// T1CR3 contain the TC value
    }
    VICVectAddr = 0x00000000;
}


void DIGI_Init(void)
{
    GPIO_SetP0Function(PIN_DIGI_IN_1, FUN_DIGI_IN_1);
    GPIO_SetP0Function(PIN_DIGI_IN_2, FUN_DIGI_IN_2);
    GPIO_SetP0Function(PIN_DIGI_IN_3, FUN_DIGI_IN_3);

    cap_counter[0] = 0;
    cap_counter[1] = 0;
    cap_counter[2] = 0;
    cap_counter[3] = 0;
    
    T1TCR = 2;    // Stop & Reset timer1

    T1PR = (PCLK / 1000000) - 1;  // Timer tick each us
    T1MCR = 0;                    // No interrupt matching

    // Capture interrupt on 1.0, 1.1, 1.2    
    T1CCR = ((CAP0FE|CAP0I) | (CAP1FE|CAP1I) | (CAP2FE|CAP2I));

    T1TCR = 1;  // Enable timer

    VICVectAddrN(VIC_TIMER1_NUM) = (unsigned long) timer1_isr;
    VICVectCntlN(VIC_TIMER1_NUM) = (0x20 | INT_TIMER1);
}

void DIGI_Enable()
{
    VICIntEnable = (1 << INT_TIMER1);
}

void DIGI_Disable()
{
    VICIntEnClr = (1 << INT_TIMER1);
}

UINT32 DIGI_Read(int i, BOOL reset)
{
    UINT32 v = cap_counter[i];
    if (reset)
	cap_counter[i] -= v;
    return v;
}
