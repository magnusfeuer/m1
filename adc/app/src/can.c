/*
 * CAN 1..4 functionality
 */

#include "../include/types.h"
#include "../include/lpc21xx.h"
#include "../include/can.h"
#include "../include/gpio.h"

#include "../include/cfg.h"


// Private functions 
#ifdef ENABLE_CAN1
CAN_Rx gCan1Rx;
CAN_Tx gCan1Tx;
void Can1_IrqHandler(void) IRQ_FUNC_PRE; 
#endif
#ifdef ENABLE_CAN2
CAN_Rx gCan2Rx;
CAN_Tx gCan2Tx;
void Can2_IrqHandler(void) IRQ_FUNC_PRE; 
#endif
#ifdef ENABLE_CAN3
CAN_Rx gCan3Rx;
CAN_Tx gCan3Tx;
void Can3_IrqHandler(void) IRQ_FUNC_PRE; 
#endif
#ifdef ENABLE_CAN4
CAN_Rx gCan4Rx;
CAN_Tx gCan4Tx;
void Can4_IrqHandler(void) IRQ_FUNC_PRE; 
#endif

void Can_RxHandler(CAN_TypeDef* canif, CAN_Rx* rx);

CAN_TypeDef* gCanIf[4] = { CAN1, CAN2, CAN3, CAN4 };

CAN_Rx* gCanRx[4] = {
#ifdef ENABLE_CAN1
    &gCan1Rx,
#else
    (CAN_Rx*) 0,
#endif
#ifdef ENABLE_CAN2
    &gCan2Rx,
#else
    (CAN_Rx*) 0,
#endif
#ifdef ENABLE_CAN3
    &gCan3Rx,
#else
    (CAN_Rx*) 0,
#endif
#ifdef ENABLE_CAN4
    &gCan4Rx
#else
    (CAN_Rx*) 0
#endif
};

CAN_Tx* gCanTx[4] = {
#ifdef ENABLE_CAN1
    &gCan1Tx,
#else
    (CAN_Tx*) 0,
#endif
#ifdef ENABLE_CAN2
    &gCan2Tx,
#else
    (CAN_Tx*) 0,
#endif
#ifdef ENABLE_CAN3
    &gCan3Tx,
#else
    (CAN_Tx*) 0,
#endif
#ifdef ENABLE_CAN4
    &gCan4Tx
#else
    (CAN_Tx*) 0
#endif
};
    
//=function===================================
///
//============================================

void Can_RxHandler(CAN_TypeDef* canif, CAN_Rx* rx)
{
    rx->mActivity++;
    if (canif->GSR & 0x01) { // receive buffer available?
	UINT8 ix = rx->mWrIndex;
	if (rx->mMask & (1<< ix)) { // occupied?
	    rx->mOverflow++; // mark overflow
	    canif->CMR = 0x04;       // RRB - Release receive buffer
	}
	else {
	    CAN_Message* mesg = &rx->mBuff[ix];
	    mesg->mId   = canif->RID;    
	    mesg->mLen  = ((canif->RFS) & 0xC0000000) |
		((canif->RFS >> 16) & 0x0F);
	    mesg->u.wData[0] = canif->RDA;
	    mesg->u.wData[1] = canif->RDB;
	    canif->CMR = 0x04;       // RRB - Release receive buffer
	    rx->mMask |= (1<<ix); // Mark buffer pos as occupied
	    rx->mWrIndex = (ix+1) & 0x0F;
	}
    }
}

//
// Interrupt wrapper functions
//
#ifdef ENABLE_CAN1
void Can1_IrqHandler(void) IRQ_FUNC 
{
    Can_RxHandler(gCanIf[0], gCanRx[0]);
    VICVectAddr = 0;
}
#endif

#ifdef ENABLE_CAN2
void Can2_IrqHandler(void) IRQ_FUNC 
{
    Can_RxHandler(gCanIf[1], gCanRx[1]);
    VICVectAddr = 0;
}
#endif

#ifdef ENABLE_CAN3
void Can3_IrqHandler(void) IRQ_FUNC 
{
    Can_RxHandler(gCanIf[2], gCanRx[2]);
    VICVectAddr = 0;
}
#endif

#ifdef ENABLE_CAN4
void Can4_IrqHandler(void) IRQ_FUNC 
{
    Can_RxHandler(gCanIf[3], gCanRx[3]);
    VICVectAddr = 0;
}
#endif

#define SAM 0
#define SJW 3
#define BTR(sam,sjw,tseg2,tseg1,brp) \
    (((sam) << 23) | ((sjw) << 14) | ((tseg2) << 20) | ((tseg1) << 16) | (brp))

void CAN_SetBitRate(int canid, UINT32 bitrate)
{
    CAN_TypeDef* canif = gCanIf[canid];
    UINT32 brp;
    UINT32 btr;

    if (bitrate > 500000) {
	// TSEG1 = 8, TSEG2 = 4 
	brp = ((PCLK / bitrate) / (8+4+3)) - 1;
	btr = BTR(SAM,SJW,4,8,brp);
    }
    else {
	// TSEG1 = 5, TSEG2 = 2
	brp = ((PCLK / bitrate) / (5+2+3))-1;
	btr = BTR(SAM,SJW,2,5,brp);
    }
/*** for reference
    else {
	// TSEG1 = 6, TSEG2 = 6
	brp = ((PCLK / bitrate) / (6+6+3))-1;
	btr = BTR(SAM,SJW,2,5,brp);
    }
***/
    // Setup can controller
    canif->MOD = 0x05;  // Disable CAN controller
    canif->BTR = btr;
    canif->MOD = 0x04;  // Enable CAN controller
}

//
// Initialize the CAN_Rx data structure
//
void CAN_Setup(int canid)
{
    CAN_Rx* rx      = gCanRx[canid];
    CAN_Tx* tx      = gCanTx[canid];
    CAN_TypeDef* canif = gCanIf[canid];

    if (rx) {
	// Setup Rx data structure
	rx->mActivity = 0;
	rx->mWrIndex = 0;
	rx->mOverflow  = 0;
	rx->mMask    = 0;
	rx->mRdIndex = 0;
    }
    if (tx) {
	// Setup Tx data structure
	tx->mActivity = 0;
	tx->mWrIndex = 0;
	tx->mOverflow  = 0;
	tx->mMask    = 0;
	tx->mRdIndex = 0;
    }

    if (canif) {
	// Setup can controller
	canif->MOD = 0x05;  // Disable CAN controller
	canif->GSR = 0x00;
	canif->MOD = 0x05;  // Disable CAN controller
	// LOM = 0  => No silent mode.
	// STM = 1  => Successful tx requires acknowledge.
	// TPM = 0  => Priority depends on id.
	// SM  = 0  => No sleep mode.
	// RPM = 0  => Low is dominant.
	// TM  = 0  => No test mode.
	canif->BTR = 0x00660003;   // 250 kb/s when PCLK = 15Mhz
	// canif->MOD |= 0x04;  // + STM=1 (self-test-mode!?) 
	canif->IER = 0x01;
	canif->MOD = 0x04;  // Enable CAN controller
    }
}

void CAN_Enable(int canid)
{
    CAN_TypeDef* canif = gCanIf[canid];

    canif->MOD |= 0x01;  // Disable CAN controller

    switch(canid) {
    case 0: VICIntEnable = (1<<INT_CAN1_RX); break;
    case 1: VICIntEnable = (1<<INT_CAN2_RX); break;
    case 2: VICIntEnable = (1<<INT_CAN3_RX); break;
    case 3: VICIntEnable = (1<<INT_CAN4_RX); break;
    }
    canif->IER |= 0x01;  // Enable rx interrupt

    canif->MOD &= ~0x01;  // Enable CAN controller
    canif->IER |= 0x01;  // Enable rx interrupt
}

void CAN_Disable(int canid)
{
    CAN_TypeDef* canif = gCanIf[canid];

    canif->MOD |= 0x01;  // Disable CAN controller

    canif->IER |= ~0x01;  // Disable rx interrupt
    switch(canid) {
    case 0: VICIntEnClr = (1<<INT_CAN1_RX); break;
    case 1: VICIntEnClr = (1<<INT_CAN2_RX); break;
    case 2: VICIntEnClr = (1<<INT_CAN3_RX); break;
    case 3: VICIntEnClr = (1<<INT_CAN4_RX); break;
    }
    canif->MOD &= ~0x01;  // Enable CAN controller
}

//
// Send message on CAN
// 
//

void CAN_Send(int canid, CAN_Message* mesg)
{
    CAN_TypeDef* canif = gCanIf[canid];
    CAN_Tx* tx = gCanTx[canid];

    if (!canif)
	return;
    canif->TFI1 = (0xC0000000 & mesg->mLen) | ((mesg->mLen & 0xF)<<16);
    canif->TID1 = mesg->mId;
    canif->TDA1 = mesg->u.wData[0];
    canif->TDB1 = mesg->u.wData[1];
    canif->CMR  = 0x21;  // Transmit Request | STB1-Select Tx Buffer 1
    /* While TCS=0, ES=0, BS=0 */
    while ( (canif->GSR & 0xC4) == 0x00)
	;
    // Check if there was a transmission error... ? 
    // Simulate Tx buffer use by update the Wr
    tx->mWrIndex = (tx->mWrIndex+1) & 0xF;
    tx->mActivity++;
}

//
// Recive message from CAN Rx buffer
//

BOOL CAN_Recv(int canid, CAN_Message* mesg)
{
    CAN_TypeDef* canif = gCanIf[canid];
    CAN_Rx* rx = gCanRx[canid];
    UINT8 ix;

    if (!canif)
	return FALSE;
    if (!rx)
	return FALSE;

    if (rx->mMask == 0)
	return FALSE; // No messages in buffer

    ix = rx->mRdIndex;
    *mesg = rx->mBuff[ix];
    rx->mMask &= ~(1 << ix);   // mark as free
    rx->mRdIndex = (ix + 1) & 0xF;
    return TRUE;
}

/*
 * Return flags matching SJA1000 CAN Controller register description:
 * Bit 0 CAN receive FIFO queue full 
 * Bit 1 CAN transmit FIFO queue full 
 * Bit 2 Error warning (EI), see SJA1000 datasheet 
 * Bit 3 Data Overrun (DOI), see SJA1000 datasheet 
 * Bit 4 Not used. 
 * Bit 5 Error Passive (EPI), see SJA1000 datasheet 
 * Bit 6 Arbitration Lost (ALI), see SJA1000 datasheet * 
 * Bit 7 Bus Error (BEI), see SJA1000 datasheet ** 
 */

UINT8 CAN_Read_Status(int canid)
{
    CAN_TypeDef* canif = gCanIf[canid];
    UINT8 status = 0;
    UINT32 gsr = canif->GSR;
    
    if (gsr & (1<<0))     // RBS - receive buffer full
	status  |= (1<<0);
    if (!(gsr & (1<<2)))  // TBS - transmit buffer status
	status |= (1<<1);
    if (gsr & (1<<6))     // ES - error status
	status |= (1<<2);
    if (gsr & (1<<1))     // DOS - data overrun status
	status |= (1<<3);
    // error-passiv    5
    // arbitration-lost 6
    if (gsr & (1<<7))     // BS - bus status
	status |= (1<<7);
    return status;
}

void CAN_Init(int canid)
{
    switch(canid) {
    case 0:
#ifdef ENABLE_CAN1
	GPIO_SetP0Function(PIN_CAN1_RX, FUN_CAN1_RX);
	CAN_Setup(0);
	VICVectAddrN(VIC_CAN1_NUM) = (unsigned long)Can1_IrqHandler;
	VICVectCntlN(VIC_CAN1_NUM) = (0x20 | INT_CAN1_RX);
#endif
	break;
    case 1:
#ifdef ENABLE_CAN2
	GPIO_SetP0Function(PIN_CAN2_RX, FUN_CAN2_RX);
	GPIO_SetP0Function(PIN_CAN2_TX, FUN_CAN2_TX);
	CAN_Setup(1);
	VICVectAddrN(VIC_CAN2_NUM) = (unsigned long)Can2_IrqHandler;
	VICVectCntlN(VIC_CAN2_NUM) = (0x20 | INT_CAN2_RX);
#endif
	break;
    case 2:
#ifdef ENABLE_CAN3
	GPIO_SetP0Function(PIN_CAN3_RX, FUN_CAN3_RX);
	GPIO_SetP0Function(PIN_CAN3_TX, FUN_CAN3_TX);
	CAN_Setup(2);
	VICVectAddrN(VIC_CAN3_NUM) = (unsigned long)Can3_IrqHandler;
	VICVectCntlN(VIC_CAN3_NUM) = (0x20 | INT_CAN3_RX);
#endif
	break;
    case 3:
#ifdef ENABLE_CAN4
	GPIO_SetP0Function(PIN_CAN4_RX, FUN_CAN4_RX);
	GPIO_SetP0Function(PIN_CAN4_TX, FUN_CAN4_TX);
	CAN_Setup(3);
	VICVectAddrN(VIC_CAN4_NUM) = (unsigned long)Can4_IrqHandler;
	VICVectCntlN(VIC_CAN4_NUM) = (0x20 | INT_CAN4_RX);
#endif
	break;
    }
    AFMR = 3;  // Disable the Acceptance filters
}
