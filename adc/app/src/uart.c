
#include "../include/lpc21xx.h"
#include "../include/cfg.h"
#include "../include/uart.h"


UART_TypeDef* uart[2] = { UART0, UART1 };
UARTPRG_TypeDef* uart_prg[2] = { UARTPRG0, UARTPRG1 };

/*
 *  Calculate the UART clock divisor
 */
static UINT16 UART_BaudDivisor(UINT32 baud)
{
    UINT32 baud16 = baud<<4;
    return (PCLK+baud16-1)/baud16;
}

/*
 * Set baud rate
 *
 */
int UART_SetBaud(int id, UINT32 baud)
{
    UARTPRG_TypeDef* upg = uart_prg[id];
    UINT16 bd;

    bd = UART_BaudDivisor(baud);
    upg->LCR = 0x80;
    upg->DLL = bd & 0xFF;
    upg->DLM = (bd >> 8) & 0xFF;
    upg->FCR = 0x4F;
    upg->LCR = 0x33;
    return 0;
}

void UART_Init(int id, UINT32 baud)
{
    if (id == 0) {
	GPIO_SetP0Function(GPIO(0,0), 1);  // P0.0 Function TxD UART0
	GPIO_SetP0Function(GPIO(0,1), 1); //  P0.1 Function RxD UART0
    }
    else if (id == 1) {
	GPIO_SetP0Function(GPIO(0,8), 1);  // P0.8 Function TxD UART0
	GPIO_SetP0Function(GPIO(0,9), 1); //  P0.9 Function RxD UART0
    }
    UART_SetBaud(id, baud);
}

static BOOL UART_Tx_Ready(UART_TypeDef* up)
{
    return (up->LSR & 0x20) == 0;
}

static BOOL UART_Rx_Ready(UART_TypeDef* up)
{
    return (up->LSR & 0x01) == 0x01;
}

/* Blocking write char */
void UART_WriteChar(int id, UINT8 c)
{
    UART_TypeDef* up = uart[id];

    while(!UART_Tx_Ready(up))
	;
    up->DR = c;
}

/* Blocking write data */
void UART_WriteData(int id, UINT8* buf, UINT32 len)
{
    UART_TypeDef* up = uart[id];

    while(len) {
	while(!UART_Tx_Ready(up))
	    ;
	up->DR = *buf;
	buf++;
	len--;
    }
}

void UART_WriteString(int id, char* str)
{
    UART_TypeDef* up = uart[id];

    while(*str) {
	while(!UART_Tx_Ready(up))
	    ;
	up->DR = (UINT8)*str;
	str++;
    }
}

/*
 * Non blocking read
 */
int UART_PollChar(int id)
{
    UART_TypeDef* up = uart[id];

    if (UART_Rx_Ready(up))
	return up->DR;
    return -1;
}

/*
 * Blocking read char
 */
int UART_ReadChar(int id)
{
    UART_TypeDef* up = uart[id];

    while(!UART_Rx_Ready(up))
	;
    return up->DR;
}

/*
 * Read blocking 
 */
int UART_ReadData(int id, UINT8* buf, UINT32 len)
{
    UART_TypeDef* up = uart[id];
    int res = (int) len;

    while(len) {
	while(!UART_Rx_Ready(up))
	    ;
	*buf = up->DR;
	buf++;
	len--;
    }
    return res;
}
