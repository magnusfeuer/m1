/*
 * Timer functionality
 *
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#define TIME_WAIT_1MS()   TIME_WaitNop(7000)
#define TIME_WAIT_100US() TIME_WaitNop(700)
#define TIME_WAIT_10US()  TIME_WaitNop(70)
#define TIME_WAIT_1US()   TIME_WaitNop(1)

// functions
void   TIME_WaitNop(UINT32 n);
void   TIME_Wait(UINT32 us);
BOOL   TIME_IsTimeout(UINT32 timer, UINT32 tmo);
UINT32 TIME_GetTick(void);
void   TIME_Enable(void);
void   TIME_Disable(void);
void   TIME_Create(void); 

#endif
