/*
 * Magden ADC main functions
 */
#include "../include/mux.h"
#include "../include/ad.h"
#include "../include/digi.h"
#include "../include/uart.h"
#include "../include/can.h"
#include "../include/time.h"
#include "../include/cfg.h"

#define XON  17
#define XOFF 19
#define BELL 7
#define CR   13   // Put 10 here to get debug output

#define OK     0
#define ERROR -1
#define IGNORE 1

#define UART_ID  0
#define CAN_ID   0

#define CBUF_SIZE 80
static UINT8 cbuf[CBUF_SIZE];
static UINT8 cbuf_pos;
static UINT8 cbuf_length;

#define ADC_NUM_CHANNELS 18

#define ADC_FLAG_ACTIVE 0x80
#define ADC_FLAG_READ   0x40
#define ADC_FLAG_ASYNC  0x01
#define ADC_FLAG_CHANGE 0x02
#define ADC_FLAG_ONCE   0x04
#define ADC_MASK_MODE   0xF0

typedef struct {
    UINT32  flags;     // Mode flags
    UINT16  v;         // Current value
    UINT16  last_v;    // Last value reported
    UINT32  tv;        // Last Sample time
    UINT32  tr;        // Last Report time
    UINT32  interval;  // Report interval
    UINT32  timer;     // Report timer
    UINT32  nr;        // Number of Reports (samples)
} ADC_ChannelType;


#define DIG_NUM_CHANNELS 3

#define DIG_FLAG_ACTIVE 0x80
#define DIG_FLAG_READ   0x40
#define DIG_FLAG_ASYNC  0x01

typedef struct {
    UINT32 flags;      // Mode flags
    UINT32 counter;    // Digital counter value
    UINT32 tr;         // Last Report time
    UINT32 interval;   // Report interval
    UINT32 timer;      // Report timer
    UINT32  nr;        // Number of Reports (samples)
} DIG_ChannelType;

#define CAN_FLAG_OPEN    0x01
#define CAN_FLAG_TSTAMP  0x02

UINT32 can_flags;
UINT32 can_bitrate;

ADC_ChannelType adc[ADC_NUM_CHANNELS];
DIG_ChannelType dig[DIG_NUM_CHANNELS];

UINT32 adc_chan_mask;
UINT32 dig_chan_mask;

BOOL output_enable;
UINT32 main_loops;
UINT32 main_loops_0;
UINT32 main_loops_t;


#ifdef ENABLE_CAN1
#define PCONP_CAN1 (1 << PCCAN1)
#else
#define PCONP_CAN1 0
#endif

#ifdef ENABLE_CAN2
#define PCONP_CAN2 (1 << PCCAN2)
#else
#define PCONP_CAN2 0
#endif

#ifdef ENABLE_CAN3
#define PCONP_CAN3 (1 << PCCAN3)
#else
#define PCONP_CAN3 0
#endif

#ifdef ENABLE_CAN4
#define PCONP_CAN4 (1 << PCCAN4)
#else
#define PCONP_CAN4 0
#endif

/* Define all peripherals used by adc */
#define PCONP_SETTING  \
    ((1 << PCTIM0) |			\
     (1 << PCTIM1) |			\
     (1 << PCPWM0) |			\
     (1 << PCI2C)  |			\
     (1 << PCAD)   |			\
     (PCONP_CAN1 | PCONP_CAN2 | PCONP_CAN3 | PCONP_CAN4))


/* declarations */
void CBUF_Init(void);
void CBUF_AppendChar(char c);
int CBUF_Char(void);
BOOL CBUF_Empty(void);
int CBUF_XDigit(void);
int CBUF_Digit(void);


int PARSE_XDigits(int n, UINT32* dr0, UINT32* dr1);
int PARSE_DHex(int n, UINT32* dr0, UINT32* dr1);
int PARSE_Hex(int n, UINT32* dr);
int PARSE_Timer4(UINT32* dr);
int PARSE_ADC_Channel(UINT32* chanp);
int PARSE_DIG_Channel(UINT32* chanp);
int PARSE_XBuffer(int n, UINT8* buf);
int PARSE_Number(int n, UINT32* dr);


char* UnsignedToString(UINT32 n, int base, char* buf, int len, int num);
char* BufferToHex(UINT8* ibuf, int ilen, char* obuf, int olen);


UINT16 EMIT_sx(int i);
void EMIT_si(int i);
void REPORT_si(int i);
void REPORT_loop(void);
void ADC_Report(int i, char c);
void DIG_Report(int i, char c);
void CAN_Report(CAN_Message* msg);


void APP_CAN_Init(void);
int COMMAND_CAN_SetBitRate(void);
int COMMAND_CAN_Open(void);
int COMMAND_CAN_Close(void);
int COMMAND_CAN_Transmit(BOOL ext);
int COMMAND_CAN_Status(void);
int COMMAND_CAN_ACR(void);
int COMMAND_CAN_AMR(void);
int COMMAND_CAN_Version(void);
int COMMAND_CAN_SerialNumber(void);
int COMMAND_CAN_TimeStamp(void);


void APP_DIG_Init(void);
int COMMAND_DIG_Enable(void);
int COMMAND_DIG_AsyncMode(void);
int COMMAND_DIG_Read(void);

void APP_ADC_Init(void);

int COMMAND_ADC_Enable(void);
int COMMAND_ADC_AsyncMode(void);
int COMMAND_ADC_ChangeMode(void);
int COMMAND_ADC_Read(void);
int COMMAND_ADC_si(void);

int COMMAND_UART_Baud(void);
int COMMAND_APP_Info(void);

void ADC_Poll(int i);
void DIG_Poll(int i);
void AD_Poll(void);

void DI_Poll(void);

int  APP_Dispatch(void);
void APP_Input(char c);
void APP_Init(void);

void POWER_Up(BOOL wokeUp);



void CBUF_Init(void)
{
    cbuf_pos = 0;
    cbuf_length = 0;
}

/*
 * Insert a character into command buffer
 */
void CBUF_AppendChar(char c)
{
    if (cbuf_length < CBUF_SIZE) {
	cbuf[cbuf_length] = c;
	cbuf_length++;
    }
}

/*
 * Read one character form command buffer
 * return -1 on error (no char)
 */
int CBUF_Char(void)
{
    if (cbuf_pos >= cbuf_length)
	return -1;
    return cbuf[cbuf_pos++];
}

BOOL CBUF_Empty(void)
{
    return (cbuf_pos >= cbuf_length);
}

/* Return next hex digit or -1 if not avail */
int CBUF_XDigit(void)
{
    if (cbuf_pos < cbuf_length) {
	int c = cbuf[cbuf_pos];
	if ((c >= '0') && (c <= '9'))
	    c = (c -'0');
	else if ((c >= 'A') && (c <= 'F'))  
	    c = (c-'A')+10;
	else if ((c >= 'a') && (c <= 'f'))
	    c = (c-'a')+10;
	else
	    return -1;
	cbuf_pos++;
	return c;
    }
    return -1;
}

/* Return next decimal digit or -1 if not avail */
int CBUF_Digit(void)
{
    if (cbuf_pos < cbuf_length) {
	int c = cbuf[cbuf_pos];
	if ((c >= '0') && (c <= '9'))
	    c = (c-'0');
	else
	    return -1;
	cbuf_pos++;
	return c;
    }
    return -1;
}


/*
 * Read at most n hex digits from input (n0 >= 0, n0 <= 16)
 * Store hex nybble in buf from high to low
 * return number of chars NOT processed 
 */
int PARSE_XBuffer(int n, UINT8* buf)
{
    while (n) {
	int d;
	UINT8 di;

	if ((d = CBUF_XDigit()) < 0)
	    break;
	di = (d << 4);

	if ((d = CBUF_XDigit()) < 0) {
	    *buf = di;
	    return n-1;
	}
	di |= d;
	*buf++ = di;
	n -= 2;
    }
    return n;
}
	
/*
 * Read at most n0 hex digits from input (n0 >= 0, n0 <= 16)
 * return the value in dr0 (low) and dr1 (high)
 * return number of chars NOT processed 
 */
int PARSE_XDigits(int n, UINT32* dr0, UINT32* dr1)
{
    UINT32 d0;
    UINT32 d1;
    int d;
    int i;

    i = 0;
    d0 = 0;
    d1 = 0;
    while(n && (i<8)) {
	if ((d = CBUF_XDigit()) < 0) goto done;
	d0 = (d0 << 4) | d;
	n--;
	i++;
    }
    while(n && (i<16)) {
	if ((d = CBUF_XDigit()) < 0) goto done;
	d1 = (d1 << 4) | d;
	n--;
	i++;
    }

done:
    if (dr0) *dr0 = d0;
    if (dr1) *dr1 = d1;
    return n;
}

/*
 * Read at most n digits from input (n >= 0, n <= 9)
 * return the value dr
 * return number of chars NOT processed 
 */
int PARSE_Number(int n, UINT32* dr)
{
    UINT32 d0;
    int d;
    int i;

    i = 0;
    d0 = 0;

    while(n) {
	if ((d = CBUF_Digit()) < 0) goto done;
	d0 = d0*10 + d;
	n--;
    }
done:
    if (dr)
	*dr = d0;
    return n;
}

/*
 * Read exacly n hex characters and return in dr0 and dr1
 */

int PARSE_DHex(int n, UINT32* dr0, UINT32* dr1)
{
    n = PARSE_XDigits(n, dr0, dr1);
    if (n != 0)
	return ERROR;
    return n;
}

/*
 * Read exactly 0-4 digits
 */
int PARSE_Hex(int n, UINT32* dr)
{
    return PARSE_XDigits(n, dr, (UINT32*) 0);
}

int PARSE_Timer4(UINT32* dr)
{
    switch(PARSE_XDigits(4, dr, (UINT32*) 0)) {
    case 4:
	*dr = 0x03E8;   // 1000 - 1s
	return OK;
    case -1:
	return ERROR;
    default:
	if (*dr > 0xEA60)  // more than 60 s
	    return ERROR;
	return OK;
    }
}

/*
 * Get the adc channel number [0-11]
 * 0 - 17  decimal
 * 0       mux 1 - ext_12 - [ANALOG #1]
 * 1 - 7   mux 1 - an0    - [ANALOG #2-ANALOG #8]  - [J2-1 - J2-7]
 * 8 - 15  mux 0 - an1    - [ANALOG #9-ANALOG #16] - [J2-8 - J2-15]
 * 16      an2
 * 17      an3
 */
int PARSE_ADC_Channel(UINT32* chanp)
{
    if (PARSE_Hex(2, chanp) < 0)
	return ERROR;
    if (*chanp >= ADC_NUM_CHANNELS)
	return ERROR;
    return OK;
}

/*
 * Get the digital channel number [00-02]
 */
int PARSE_DIG_Channel(UINT32* chanp)
{
    if (PARSE_Hex(2, chanp) < 0)
	return ERROR;
    if (*chanp >= DIG_NUM_CHANNELS)
	return ERROR;
    return OK;
}

/*
 * Format unsigned number in any base
 */
char* UnsignedToString(UINT32 n, int base, char* buf, int len, int num)
{
    int i = len-1;
    int j = num;

    if (i < 0)
	return (char*) 0;
    buf[i--] = '\0';
    if ((n == 0) && (num == 0)) {
	buf[i--] = '0';
    }
    else {
	while((i >= 0) && ((j>0) || (n && (num==0)))) {
	    int d = n % base;
	    buf[i--] = (d < 10) ? (d+'0') : ((d-10)+'A');
	    n = n / base;
	    j--;
	}
    }
    return &buf[i+1];
}


char* BufferToHex(UINT8* ibuf, int ilen, char* obuf, int olen)
{
    char* ptr = obuf;

    while((ilen >= 0) && (olen > 1)) {
	int d;

	d = *ibuf >> 4;
	*obuf++ = (d < 10) ? (d+'0') : ((d-10)+'A');
	olen--;

	if (olen > 1) {
	    d = (*ibuf++) & 0xf;
	    *obuf++ = (d < 10) ? (d+'0') : ((d-10)+'A');
	    olen--;
	}
	ilen--;
    }
    if (olen >= 1)
	*obuf = '\0';
    return ptr;
}


UINT16 EMIT_sx(int i)
{
    UINT8 high = (adc[i].v >> 8) & 0xff;
    UINT8 low  = adc[i].v & 0xff;
    
    UART_WriteChar(UART_ID, high);
    UART_WriteChar(UART_ID, low);
    return high + low;
}

void EMIT_si(int i)
{
    UINT16 sum = EMIT_sx(i);
    UART_WriteChar(UART_ID, sum & 0xff);
}

void REPORT_si(int i)
{
    char ibuf[10];
    char* ptr;
    ptr = UnsignedToString(adc[i].v*2,10, ibuf, sizeof(ibuf), 0);
    
    UART_WriteString(UART_ID, "Channel N");
    UART_WriteChar(UART_ID, i+'1');
    UART_WriteString(UART_ID, " : ");
    UART_WriteString(UART_ID, ptr);
    UART_WriteString(UART_ID, "\r\n");
}

/* Report main loop frequency */
void REPORT_loop(void)
{
    char ibuf[10];
    char* ptr;
    UINT32 loops = main_loops - main_loops_0;
    UINT32 t     = TIME_GetTick();
    UINT32 ms    = (t - main_loops_t) / 1000;
    
    ptr = UnsignedToString(1000*(loops/ms),10, ibuf, sizeof(ibuf), 0);
    UART_WriteString(UART_ID, ptr);
    UART_WriteString(UART_ID, "Hz");
    UART_WriteString(UART_ID, "\r");

    main_loops_0 = main_loops;
    main_loops_t = t;
}

void ADC_Report(int i, char c)
{
    char ibuf[10];
    char* ptr;
    UINT32 t;

    if (!output_enable)
	return;
    t = TIME_GetTick();
    UART_WriteChar(UART_ID, c);
    ptr = UnsignedToString((UINT32)i, 16, ibuf, sizeof(ibuf), 2);
    UART_WriteString(UART_ID, ptr);
    ptr = UnsignedToString((adc[i].v & 0x3FF)<<6, 16, ibuf, sizeof(ibuf), 4);
    UART_WriteString(UART_ID, ptr);
    ptr = UnsignedToString((t-adc[i].tr)/1000, 16, ibuf, sizeof(ibuf), 4);
    UART_WriteString(UART_ID, ptr);
    adc[i].tr = t;
    UART_WriteChar(UART_ID, CR);
    adc[i].nr++;
}

void DIG_Report(int i, char c)
{
    char ibuf[10];
    char* ptr;
    UINT32 t;

    if (!output_enable)
	return;
    t = TIME_GetTick();
    UART_WriteChar(UART_ID, c);
    ptr = UnsignedToString((UINT32)i, 16, ibuf, sizeof(ibuf), 2);
    UART_WriteString(UART_ID, ptr);
    ptr = UnsignedToString(dig[i].counter, 16, ibuf, sizeof(ibuf), 4);
    UART_WriteString(UART_ID, ptr);
    ptr = UnsignedToString((t - dig[i].tr)/1000, 16, ibuf, sizeof(ibuf), 4);
    UART_WriteString(UART_ID, ptr);
    dig[i].tr = t;
    UART_WriteChar(UART_ID, CR);
    dig[i].nr++;
}

/*
 * Report can message
 * 'T' <h>*8 <l> <x>*16 [ <x>*4 ]
 * 't' <h>*3 <l> <x>*16 [ <x>*4 ]
 */
void CAN_Report(CAN_Message* msg)
{
    char ibuf[10];
    char xbuf[17];
    char* ptr;

    if (msg->mLen & CAN_MESSAGE_EXT) {
	UART_WriteChar(UART_ID, 'T');
	ptr = UnsignedToString(msg->mId,16,ibuf,sizeof(ibuf),8);
    }
    else {
	UART_WriteChar(UART_ID, 't');
	ptr = UnsignedToString(msg->mId,16,ibuf,sizeof(ibuf),3);
    }
    UART_WriteString(UART_ID, ptr);
    UART_WriteChar(UART_ID, (msg->mLen & 0xF)+'0');
    // FORMAT data
    ptr = BufferToHex(msg->u.mData,(msg->mLen&0xF),xbuf,sizeof(xbuf));
    UART_WriteString(UART_ID, ptr);    
    // FOrmat timestamp
    if (can_flags & CAN_FLAG_TSTAMP) {
	UINT32 t;
	t = TIME_GetTick();  // Get timestamp
	t /= 1000;           // Make into milliseconds
	t %= 60000;          // Wrap at 60000 millicseconds
	ptr = UnsignedToString(t, 16, ibuf, sizeof(ibuf), 4);
	UART_WriteString(UART_ID, ptr);
    }
    UART_WriteChar(UART_ID, CR);
}


void APP_CAN_Init()
{
    can_flags = 0;
    can_bitrate = 0;
}


/*
 * Sn[CR] Setup with standard CAN bit-rates where n is 0-8. 
 * This command is only active if the CAN channel is closed.
 *
 * S0 Setup 10Kbit
 * S1 Setup 20Kbit
 * S2 Setup 50Kbit
 * S3 Setup 100Kbit
 * S4 Setup 125Kbit
 * S5 Setup 250Kbit
 * S6 Setup 500Kbit 
 * S7 Setup 800Kbit 
 * S8 Setup 1Mbit 
 * Example: S4[CR] 
 * Setup CAN to 125Kbit. 
 * Returns: CR (Ascii 13) for OK or BELL (Ascii 7) for ERROR.
 *
 */
int COMMAND_CAN_SetBitRate()
{
    int c;
    UINT32 bitrate;

    if (can_flags & CAN_FLAG_OPEN)
	return ERROR;
    c = CBUF_Char();
    if (!CBUF_Empty())
	return ERROR;
    switch(c) {
    case '0': bitrate = 10000; break;
    case '1': bitrate = 20000; break;
    case '2': bitrate = 50000; break;
    case '3': bitrate = 100000; break;
    case '4': bitrate = 125000; break;
    case '5': bitrate = 250000; break;
    case '6': bitrate = 500000; break;
    case '7': bitrate = 800000; break;
    case '8': bitrate = 1000000; break;
    default:
	return ERROR;
    }
    can_bitrate = bitrate;
    return OK;
}

/*
 * O[CR] Open the CAN channel. 
 * This command is only active if the CAN channel is closed and 
 * has been set up prior with either the S or s command (i.e. initiated). 
 * Example: O[CR] 
 * Open the channel 
 *     Returns: CR (Ascii 13) for OK or BELL (Ascii 7) for ERROR.
 */

int COMMAND_CAN_Open()
{
    if (!CBUF_Empty())
	return ERROR;
    if (can_flags & CAN_FLAG_OPEN)
	return ERROR;
    if (can_bitrate == 0)
	return ERROR;
    CAN_SetBitRate(CAN_ID, can_bitrate);
    CAN_Enable(CAN_ID);
    return OK;
}

/*
 * C[CR] Close the CAN channel. 
 * This command is only active if the CAN channel is open. 
 *  Example: C[CR] 
 * Close the channel 
 * Returns: CR (Ascii 13) for OK or BELL (Ascii 7) for ERROR.
 */
int COMMAND_CAN_Close()
{
    if (!CBUF_Empty())
	return ERROR;
    if (!(can_flags & CAN_FLAG_OPEN))
	return ERROR;
    can_flags &= ~CAN_FLAG_OPEN;
    CAN_Disable(CAN_ID);
    return OK;
}

/*
 * tiiildd...[CR] Transmit a standard (11bit) CAN frame. 
 * This command is only active if the CAN channel is open. 
 * iii Identifier in hex (000-7FF) 
 * l Data length (0-8) 
 * dd Byte value in hex (00-FF). Numbers of dd pairs 
 * must match the data length, otherwise an error 
 * occur. 
 * Example 1: t10021133[CR] 
 * Sends an 11bit CAN frame with ID=0x100, 2 bytes 
 * with the value 0x11 and 0x33. 
 * Example 2: t0200[CR] 
 * Sends an 11bit CAN frame with ID=0x20 & 0 bytes. 
 * Returns: z[CR] for OK or BELL (Ascii 7) for ERROR.
 *
 *
 * Tiiiiiiiildd...[CR] Transmit an extended (29bit) CAN frame. 
 * This command is only active if the CAN channel is open. 
 * iiiiiiii Identifier in hex (00000000-1FFFFFFF) 
 * l Data length (0-8) 
 * dd Byte value in hex (00-FF). Numbers of dd pairs 
 * must match the data length, otherwise an error 
 * occur. 
 * Example 1: t0000010021133[CR] 
 * Sends a 29bit CAN frame with ID=0x100, 2 bytes 
 * with the value 0x11 and 0x33. 
 * Returns: Z[CR] for OK or BELL (Ascii 7) for ERROR.
 *
 */

int COMMAND_CAN_Transmit(BOOL ext)
{
    CAN_Message msg;

    if (!(can_flags & CAN_FLAG_OPEN))
	return ERROR;
    if (ext) {
	if (PARSE_Hex(8, &msg.mId) < 0)
	    return ERROR;
    }
    else {
	if (PARSE_Hex(3, &msg.mId) < 0)
	    return ERROR;
    }
    if (PARSE_Hex(1, &msg.mLen) < 0)
	return ERROR;
    if (msg.mLen > 8)
	return ERROR;
    if (PARSE_DHex(msg.mLen*2, &msg.u.wData[0], &msg.u.wData[1]) < 0)
	return ERROR;
    if (!CBUF_Empty())
	return ERROR;
    if (ext)
	msg.mLen |= CAN_MESSAGE_EXT;
    CAN_Send(CAN_ID, &msg);
    return OK;
}

/*
 * F[CR] Read Status Flags. 
 * This command is only active if the CAN channel is open. 
 * Example 1: F[CR] 
 * Read Status Flags. 
 * Returns: An F with 2 bytes BCD hex value plus CR (Ascii 13) 
 * for OK. If CAN channel isn’t open it returns BELL 
 * (Ascii 7). This command also clear the RED Error 
 * LED. See availible errors below. E.g. F01[CR] 
 * Bit 0 CAN receive FIFO queue full 
 * Bit 1 CAN transmit FIFO queue full 
 * Bit 2 Error warning (EI), see SJA1000 datasheet 
 * Bit 3 Data Overrun (DOI), see SJA1000 datasheet 
 * Bit 4 Not used. 
 * Bit 5 Error Passive (EPI), see SJA1000 datasheet 
 * Bit 6 Arbitration Lost (ALI), see SJA1000 datasheet * 
 * Bit 7 Bus Error (BEI), see SJA1000 datasheet ** 
 * * Arbitration lost doesn’t generate a blinking RED light! 
 * ** Bus Error generates a constant RED light!
 */
int COMMAND_CAN_Status(void)
{
    char ibuf[3];
    char* ptr;
    UINT8 status;

    if (!CBUF_Empty())
	return ERROR;
    if (!(can_flags & CAN_FLAG_OPEN))
	return ERROR;
    status = CAN_Read_Status(CAN_ID);
    ptr = UnsignedToString(status, 16, ibuf, sizeof(ibuf), 2);
    UART_WriteString(UART_ID, ptr);
    return OK;
}

int COMMAND_CAN_ACR(void)
{
    return ERROR;
}

int COMMAND_CAN_AMR(void)
{
    return ERROR;
}

/*
 * V[CR] Get Version number of both CANUSB hardware and software 
 * This command is active always. 
 * Example: V[CR] 
 * Get Version numbers 
 * Returns: V and a 2 bytes BCD value for hardware version and 
 * a 2 byte BCD value for software version plus 
 * CR (Ascii 13) for OK. E.g. V1013[CR] 
 */
int COMMAND_CAN_Version(void)
{
    if (!CBUF_Empty())
	return ERROR;
    UART_WriteString(UART_ID, "1010");
    return OK;
}

/*
 * N[CR] Get Serial number of the CANUSB. 
 * This command is active always. 
 * Example: N[CR]
 * Get Serial number
 * Returns: N and a 4 bytes value for serial number plus 
 * CR (Ascii 13) for OK. E.g. NA123[CR] 
 * Note that the serial number can have both numerical 
 * and alfa numerical values in it. The serial number is 
 * also printed on the CANUSB for a quick reference, 
 * but could e.g. be used in a program to identify a 
 * CANUSB so the program know that it is set up in the 
 * correct way (for parameters saved in EEPROM).
 *
 */

int COMMAND_CAN_SerialNumber(void)
{
    if (!CBUF_Empty())
	return ERROR;
    // FIXME - sync with USB serial?
    UART_WriteString(UART_ID, "0000");
    return OK;
}

/*
 * Zn[CR] Sets Time Stamp ON/OFF for received frames only. 
 * This command is only active if the CAN channel is closed. 
 * The value will be saved in EEPROM and remembered next time 
 * the CANUSB is powered up. This command shouldn’t be used more 
 * than when you want to change this behaviour. It is set to OFF by 
 * default, to be compatible with old programs written for CANUSB. 
 * Setting it to ON, will add 4 bytes sent out from CANUSB with the A 
 * and P command or when the Auto Poll/Send feature is enabled. 
 * When using Time Stamp each message gets a time in milliseconds 
 * when it was received into the CANUSB, this can be used for real 
 * time applications for e.g. knowing time inbetween messages etc. 
 * Note however by using this feature you maybe will decrease 
 * bandwith on the CANUSB, since it adds 4 bytes to each message 
 * being sent (specially on VCP drivers). 
 * If the Time Stamp is OFF, the incomming frames looks like this: 
 * t10021133[CR] (a standard frame with ID=0x100 & 2 bytes) 
 * If the Time Stamp is ON, the incomming frames looks like this: 
 * t100211334D67[CR] (a standard frame with ID=0x100 & 2 bytes) 
 * Note the last 4 bytes 0x4D67, which is a Time Stamp for this 
 * specific message in milliseconds (and of course in hex). The timer 
 * in the CANUSB starts at zero 0x0000 and goes up to 0xEA5F before 
 * it loop arround and get’s back to 0x0000. This corresponds to exact 
 * 60,000mS (i.e. 1 minute which will be more than enough in most 
 * systems). 
 * Example 1: Z0[CR] 
 * Turn OFF the Time Stamp feature (default). 
 * Example 2: Z1[CR] 
 * Turn ON the Time Stamp feature. 
 * Returns: CR (Ascii 13) for OK or BELL (Ascii 7) for ERROR.
 */

int COMMAND_CAN_TimeStamp(void)
{
    int c;
    if (can_flags & CAN_FLAG_OPEN)
	return ERROR;
    c = CBUF_Char();
    if (!CBUF_Empty())
	return ERROR;
    switch(c) {
    case '0':
	can_flags &= ~CAN_FLAG_TSTAMP;
	break;
    case '1':
	can_flags |= ~CAN_FLAG_TSTAMP;
	break;
    default:
	return ERROR;
    }
    return OK;
}

/*************************************************************************
 *
 * ADC commands
 *
 ************************************************************************/

void APP_ADC_Init()
{
    int i;

    adc_chan_mask = 0;

    MUX_Init();     // Set up MUX-0 and MUX-1
    MUX_0_Set(0);   // Make sure MUX-0 set set  0
    MUX_1_Set(0);   // Make sure MUX-1 set set  0
    AD_Init();      // Initailzie/Setup AD channels
    
    for (i = 0; i < ADC_NUM_CHANNELS; i++) {
	adc[i].flags    = 0;
	adc[i].v        = 0;
	adc[i].last_v   = (UINT16)-1;
	adc[i].tr       = 0;
	adc[i].tv       = 0;
	adc[i].interval = 0;
	adc[i].timer    = 0;
	adc[i].nr       = 0;
    }
}

/*
 * Ann<tttt><CR>
 *
 * 	When tttt > 0 the enable and start sample the analogue channel n
 * 	[0-F]. The channel is sampled once every tttt milliseconds.
 * 	If tttt = 0 then disable the channel
 *
 * 	tttt is the sample time	in milliseconds as a hex number.
 * 	Max number is EA60 = 60000, meaning that it's
 * 	not possible to have sample time longer than 1 minute.
 * 	If tttt is not given a sample time of 1000 milliseconds (03E8) is
 * 	 assumed.
 *
 */
int COMMAND_ADC_Enable()
{
    UINT32 i;
    UINT32 t;

    if (PARSE_ADC_Channel(&i) < 0)
	return ERROR;
    if (PARSE_Timer4(&t) < 0)
	return ERROR;
    if (!CBUF_Empty())
	return ERROR;
    adc[i].flags    = 0;
    adc[i].v        = 0;
    adc[i].last_v   = (UINT16)-1;
    adc[i].tv       = 0;
    adc[i].tr       = 0;
    adc[i].timer    = 0;
    adc[i].interval = 0;
    if (t > 0) {
	adc[i].timer = TIME_GetTick();
	adc[i].tr = adc[i].timer;
	adc[i].flags |= ADC_FLAG_ACTIVE;
	adc_chan_mask |= (1 << i);
    }
    else {
	adc_chan_mask &= ~(1 << i);
    }
    adc[i].interval = t*1000;  // interval in micro seconds
    return OK;
}

/*
 * ann<CR>
 * 	Asynchrounous mode. Report AD samples every sample time
 * 	as setup by An
 *
 * 	The sample will be reported as:
 *
 *	    AnvvvvTTTT<CR>
 *
 *	 Where the n is the input channel, vvvv is hex 16 bit sample number
 *	 In the case of 10 bit samples the value is scaled to the top 16 bits.
 *	 To convert back to 10 bits then vvvv >> 6 is needed but to scale
 *	 to float [0.0 - 1.0] then (vvvv/16#ffff) will be fine.
 *	 TTTT is the number of milliseconds since last read.
 *
 *	 Command respons is either <CR> for ok or <BELL> for error.
 *	 The channel must have been setup before this command (An)
 */

int COMMAND_ADC_AsyncMode()
{
    UINT32 i;

    if (PARSE_ADC_Channel(&i) < 0)
	return ERROR;
    if (!CBUF_Empty())
	return ERROR;
    if (!(adc[i].flags & ADC_FLAG_ACTIVE))
	return ERROR;
    adc[i].flags = (adc[i].flags & ADC_MASK_MODE) | ADC_FLAG_ASYNC;
    return OK;
}

/*
 * bnn<CR>
 * 	Change mode. Report AD samples when either sample value was
 * 	changed since last sample or if last report time is greater
 * 	than one minute. After isssue the command a sample will be
 * 	reported directly in order to initialize the sequence.
 *
 *	Samples will be reported the same way as the an command.
 *
 *	 Command respons is either <CR> for ok or <BELL> for error.
 *	 The channel must have been setup before this command (An)
 */

int COMMAND_ADC_ChangeMode()
{
    UINT32 i;

    if (PARSE_ADC_Channel(&i) < 0)
	return ERROR;
    if (!CBUF_Empty())
	return ERROR;
    if (!(adc[i].flags & ADC_FLAG_ACTIVE))
	return ERROR;
    adc[i].flags = (adc[i].flags & ADC_MASK_MODE) | 
	(ADC_FLAG_ONCE|ADC_FLAG_CHANGE);
    return OK;
}


/*
 * rnn<CR>
 * 	Syncrounous read analouge value from channel nn.
 *
 *	Command respons is either RnvvvvTTTT<CR> when ok
 *	or  <BELL> for error.
 *	The channel must have been setup before this command (An)
 */
int COMMAND_ADC_Read()
{
    UINT32 i;
    if (PARSE_ADC_Channel(&i) < 0)
	return ERROR;
    if (!CBUF_Empty())
	return ERROR;
    if (!(adc[i].flags & ADC_FLAG_ACTIVE))
	return ERROR;
    adc[i].flags |= ADC_FLAG_READ;
    return IGNORE;
}


/*
 * s[n]
 *   s0  report channels 1-8 in millivolts in step of 2mV
 *   si  when i is 1-8 reports channel [i+1] as binary 2 byte value
 *   s9  report all channels 1-8 as binary 2 byte values (16 bytes)
 */
int COMMAND_ADC_si()
{
    UINT32 n;

    if (PARSE_Hex(1, &n) < 0)
	return ERROR;
    if (n == 0) {
	int i;
	for (i = 0; i < 8; i++)
	    REPORT_si(i);
    }
    else if ((n >= 1) && (n <= 8)) {
	EMIT_si(n-1);
    }
    else if (n == 9) {
	UINT16 sum = 0;
	int i;
	for (i = 0; i < 8; i++)
	    sum += EMIT_sx(i);
	UART_WriteChar(UART_ID, sum & 0xFF);
    }
    else
	return ERROR;
    return IGNORE;
}


/*************************************************************************
 *
 * DIG commands
 *
 ************************************************************************/

void APP_DIG_Init(void)
{
    int i;

    DIGI_Init();

    dig_chan_mask = 0;

    for (i = 0; i < DIG_NUM_CHANNELS; i++) {
	dig[i].flags    = 0;
	dig[i].counter  = 0;
	dig[i].tr       = 0;
	dig[i].interval = 0;
	dig[i].timer    = 0;
	dig[i].nr       = 0;
    }
}

/*
 * Dnn<tttt><CR>
 *
 * 	Enable or disable digital counter sampling.
 *	If tttt > 0 then set up digital input counter n [0-2].
 *	tttt is the sample time	in milliseconds as a hex number.
 *	Max number is EA60 = 60000, meaning that it's
 *	not possible to have sample time longer than 1 minute.
 *
 *	if tttt = 0 then disable the counter sampling.
 */

int COMMAND_DIG_Enable(void)
{
    UINT32 i;
    UINT32 t;

    if (PARSE_DIG_Channel(&i) < 0)
	return ERROR;
    if (PARSE_Timer4(&t) < 0)
	return ERROR;
    if (!CBUF_Empty())
	return ERROR;
    dig[i].flags    = 0;
    dig[i].counter  = 0;
    dig[i].timer    = 0;
    dig[i].tr       = 0;
    dig[i].interval = 0;
    if (t > 0) {
	dig[i].timer = TIME_GetTick();
	dig[i].tr = dig[i].timer;
	dig[i].flags |= DIG_FLAG_ACTIVE;
	if (!dig_chan_mask)
	    DIGI_Enable();  // Turn on match interrupt
	dig_chan_mask |= (1 << i);	
    }
    else {
	dig_chan_mask &= ~(1 << i);
	if (!dig_chan_mask)
	    DIGI_Disable();  // Turn off match interrupt
    }
    dig[i].interval = t*1000;  // interval in micro seconds
    return OK;
} 

/*
 * dnn<CR>
 * 	Start report the digital counter values asynchronously.
 * 	The sample will be reported as:
 * 
 * 	    DnccccTTTT<CR>
 * 
 * 	 Where the n is the digital channel, cccc is hex 16 bit counter number.
 * 	 Since last read. A read will reset the counter.
 * 	 TTTT is the the number of milliseconds since the last read.
 *
 */
int COMMAND_DIG_AsyncMode(void)
{
    UINT32 i;

    if (PARSE_DIG_Channel(&i) < 0)
	return ERROR;
    if (!CBUF_Empty())
	return ERROR;
    if (!(dig[i].flags & DIG_FLAG_ACTIVE))
	return ERROR;
    dig[i].flags |= DIG_FLAG_ASYNC;
    return OK;
}


/*
 * enn<CR>
 *	Read the digital input bit counter n.
 *	Reply as:
 *
 *	    Encccctttt<CR>
 */

int COMMAND_DIG_Read(void)
{
    UINT32 i;

    if (PARSE_DIG_Channel(&i) < 0)
	return ERROR;
    if (!CBUF_Empty())
	return ERROR;
    if (!(dig[i].flags & DIG_FLAG_ACTIVE))
	return ERROR;
    dig[i].flags |= DIG_FLAG_READ;
    return IGNORE;
}


/*
 * B<n>
 *  Set UART baud rate baud as hex number
 */

int COMMAND_UART_Baud(void)
{
    UINT32 baud;

    if (PARSE_Number(7, &baud) < 0)
	return ERROR;

    if (!CBUF_Empty())
	return ERROR;    
    UART_SetBaud(UART_ID, baud);
    return OK;
}


int COMMAND_APP_Info(void)
{
    if (!CBUF_Empty())
	return ERROR;
    REPORT_loop();
    return OK;
}


void ADC_Poll(int i)
{
    UINT32 flags = adc[i].flags;

    if (!(flags & ADC_FLAG_ACTIVE))
	return;
    if (flags & ADC_FLAG_READ) {
	adc[i].flags &= ~ADC_FLAG_READ;
	ADC_Report(i, 'R');
    }
    if (flags & ADC_FLAG_ONCE) {
	adc[i].flags &= ~ADC_FLAG_ONCE;
	ADC_Report(i, 'A');
    }
    if (flags & ADC_FLAG_CHANGE) {
	if (TIME_IsTimeout(adc[i].timer, adc[i].interval)) {
	    if ((adc[i].v != adc[i].last_v) ||
		TIME_IsTimeout(adc[i].tr, 60*1000*1000)) {
		ADC_Report(i, 'A');
		adc[i].last_v = adc[i].v;
	    }
	    adc[i].timer = TIME_GetTick();
	}
    }
    if (flags & ADC_FLAG_ASYNC) {
	if (TIME_IsTimeout(adc[i].timer, adc[i].interval)) {
	    ADC_Report(i, 'A');
	    adc[i].timer = TIME_GetTick();
	}
    }
}

/*
 * Scan trough all AD channels and increment mux addresses
 */
void AD_Poll()
{
    UINT8  i;
    UINT8  j;

    AD_StartSample(1);  // Mux-0 <= Channel 1
    i = MUX_0_Get();
    while(!AD_GetSample(&adc[i].v))
	;
    adc[i].tv = TIME_GetTick();
    MUX_0_Set(i+1);


    AD_StartSample(0);  // Mux-1 <= Channel 0
    j = MUX_1_Get();
    i = j + 8;
    while(!AD_GetSample(&adc[i].v))
	;
    adc[i].tv = TIME_GetTick();
    MUX_1_Set(j+1);

    AD_StartSample(2);
    while(!AD_GetSample(&adc[16].v))
	;
    adc[16].tv = TIME_GetTick();

    AD_StartSample(3);
    while(!AD_GetSample(&adc[17].v))
	;
    adc[17].tv = TIME_GetTick();
}

void DIG_Poll(int i)
{
    UINT32 flags = dig[i].flags;

    if (!(flags & DIG_FLAG_ACTIVE))
	return;
    if (flags & DIG_FLAG_READ) {
	dig[i].flags &= ~DIG_FLAG_READ;
	DIG_Report(i, 'E');
    }
    if (flags & DIG_FLAG_ASYNC) {
	if (TIME_IsTimeout(dig[i].timer, dig[i].interval)) {
	    DIG_Report(i, 'D');
	    dig[i].timer = TIME_GetTick();
	}
    }
}

/*
 * Read digitial counter values
 */
void DI_Poll(void)
{
    if (dig_chan_mask & 0x01)
	dig[0].counter = DIGI_Read(0, TRUE);
    if (dig_chan_mask & 0x02)
	dig[1].counter = DIGI_Read(1, TRUE);
    if (dig_chan_mask & 0x04)
	dig[2].counter = DIGI_Read(2, TRUE);
}

int APP_Dispatch(void)
{
    if (CBUF_Empty())
	return OK;
    switch(CBUF_Char()) {
	// CANUSB commands
    case 'S': return COMMAND_CAN_SetBitRate();
    case 'O': return COMMAND_CAN_Open();
    case 'C': return COMMAND_CAN_Close();
    case 't': return COMMAND_CAN_Transmit(FALSE);
    case 'T': return COMMAND_CAN_Transmit(TRUE);
    case 'F': return COMMAND_CAN_Status();
    case 'M': return COMMAND_CAN_ACR();
    case 'm': return COMMAND_CAN_AMR();
    case 'V': return COMMAND_CAN_Version();
    case 'N': return COMMAND_CAN_SerialNumber();
    case 'Z': return COMMAND_CAN_TimeStamp();
	// ADC commands
    case 'A': return COMMAND_ADC_Enable();
    case 'a': return COMMAND_ADC_AsyncMode();
    case 'b': return COMMAND_ADC_ChangeMode();
    case 'r': return COMMAND_ADC_Read();
	// DIG commands
    case 'D': return COMMAND_DIG_Enable();
    case 'd': return COMMAND_DIG_AsyncMode();
    case 'e': return COMMAND_DIG_Read();
	// s0..s9 can be used with DMM ProfileLab
    case 's': return COMMAND_ADC_si();
	// MISC Command
    case 'B': return COMMAND_UART_Baud();
    case 'i': return COMMAND_APP_Info();
    default:
	return ERROR;
    }
}


void APP_Input(char c)
{
    switch(c) {
    case XOFF:
	output_enable = FALSE;
	break;
    case XON:
	output_enable = TRUE;
	break;
    case '\r':
    case '\n':
	switch(APP_Dispatch()) {
	case ERROR:
	    UART_WriteChar(UART_ID, BELL);
	    break;
	case OK:
	    UART_WriteChar(UART_ID, CR);
	case IGNORE:
	default:
	    break;
	}
	CBUF_Init();
	break;
    default:
	CBUF_AppendChar(c);
	break;
    }
}

void APP_Init(void)
{
    output_enable = TRUE;

    CBUF_Init();
    APP_CAN_Init();
    APP_ADC_Init();
    APP_DIG_Init();
    
    UART_Init(UART_ID, 115200);
    CAN_Init(CAN_ID);
}


int main(void)
{
    int i;

    POWER_Up(FALSE);

    APP_Init();
    
    for (i = 0; i < 8; i++)
	AD_Poll();
    DI_Poll();

    main_loops = 0;
    main_loops_0 = 0;
    main_loops_t = TIME_GetTick();

    while(1) {
	int c = UART_PollChar(UART_ID);
	UINT32 mask;

	if (c >= 0)
	    APP_Input(c);

	AD_Poll();
	DI_Poll();

	i = 0;
	mask = adc_chan_mask;
	while(mask) {
	    if (mask & 1)
		ADC_Poll(i);
	    mask >>= 1;
	    i++;
	}

	i = 0;
	mask = dig_chan_mask;
	while(mask) {
	    if (mask & 1)
		DIG_Poll(i);
	    mask >>= 1;
	    i++;
	}

	if (can_flags & CAN_FLAG_OPEN) {
	    CAN_Message msg;

	    if (CAN_Recv(CAN_ID, &msg)) {
		CAN_Report(&msg);
	    }
	}
	main_loops++;
    }
}
