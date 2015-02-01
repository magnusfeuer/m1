/*
 * ADC Device simulator
 *  
 */
/*
** Simulatin functions
*/
#ifdef ISC32
#define _POSIX_SOURCE
#define _XOPEN_SOURCE
#endif
#define _GNU_SOURCE

#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <poll.h>

#include <dlfcn.h>
#include <termios.h>
#include <errno.h>

// FIXME configure
// Darwin have poll that only works on sockets & pipes (BAD karma)
#ifdef DARWIN
#include <sys/select.h>
#define HAVE_SELECT
#define HAVE_BROKEN_POLL
#else
#define HAVE_POLL
#endif

#include "../include/types.h"
#include "../include/cfg.h"
#include "../include/gpio.h"
#include "../include/mux.h"
#include "../include/ad.h"
#include "../include/can.h"

static UINT8 mux0 = 0;
static UINT8 mux1 = 0;



int do_poll(struct pollfd* fds, int nfds, int timeout);

struct timeval boot_time;

#ifndef SIGRETTYPE		/* allow override via Makefile */
#define SIGRETTYPE void
#endif


int tty_fd = -1;
static struct termios tty_smode;
static struct termios tty_rmode;

static SIGRETTYPE ctl_c(int);
static SIGRETTYPE suspend(int);
static SIGRETTYPE (*orig_ctl_c)(int);

SIGRETTYPE (*sys_sigset(int sig, SIGRETTYPE (*func)(int)))(int)
{
    struct sigaction act, oact;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = func;
    sigaction(sig, &act, &oact);
    return(oact.sa_handler);
}

void sys_sigblock(int sig)
{
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, sig);
    sigprocmask(SIG_BLOCK, &mask, (sigset_t *)NULL);
}

void sys_sigrelease(int sig)
{
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, sig);
    sigprocmask(SIG_UNBLOCK, &mask, (sigset_t *)NULL);
}


/* of terminal device */
static int tty_set(int fd)
{
    if (tcsetattr(fd, TCSANOW, &tty_smode) < 0)
	return -1;
    return 0;
}

/* of terminal device */
static int tty_reset(int  fd)
{
    if (tcsetattr(fd, TCSANOW, &tty_rmode) < 0)
	return -1;
    return 0;
}


static SIGRETTYPE suspend(int sig)	
{
    tty_reset(tty_fd);

    sys_sigset(sig, SIG_DFL);	/* Set signal handler to default */
    sys_sigrelease(sig);	/* Allow 'sig' to come through */
    kill(getpid(), sig);	/* Send ourselves the signal */
    sys_sigblock(sig);		/* Reset to old mask */
    sys_sigset(sig, suspend);	/* Reset signal handler */

    tty_set(tty_fd);
}


static SIGRETTYPE ctl_c(int sig)
{
    tty_reset(tty_fd);

    sys_sigset(sig, orig_ctl_c); /* Set ctl_c break handler to original */
    sys_sigrelease(sig);	/* Allow 'sig' to come through */
    kill(getpid(), sig);	/* Send ourselves the signal */
    sys_sigblock(sig);		/* Reset to old mask */
    sys_sigset(sig, ctl_c);	/* Reset signal handler */ 

    tty_set(tty_fd);
}


/* tty_init(int fd, unsigned int baudrate)
 *  SETUP
 *  map cr -> nl on input
 *  map nl -> nl cr on output
 */

int tty_init(int fd, unsigned int baudrate)
{
    /* FIXME: handle speed */
    if (tcgetattr(fd, &tty_rmode) < 0)
      return -1;
    tty_smode = tty_rmode;

    /* Default characteristics for all usage including termcap output. */
    tty_smode.c_iflag &= ~ISTRIP;

    tty_smode.c_iflag |= ICRNL;     /* cr -> nl on input */
    tty_smode.c_lflag &= ~ICANON;   
    tty_smode.c_oflag |= OPOST;     /* nl -> cr-nl .. */
    /* Must get these really right or funny effects can occur. */
    tty_smode.c_cc[VMIN] = 1;
    tty_smode.c_cc[VTIME] = 0;
#ifdef VDSUSP
    tty_smode.c_cc[VDSUSP] = 0;
#endif

    tty_smode.c_cflag &= ~(CSIZE | PARENB); /* clear char-size,diable parity */
    tty_smode.c_cflag |= CS8;               /* 8-bit */
    tty_smode.c_lflag &= ~ECHO;             /* no echo */

    if (tty_set(fd) < 0) {
	tty_fd = -1;
	tty_reset(fd);
	return -1;
    }
    tty_fd = fd;

    sys_sigset(SIGTSTP, suspend);
    sys_sigset(SIGTTIN, suspend);
    sys_sigset(SIGTTOU, suspend);
    orig_ctl_c = sys_sigset(SIGINT, ctl_c);

    return 0;
}

/* void tty_terminate()
 *  Reset the terminal to the state that existed before init_term()
 *  was called.
 */
int tty_terminate(int fd)
{
    if (tty_fd == fd)
	tty_reset(tty_fd);
    return 0;
}

/* compatiblilty problems on mac os x :-(  */
#if defined(HAVE_POLL)
int do_poll(struct pollfd* fds, int nfds, int timeout)
{
    return poll(fds, nfds, timeout);
}
#elif defined(HAVE_SELECT) || defined(HAVE_BROKEN_POLL)
#include <sys/select.h>
int do_poll(struct pollfd* fds, int nfds, int timeout)
{
    fd_set rfds;
    fd_set wfds;
    fd_set xfds;
    struct timeval tm;
    struct timeval* tp;
    int maxfd = 0;
    int i;
    int r;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&xfds);

    if (timeout < 0)
	tp = NULL;
    else {
	tm.tv_sec = timeout / 1000;
	tm.tv_usec = (timeout % 1000)*1000;
	tp = &tm;
    }

    for (i = 0; i< nfds; i++) {
	if (fds[i].fd < 0)
	    fds[i].revents = POLLNVAL;
	else {
	    fds[i].revents = 0;

	    if (fds[i].events & POLLIN) {
		FD_SET(fds[i].fd, &rfds);
		if (fds[i].fd > maxfd) maxfd = fds[i].fd;
	    }
#ifdef POLLRDNORM
	    if (fds[i].events & POLLRDNORM) {
		FD_SET(fds[i].fd, &rfds);
		if (fds[i].fd > maxfd) maxfd = fds[i].fd;
	    }
#endif
#ifdef POLLRDBAND
	    if (fds[i].events & POLLRDBAND) {
		FD_SET(fds[i].fd, &xfds);
		if (fds[i].fd > maxfd) maxfd = fds[i].fd;
	    }
#endif
	    if (fds[i].events & POLLOUT) {
		FD_SET(fds[i].fd, &wfds);
		if (fds[i].fd > maxfd) maxfd = fds[i].fd;
	    }
#ifdef 	POLLWRNORM
	    if (fds[i].events & POLLWRNORM) {
		FD_SET(fds[i].fd, &wfds);
		if (fds[i].fd > maxfd) maxfd = fds[i].fd;
	    }
#endif

#ifdef 	POLLWRBAND
	    if (fds[i].events & POLLWRBAND) {
		FD_SET(fds[i].fd, &xfds);
		if (fds[i].fd > maxfd) maxfd = fds[i].fd;
	    }
#endif
	    
#ifdef POLLPRI
	    if (fds[i].events & POLLPRI) {
		FD_SET(fds[i].fd, &xfds);
		if (fds[i].fd > maxfd) maxfd = fds[i].fd;
	    }
#endif
	}
    }
    
again:
    if ((r = select(maxfd+1, &rfds, &wfds, &xfds, tp)) <= 0) {
	/* probably an interrupt or poll with no input */
	if ((r == -1) && (errno == EBADF)) {
	    int fixed = 0;
	    /* check which fd that is bad */
	    for (i = 0; i <= nfds; i++) {
		if ((fds[i].fd >= 0) && FD_ISSET(fds[i].fd, &rfds)) {
		    fd_set efds;
		    tm.tv_sec  = 0;
		    tm.tv_usec = 0;
		    tp = &tm;
		    FD_ZERO(&efds);
		    FD_SET(fds[i].fd, &efds);
		    if (select(fds[i].fd+1, &efds, NULL, NULL, tp) == -1) {
			FD_CLR(fds[i].fd, &rfds);
			fds[i].revents |= POLLNVAL;
			fixed++;
		    }
		}
		if ((fds[i].fd >= 0) && FD_ISSET(fds[i].fd, &wfds)) {
		    fd_set efds;
		    tm.tv_sec  = 0;
		    tm.tv_usec = 0;
		    tp = &tm;
		    FD_ZERO(&efds);
		    FD_SET(fds[i].fd, &efds);
		    if (select(fds[i].fd+1, NULL, &efds, NULL, tp)  == -1) {
			FD_CLR(fds[i].fd, &wfds);
			fds[i].revents |= POLLNVAL;
			fixed++;
		    }
		}
	    }
	    if (fixed)
		goto again;
	}
	return r;
    }

    for (i = 0; i < nfds; i++) {
	/* FIXME: Handle HUP and pri data */
	if (FD_ISSET(fds[i].fd, &rfds))
	    fds[i].revents |= POLLIN;
	if (FD_ISSET(fds[i].fd, &wfds))
	    fds[i].revents |= POLLOUT;
    }
    return r;
}

#else
#error "no poll method found"
#endif

void SIMULATOR_init(void)
{
    gettimeofday(&boot_time, NULL);
}


UINT32 TIME_GetTick(void)
{
    struct timeval now;
    struct timeval t;
    gettimeofday(&now, NULL);
    timersub(&now, &boot_time, &t);
    return (t.tv_sec*1000000 + t.tv_usec);
}

BOOL TIME_IsTimeout(UINT32 dwTimeMark,UINT32 dwTimeout)
{
    UINT32 t = TIME_GetTick();
    dwTimeMark += dwTimeout;
    return ((dwTimeMark-t)&0x80000000)?TRUE:FALSE;
}

void TIME_Wait(int nDelay)
{
    do_poll(0, 0, nDelay);
}



void GPIO_SetPinDir(UINT8 pin, BOOL dir)
{
}

void GPIO_SetP0Function(UINT8 pin,UINT8 func)
{
}

void MUX_0_Set(UINT8 n)
{
    n &= 0x7;
    mux0 = n;
}

void MUX_1_Set(UINT8 n)
{
    n &= 0x7;
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


/*
 * Initialize the LPC AD converter inputs
 */
void AD_Init(void)
{
    GPIO_SetP0Function(PIN_AIN0, FUN_AIN0);
    GPIO_SetP0Function(PIN_AIN1, FUN_AIN1);
    GPIO_SetP0Function(PIN_AIN2, FUN_AIN2);
    GPIO_SetP0Function(PIN_AIN3, FUN_AIN3);
}

void AD_PowerUp()
{
    /* No powerup needed since PDN is set when ever sample is read */
}

void AD_PowerDown()
{
}

/*
 * Convert sample value to mV
 */
UINT16 AD_To_mV(UINT16 sample)
{
    UINT32 v;
    v = (sample * 3300) / 1024;
    return (UINT16) v;
}

/*
 * Start sample a channel 0-3
 */    

void AD_StartSample(UINT8 n)
{
}

/*
 *  Get sample from ADC unit return FALSE if not ready
 */

BOOL AD_GetSample(UINT16* samp)
{
    UINT32 val = random() % 0x3FF;

    *samp = val;
    return TRUE;
} 


UINT32 cap_counter[4];

void DIGI_Init(void)
{
    GPIO_SetP0Function(PIN_DIGI_IN_1, FUN_DIGI_IN_1);
    GPIO_SetP0Function(PIN_DIGI_IN_2, FUN_DIGI_IN_2);
    GPIO_SetP0Function(PIN_DIGI_IN_3, FUN_DIGI_IN_3);

    cap_counter[0] = 0;
    cap_counter[1] = 0;
    cap_counter[2] = 0;
    cap_counter[3] = 0;
}

void DIGI_Enable()
{
}

void DIGI_Disable()
{
}

UINT32 DIGI_Read(int i, BOOL reset)
{
    UINT32 v = cap_counter[i];
    if (reset)
	cap_counter[i] -= v;
    cap_counter[i] += (random() % 10);
    return v;
}


int uart[4];

int UART_SetBaud(int id, UINT32 baud)
{
    return 0;
}

void UART_Init(int id, UINT32 baud)
{
    if (id == 0) {
	uart[0] = 0; // open("/dev/tty", O_RDWR);
	tty_init(uart[0], baud);
    }
    else if (id == 1) {
	uart[1] = open("/dev/ttys000", O_RDWR);
	tty_init(uart[1], baud);
    }
    // UART_SetBaud(id, baud);
}

static BOOL UART_Tx_Ready(int fd)
{
    return TRUE;
}

static BOOL UART_Rx_Ready(int fd)
{
    struct pollfd pfd;

    pfd.fd = fd;
    pfd.events = POLLIN;
    if (do_poll(&pfd, 1, 0) == 0)
	return FALSE;
    if (pfd.revents & POLLIN)
	return TRUE;
    return FALSE;
}

/* Blocking write char */
void UART_WriteChar(int id, UINT8 c)
{
    int fd = uart[id];

    while(!UART_Tx_Ready(fd))
	;
    write(fd, &c, 1);
}

/* Blocking write data */
void UART_WriteData(int id, UINT8* buf, UINT32 len)
{
    int fd = uart[id];

    write(fd, buf, len);
}

void UART_WriteString(int id, char* str)
{
    int fd = uart[id];
    int len = strlen(str);

    write(fd, str, len);
}

/*
 * Non blocking read
 */
int UART_PollChar(int id)
{
    int fd = uart[id];

    if (UART_Rx_Ready(fd)) {
	UINT8 c;
	if (read(fd, &c, 1) == 1)
	    return c;
	return -1;
    }
    return -1;
}

/*
 * Blocking read char
 */
int UART_ReadChar(int id)
{
    int fd = uart[id];
    UINT8 c;

    while(!UART_Rx_Ready(fd))
	;
    if (read(fd, &c, 1) == 1)
	return c;
    
    return -1;
}

/*
 * Read blocking 
 */
int UART_ReadData(int id, UINT8* buf, UINT32 len)
{
    int fd = uart[id];
    int res = 0;

    while(len) {
	int n;
	n = read(fd, buf, len);
	if (n < 0)
	    return res;
	res += n;
	len -= n;
	buf += n;
    }
    return res;
}


// Private functions 
#ifdef ENABLE_CAN1
CAN_Rx gCan1Rx;
CAN_Tx gCan1Tx;
#endif
#ifdef ENABLE_CAN2
CAN_Rx gCan2Rx;
CAN_Tx gCan2Tx;
#endif
#ifdef ENABLE_CAN3
CAN_Rx gCan3Rx;
CAN_Tx gCan3Tx;
#endif
#ifdef ENABLE_CAN4
CAN_Rx gCan4Rx;
CAN_Tx gCan4Tx;
#endif

// CAN_TypeDef* gCanIf[4] = { CAN1, CAN2, CAN3, CAN4 };

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


void CAN_SetBitRate(int canid, UINT32 bitrate)
{
}

//
// Initialize the CAN_Rx data structure
//
void CAN_Setup(int canid)
{
    CAN_Rx* rx      = gCanRx[canid];
    CAN_Tx* tx      = gCanTx[canid];

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
}

void CAN_Enable(int canid)
{
}

void CAN_Disable(int canid)
{
}

//
// Send message on CAN
// 
//

void CAN_Send(int canid, CAN_Message* mesg)
{
}

//
// Recive message from CAN Rx buffer
//

BOOL CAN_Recv(int canid, CAN_Message* mesg)
{
    return FALSE;
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
    UINT8 status = 0;
    UINT32 gsr = 0;
    
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
#endif
	break;
    case 1:
#ifdef ENABLE_CAN2
	GPIO_SetP0Function(PIN_CAN2_RX, FUN_CAN2_RX);
	GPIO_SetP0Function(PIN_CAN2_TX, FUN_CAN2_TX);
	CAN_Setup(1);
#endif
	break;
    case 2:
#ifdef ENABLE_CAN3
	GPIO_SetP0Function(PIN_CAN3_RX, FUN_CAN3_RX);
	GPIO_SetP0Function(PIN_CAN3_TX, FUN_CAN3_TX);
	CAN_Setup(2);
#endif
	break;
    case 3:
#ifdef ENABLE_CAN4
	GPIO_SetP0Function(PIN_CAN4_RX, FUN_CAN4_RX);
	GPIO_SetP0Function(PIN_CAN4_TX, FUN_CAN4_TX);
	CAN_Setup(3);
#endif
	break;
    }
}


void POWER_Up(BOOL wokeUp)
{
}
