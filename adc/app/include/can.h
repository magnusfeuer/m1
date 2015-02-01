/*
 *  Can interface
 */
#ifndef _CANDRV_H_
#define _CANDRV_H_

#include "lpc21xx.h"
#include "types.h"

// Bits used in mLen field to signal RTR frame or 29 bit frame id.
#define CAN_MESSAGE_EXT 0x80000000
#define CAN_MESSAGE_RTR 0x40000000

typedef struct {
    UINT32 mId;       // Can ID, 11 or 29 bits.
    UINT32 mLen;      // Used number of bytes in mData (0..8) + RTR + EXT
    union {
	UINT32 wData[2];   
	UINT8  mData[8];
    } u;
} CAN_Message;

#define CAN_RX_BUFF_SIZE   16  // Do not change this (16 bit mask used!)
#define CAN_TX_BUFF_SIZE   16  // Do not change this (16 bit mask used!)

/* CAN interrupt receive buffer */
typedef struct _CAN_Rx {
    UINT16      mActivity;
    UINT8       mRdIndex;   // Extract point (user)
    UINT8       pad1;
    UINT8       mWrIndex;   // Insert point  (interrupt)
    UINT8       mOverflow;  // Overflow counter
    UINT16      mMask;      // Free mask
    CAN_Message mBuff[CAN_RX_BUFF_SIZE];
} CAN_Rx;

/* CAN interrupt receive buffer */
typedef struct _CAN_Tx {
    UINT16      mActivity;
    UINT8       pad1;
    UINT8       mRdIndex;   // Extract point (interrupt)
    UINT8       mWrIndex;   // Insert point  (user)
    UINT8       mOverflow;  // Overflow counter
    UINT16      mMask;      // Free mask
    CAN_Message mBuff[CAN_TX_BUFF_SIZE];
} CAN_Tx;

extern CAN_TypeDef* gCanIf[4];
extern CAN_Rx* gCanRx[4];
extern CAN_Tx* gCanTx[4];

extern BOOL CAN_Recv(int canid, CAN_Message* mesg);
extern void CAN_Send(int canid, CAN_Message* mesg);
extern void CAN_Disable(int canid);
extern void CAN_Enable(int canid);
extern void CAN_Setup(int canid);
extern void CAN_SetBitRate(int canid, UINT32 bitrate);
extern UINT8 CAN_Read_Status(int canid);

extern void CAN_Init(int canid);

#endif
