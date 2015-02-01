/*
 * Chip setup functions
 */
#include "../include/lpc21xx.h"
#include "../include/power.h"
#include "../include/cfg.h"

/*
 *  PowerUp - power up the chip(s)
 */

#define FEED  PLLFEED = 0xAA; PLLFEED = 0x55

void POWER_Up(BOOL wokeUp)
{

    /* Setup the PLL for max speed - 60MHz */
    PLLCFG = MSEL | (1 << PSEL1);
    PLLCON = (1 << PLLE);
    FEED;

    while(!(PLLSTAT & (1<<PLOCK)))
	;
    PLLCON = (1 << PLLE) | (1 << PLLC);
    FEED;

#if HAS_POWERDOWN && WANT_POWERDOWN
    if (!wokeUp)
	PCONP = PCONP_SETTING;  // Start selected perihperals
#endif

    if (wokeUp)
	return;

    /* Init system block */
    MEMMAP = MEMMAP_USER_FLASH_MODE;
    APBDIV = APB_CCLK_4;  /* APB=CCLK/4 = 15MHz (for powerbox/panel) */

    /* Init MAM */
    MAMCR = MAMCR_DISABLED;
    MAMTIM = 4;
    MAMCR = MAMCR_FULLY_ENABLED;
}

