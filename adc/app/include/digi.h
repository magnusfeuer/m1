
#ifndef _DIGI_H
#define _DIGI_H

#include "types.h"

extern void DIGI_Init(void);

extern void DIGI_Enable(void);
extern void DIGI_Disable(void);
extern UINT32 DIGI_Read(int i, BOOL reset);


#endif
