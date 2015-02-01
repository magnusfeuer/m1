
#ifndef _UART_H
#define _UART_H

#include "types.h"

extern int UART_SetBaud(int id, UINT32 baud);
extern void UART_Init(int id, UINT32 baud);
extern void UART_WriteChar(int id, UINT8 c);
extern void UART_WriteData(int id, UINT8* buf, UINT32 len);
extern void UART_WriteString(int id, char* str);
extern int UART_PollChar(int id);
extern int UART_ReadChar(int id);
extern int UART_ReadData(int id, UINT8* buf, UINT32 len);


#endif
