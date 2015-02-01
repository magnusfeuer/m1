
/*! \file mux.h 
 *  \brief header for mux.c
 *  \author Peter Olofsson
 */

#ifndef __MUX_H_
#define __MUX_H_

#include "types.h"

extern void MUX_Init(void);

void MUX_0_Set(UINT8 n);
void MUX_1_Set(UINT8 n);
UINT8 MUX_0_Get(void);
UINT8 MUX_1_Get(void);

#endif
