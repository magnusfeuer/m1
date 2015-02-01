#ifndef _CFG_H_
#define _CFG_H_

/*
*   ADC CARD functions
*
*   PIN allocation/usage for ADC board:
*
*   CIRCUIT-PIN    LPC-PIN   FNO  Description
*
*   IC10 - RTL232RL USB-SERIAL
*   --------------------------
*   TXD            P0.0      1    TxD UART0
*   RXD            P0.1      1    RxD UART0
*   RTS            P0.14     -    Set after RESET to program!
*   CTS            P0.3      1?   Signal CTS -> USB
*   DTR            RESET     -    RESET
*
*   IC7 - 74HC4051 MUX-0
*   --------------------------
*   AN0_A          P0.18     0    ANALOG #1-#8
*   AN0_B          P0.19     0
*   AN0_C          P0.20     0
*
*   IC6 - 74HC4051 MUX-1
*   --------------------------
*   AN1_A          P0.21     0    ANALOG #9-#16
*   AN1_B          P0.22     0
*   AN1_C          P0.23     0
*
*   ANALOGUE INPUT
*   --------------------------
*   AN_0           P0.27     1     AIN0 - MUX-1 INPUT
*   AN_1           P0.28     1     AIN1 - MUX-0 INPUT
*   AN_2           P0.29     1     AIN2 - HIGH-SPEED 1
*   AN_3           P0.30     1     AIN3 - HIGH-SPEED 2
*
*   DIGITAL INPUT
*   --------------------------
*
*   IC3 - CAN 1
*   --------------------------
*   TXD                           TD1
*   RXD            P0.25     1    CAN RD1
*
*
*/

#include "gpio.h"

/* ADC operating modes */
#define F_OSC   12000000
#define PLL_MUL 5          /* 60MHz */
#define PLL_DIV 2

#define PLLE    0
#define PLLC    1
#define PLOCK   10

#define VPBDIV_VAL 1

#define PCLK   (F_OSC*PLL_MUL)/VPBDIV_VAL
#define MSEL   (PLL_MUL - 1)
#define PSEL0  5
#define PSEL1  6


#ifdef __KEIL__
  #define IRQ_FUNC_PRE  __irq
  #define IRQ_FUNC      __irq
#else
  #define IRQ_FUNC_PRE  __attribute__ ((interrupt))
  #define IRQ_FUNC      
#endif


//////////////////////////////////////////////////////////////////////////////
//
// CAN interface
//
//////////////////////////////////////////////////////////////////////////////

#define ENABLE_CAN1
// #define ENABLE_CAN2
// #define ENABLE_CAN3
// #define ENABLE_CAN4
#define CANIF 0

#define PIN_CAN1_RX   GPIO(0,25)
#define FUN_CAN1_RX   1
// #define PIN_CAN1_TX (not a pin!)
#define VIC_CAN1_NUM  6

#define PIN_CAN2_RX   GPIO(0,23)
#define FUN_CAN2_RX   1
#define PIN_CAN2_TX   GPIO(0,24)
#define FUN_CAN2_TX   1
#define VIC_CAN2_NUM  5

#define PIN_CAN3_RX   GPIO(0,21)
#define FUN_CAN3_RX   2
#define PIN_CAN3_TX   GPIO(0,22)
#define FUN_CAN3_TX   1
#define VIC_CAN3_NUM  4

#define PIN_CAN4_RX   GPIO(0,12)
#define FUN_CAN4_RX   3
#define PIN_CAN4_TX   GPIO(0,13)
#define FUN_CAN4_TX   3
#define VIC_CAN4_NUM  3

//////////////////////////////////////////////////////////////////////////////
//
// UART interface
//
//////////////////////////////////////////////////////////////////////////////

#define PIN_UART_TXD  GPIO(0,0)   // TxD UART0
#define FUN_UART_TXD  1
#define PIN_UART_RXD  GPIO(0,1)   // RxD UART0
#define FUN_UART_RXD  1
#define PIN_UART_RTS  GPIO(0,14)  // Set after RESET to program
#define PIN_UART_CTS  GPIO(0,3)   // Signal CTS -> USB

//////////////////////////////////////////////////////////////////////////////
//
// MUX interface
//
//////////////////////////////////////////////////////////////////////////////

/* ANALOG #1 - #8 */
#define PIN_AN0_A     GPIO(0,18)
#define PIN_AN0_B     GPIO(0,19)
#define PIN_AN0_C     GPIO(0,20)

/* ANALOG #8 - #16 */
#define PIN_AN1_A     GPIO(0,21)
#define PIN_AN1_B     GPIO(0,22)
#define PIN_AN1_C     GPIO(0,23)

//////////////////////////////////////////////////////////////////////////////
//
// AN interface
//
//////////////////////////////////////////////////////////////////////////////

/* Analog input */
#define PIN_AIN0      GPIO(0,27)   // AIN0 - MUX-1 INPUT
#define FUN_AIN0      1
#define PIN_AIN1      GPIO(0,28)   // AIN1 - MUX-0 INPUT
#define FUN_AIN1      1
#define PIN_AIN2      GPIO(0,29)   // AIN2 - HIGH-SPEED 1
#define FUN_AIN2      1
#define PIN_AIN3      GPIO(0,30)   // AIN3 - HIGH-SPEED 2
#define FUN_AIN3      1

//////////////////////////////////////////////////////////////////////////////
//
// DIGI interface
//
//////////////////////////////////////////////////////////////////////////////

#define PIN_DIGI_IN_1  GPIO(0,10)
#define FUN_DIGI_IN_1  2           // Capture 1.0
#define PIN_DIGI_IN_2  GPIO(0,11)
#define FUN_DIGI_IN_2  2           // Capture 1.1
#define PIN_DIGI_IN_3  GPIO(0,17)
#define FUN_DIGI_IN_3  1           // Capture 1.2

#define VIC_TIMER1_NUM 2

#endif
