/*
 * Mux controller functions
 *
 */
#include "../include/types.h"
#include "../include/lpc21xx.h"
#include "../include/mux.h"
#include "../include/gpio.h"
#include "../include/cfg.h"


static UINT8 mux0 = 0;
static UINT8 mux1 = 0;


void MUX_0_Set(UINT8 n)
{
    IO0CLR = (0x7 << 18);
    n &= 0x7;
    IO0SET = (n << 18);
    mux0 = n;
}

void MUX_1_Set(UINT8 n)
{
    IO0CLR = (0x7 << 21);
    n &= 0x7;
    IO0SET = (n << 21);
    mux1 = n;
}

UINT8 MUX_0_Get(void)
{
    return mux0;
}

UINT8 MUX_1_Get(void)
{
    return mux1;
}


void MUX_Init(void)
{
    GPIO_SetP0Function(PIN_AN0_A,0);
    GPIO_SetP0Function(PIN_AN0_B,0);
    GPIO_SetP0Function(PIN_AN0_C,0);

    GPIO_SetPinDir(PIN_AN0_A, GPIO_OUT);
    GPIO_SetPinDir(PIN_AN0_B, GPIO_OUT);
    GPIO_SetPinDir(PIN_AN0_C, GPIO_OUT);

    GPIO_SetP0Function(PIN_AN1_A,0);
    GPIO_SetP0Function(PIN_AN1_B,0);
    GPIO_SetP0Function(PIN_AN1_C,0);

    GPIO_SetPinDir(PIN_AN1_A, GPIO_OUT);
    GPIO_SetPinDir(PIN_AN1_B, GPIO_OUT);
    GPIO_SetPinDir(PIN_AN1_C, GPIO_OUT);

    MUX_0_Set(0);
    MUX_1_Set(0);
}

