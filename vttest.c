/* vttest.c -- written by Charles Howes (chowes@sfu.ca) */

/* Modified to (hopefully) be more portable, run as a function,
   and test only for vt100 terminal emulation on 9/30/93

   This function will return 1 if the tty is vt100 or 0 if an error
   occurred or the terminal doesn't appear to be a vt100 terminal.

   Completely re-written 10/9/93 for better portability


	-Sam Lantinga		(slouken@toadflax.cs.ucdavis.edu)
*/

#include	<unistd.h>
#include	<sys/types.h>
#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#ifdef HAVE_TERMIO_H 
#include        <termio.h> 
#elif defined HAVE_TERMIOS_H
# include <termios.h>
# include <sys/ioctl.h>
# ifndef termio
#  define termio termios
# endif
#else /* !HAVE_TERMIO_H && !HAVE_TERMIOS_H */
#include	<setjmp.h> 
#include	<signal.h> 
#include	<sys/ioctl.h> 
#define termio	sgttyb 
#define TCGETA	TIOCGETP
#define TCSETAW	TIOCSETP
#endif /* HAVE_TERMIO_H */

#ifdef NEED_COMPAT_H
#include	<sys/ioctl_compat.h>
#endif /* NEED_COMPAT_H */

#ifdef HAVE_BSDTTY_H
#include        <sys/bsdtty.h>
#endif /* HAVE_BSDTTY_H */

#ifndef OLCUC
# define OLCUC 0	/* Missing inf FreeBSD and GNU/kFreeBSD. */
#endif

#ifdef MAIN
int main() { if ( vttest() ) printf("vt100\n"); }
#endif

/* Register that we are alarmed. (called by SIG_ALRM on BSD) */
static int alarmed;
#if !defined(HAVE_TERMIO_H) && !defined(HAVE_TERMIOS_H)
static jmp_buf alarm_buf;
static void alrm_trap() { alarmed=1; longjmp(alarm_buf, 1); }
#endif /* No termio.h */

int vttest() 
{
	char   buff[512];
	int x=0, rc=0, fd;
	struct termio ttold, ttraw;

	/* Disabled since at least on sparc it causes a hang. */
	return 1;
	
	/* Set the terminal in a raw mode */
	if ( (fd=open("/dev/tty", O_RDWR, 0666)) < 0 )
		return(0);

#ifdef HAVE_TERMIOS_H
	if ( tcgetattr(fd, &ttold) < 0 )
		return(0);
	(void) tcgetattr(fd, &ttraw);
#else /* !HAVE_TERMIOS_H */
	if ( ioctl(fd, TCGETA, (char *)&ttold) < 0 )
		return(0);
	(void) ioctl(fd, TCGETA, (char *)&ttraw);
#endif

#if defined(HAVE_TERMIO_H) || defined(HAVE_TERMIOS_H)
#ifdef SEVEN_BIT
        ttraw.c_iflag=(IGNBRK | ISTRIP);   /* turn off all input control */
#else
	ttraw.c_iflag=(IGNBRK);   /* turn off all input control */
#endif /* SEVEN_BIT */
        ttraw.c_oflag &= ~(OLCUC | ONLCR | OCRNL | ONLRET);
                                        /* disable output post-processing */
        ttraw.c_lflag = 0;
        ttraw.c_cc[VMIN]=0;          /* 1 or more chars satisfy read */
        ttraw.c_cc[VTIME]=10;        /* 10'ths of seconds between chars */
#else /* !HAVE_TERMIO_H && !HAVE_TERMIOS_H */
	ttraw.sg_flags |= RAW;		/* turn RAW mode on */
	ttraw.sg_flags &= ~ECHO;	/* turn ECHO off */
#endif /* HAVE_TERMIO_H */

#ifdef HAVE_TERMIOS_H
        if (tcsetattr(fd, TCSADRAIN, &ttraw) < 0)
#else /* !HAVE_TERMIOS_H */
        if (ioctl(fd, TCSETAW, (char *)&ttraw) < 0)
#endif
                return(0);

  	write(fd,"\033[c", 3);	/* Vt100 test: ESC [ c */

#if !defined(HAVE_TERMIO_H) && !defined(HAVE_TERMIOS_H)
	/* We need to set an alarm */
	signal(SIGALRM, alrm_trap);
	alarmed=0;
	alarm(1);
	setjmp(alarm_buf);
#endif
	while ( !alarmed && (x < 20) ) { 
		if ( read(fd, &buff[x++], 1) <= 0 )
			break;
	}
	buff[x]='\0';		/* For printing, if we desire. */
  	if ( buff[0] == '\033' )	/* An escape sequence? :) */
		rc=1;

#if !defined(HAVE_TERMIO_H) && !defined(HAVE_TERMIOS_H)
	alarm(0);
	signal(SIGALRM, SIG_DFL);
#endif
#if HAVE_TERMIOS_H
        (void) tcsetattr(fd, TCSADRAIN, &ttold);
#else /* !HAVE_TERMIOS_H */
        (void) ioctl(fd, TCSETAW, (char *)&ttold);
#endif
	(void) close(fd);

#ifdef not_defined		/* Print out the response for debugging */
	for ( w=0; buff[w]; ++w ) 
		if ( buff[w] < ' ' ) 
			printf("^%c", buff[w]+'@');
		else
			printf("%c", buff[w]);
	printf("\n");
#endif /* not_defined */

	return rc;
}
