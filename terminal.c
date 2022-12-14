
/* This is the upper layer of the splitvt terminal driver.
   It contains the termcap stuff and the output routines that 
   actually write to the terminal.
*/

#include	<curses.h>
#include	<term.h>
#include	<stdio.h>
#include	<string.h>
#include	"splitvt.h"
#include	"video.h"
#include	"terminal.h"


/* This is a long list of termcap capabilities that can be used by splitvt */

typedef struct {
	char *name;
	char *value;
	int   required;
	} capability;

static capability capabilities[] = {
#define cs 0
	{ "cs", NULL, 0 },		/* Scrolling regions: VITAL! */
#define cm 1
	{ "cm", NULL, 1 },		/* Cursor move to coordinates */
#define sc 2
	{ "sc", NULL, 0 },		/* Save cursor position */
#define rc 3
	{ "rc", NULL, 0 },		/* Restore cursor position */
#define sr 4
	{ "sr", NULL, 1 },		/* Reverse scroll 1 line */
#define sf 5
	{ "sf", "\n", 1 },		/* Forward scroll 1 line */
#define UP 6
	{ "UP", NULL, 0 },		/* Move cursor up n lines */
#define up 7
	{ "up", NULL, 0 },		/* Move cursor up 1 line */
#define DO 8
	{ "DO", NULL, 0 },		/* Move cursor down n lines */
#define do 9
	{ "do", NULL, 0 },		/* Move cursor down 1 line */
#define RI 10
	{ "RI", NULL, 0 },		/* Move cursor right n characters */
#define nd 11
	{ "nd", NULL, 0 },		/* Move cursor right 1 character */
#define LE 12
	{ "LE", NULL, 0 },		/* Move cursor left n characters */
#define le 13
	{ "le", "\b", 0 },		/* Move cursor left 1 character */
#define cr 14
	{ "cr", "\r", 1 },		/* Carriage return */
#define DC 15
	{ "DC", NULL, 0 },		/* Delete n characters */
#define dc 16
	{ "dc", NULL, 0 },		/* Delete character */
#define DL 17
	{ "DL", NULL, 0 },		/* Delete n lines */
#define dl 18
	{ "dl", NULL, 0 },		/* Delete line */
#define ce 19
	{ "ce", NULL, 1 },		/* Clear to end of line */
#define so 20
	{ "so", NULL, 0 },		/* Start standout text mode */
#define se 21
	{ "se", NULL, 0 },		/* End standout text mode */
#define us 22
	{ "us", NULL, 0 },		/* Start underline text mode */
#define ue 23
	{ "ue", NULL, 0 },		/* End underline text mode */
#define md 24
	{ "md", NULL, 0 },		/* Start bold text mode */
#define mb 25
	{ "mb", NULL, 0 },		/* Start blinking text mode */
#define mr 26
	{ "mr", NULL, 0 },		/* Start reverse text mode */
#define mh 27
	{ "mh", NULL, 0 },		/* Start dim text mode */
#define me 28
	{ "me", NULL, 0 },		/* Clear all text attributes */
#define ZF 29
	{ "ZF", NULL, 0 },		/* Set wide (132 column) mode */
#define IC 30
	{ "IC", NULL, 0 },		/* Insert character */
	{ NULL, NULL, 0 }
	};

#ifdef TERMCAP	/* Termcap versions of the screen update routines */

/* areas and variables used to get termcap information */

extern int tgetent();
extern char *tgetstr();

static char termcap_entry[2048];        /* termcap entry for the terminal */
static char termcap_area[2048];		/* Static buffer for capabilities */
static char *tp=termcap_area;		/* Used in tgetent() calls */
static char *terminal_type;             /* type of terminal */

char *termcap_init(char *termtype)
{
	static char errmesg[BUFSIZ];
	char *value;
	int i;

	/* Determine the terminal type and get the termcap entry */
	if ( tgetent(termcap_entry, termtype) < 1 ) {
                sprintf(errmesg,
        "Can't find termcap entry for terminal type \"%s\"", terminal_type);
                return(errmesg);
        }

	/* Make sure this terminal has all the required capabilities */	
	for ( i=0; capabilities[i].name; ++i ) {
		if ( (value=tgetstr(capabilities[i].name, &tp)) == NULL ) {
			if ( !capabilities[i].value && 
						capabilities[i].required ) {
				sprintf(errmesg, 
			"Your terminal lacks the %s termcap capability.",
						capabilities[i].name);
				return(errmesg);
			}
		} else
			capabilities[i].value=value;
	}
	return(NULL);
}
int outc(int c) { return putchar(c); }

void vt_rows_cols(char *termtype, int *rows, int *cols)
{
	if ( rows ) {
		if ( (*rows=tgetnum("li")) <= 0 )
			*rows=24;
	}
	if ( cols ) {
		if ( (*cols=tgetnum("co")) <= 0 )
			*cols=80;
	}
}
void vt_bold(int on)
{
	if ( on && capabilities[md].value ) 
		tputs(capabilities[md].value, 1, outc);
	else
		printf("\033[%sm", (on ? "1" : "22"));
}
void vt_underline(int on)
{
	if ( capabilities[on ? us : ue].value ) 
		tputs(capabilities[on ? us : ue].value, 1, outc);
	else
		printf("\033[%sm", on ? "4" : "24");
}
void vt_blink(int on)
{
	if ( on && capabilities[mb].value ) 
		tputs(capabilities[mb].value, 1, outc);
	else
		printf("\033[%sm", on ? "5" : "25");
}
void vt_reverse(int on)
{
	if ( on && capabilities[mr].value ) 
		tputs(capabilities[mr].value, 1, outc);
	else
		printf("\033[%sm", on ? "7" : "27");
}
void vt_resetattr(void)
{
	if ( capabilities[me].value ) 
		tputs(capabilities[me].value, 1, outc);
	else
		printf("\033[m");
}
void vt_widemode(int on)
{
	if ( on && capabilities[ZF].value ) 
		tputs(capabilities[ZF].value, 1, outc);
	else
		printf("\033[?3%c", on ? 'h' : 'l');
}
void vt_savecursor(void)
{
	if ( capabilities[sc].value )
		tputs(capabilities[sc].value, 1, outc);
	else
		printf("\0337");
}
void vt_restcursor(void)
{
	if ( capabilities[rc].value )
		tputs(capabilities[rc].value, 1, outc);
	else
		printf("\0338");
}
#else
char *termcap_init(char *termtype)
{
	return(NULL);
}
void vt_rows_cols(char *termtype, int *rows, int *cols)
{
	if ( cols ) {
		if ( strcmp("vt100-w", termtype) == 0 )
		{ /* vt100-wide terminal (132 columns) */
			*cols=132;
		} else
			*cols = 80;
	} 
	if ( rows )
		*rows = 24;
}
void vt_bold(int on)
{
	printf("\033[%sm", (on ? "1" : "22"));
}
void vt_underline(int on)
{
	printf("\033[%sm", on ? "4" : "24");
}
void vt_blink(int on)
{
	printf("\033[%sm", on ? "5" : "25");
}
void vt_reverse(int on)
{
	printf("\033[%sm", on ? "7" : "27");
}
void vt_resetattr(void)
{
	printf("\033[m");
}
void vt_widemode(int on)
{
	printf("\033[?3%c", on ? 'h' : 'l');
}
void vt_savecursor(void)
{
	printf("\0337");
}
void vt_restcursor(void)
{
	printf("\0338");
}
#endif
		/* vt100 compatible versions of the screen update routines */
char *vt_initterm(char *terminal_type, int *rows, int *cols)
{
	extern char *getenv(const char *name);
	static char *termtype=NULL, *error=NULL;

	if ( (termtype=getenv("TERM")) == NULL )
		return("Terminal type must be set to vt100");

	if ( strncmp(termtype, "vt10x", 4) != 0 &&
	     strncmp(termtype, "xterm", 5) != 0 &&
                        /* A vt10x emulation detector -->  */   ! vttest() )
                return("Terminal type must be set to vt100");

	if ( strlen(termtype) > 256 )
		return("Terminal type too long");

	/* Get extra (possibly) capabilities from termcap */
	if ( (error=termcap_init(termtype)) != NULL )
		return(error);

	vt_rows_cols(termtype, rows, cols);
	if ( terminal_type )
		strcpy(terminal_type, termtype);
	return(NULL);
}
void vt_bell(void)
{
	printf("\007");
}
void vt_goto(int row, int col)
{
	printf("\033[%d;%dH", row, col);
}
void vt_up(int numrows)
{
	printf("\033[%dA", numrows);
}
void vt_down(int numrows)
{
	printf("\033[%dB", numrows);
}
void vt_right(int numcols)
{
	printf("\033[%dC", numcols);
}
void vt_left(int numcols)
{
	printf("\033[%dD", numcols);
}
void vt_clrscr(void)
{
	printf("\033[2J");
}
void vt_clreos(void)
{
	printf("\033[J");
}
void vt_clrbgs(void)
{
	printf("\033[1J");
}
void vt_clrline(void)
{
	printf("\033[2K");
}
void vt_clreol(void)
{
	printf("\033[K");
}
void vt_clrbgl(void)
{
	printf("\033[1K");
}
void vt_delunder(int num)
{
	printf("\033[%dP", num);
}
void vt_delline(int num)
{
	printf("\033[%dM", num);
}
void vt_insline(int num)
{
	printf("\033[%dL", num);
}
void vt_setattr(int textattr)
{
	vt_resetattr();
	
	if ( textattr&BOLD )
		vt_bold(1);
	if ( textattr&UNDERLINE )
		vt_underline(1);
	if ( textattr&BLINK )
		vt_blink(1);
	if ( textattr&REVERSE )
		vt_reverse(1);
}
void vt_setfg(int color)
{
	printf("\033[%dm", color+30);
}
void vt_setbg(int color)
{
	printf("\033[%dm", color+40);
}
void vt_altcharset(int charset, int type)
{
	switch (type) {
		case UK_CHARSET:
				printf("\033%cA", (charset == G0 ? '(' : ')'));
				break;
		case US_CHARSET:
				printf("\033%cB", (charset == G0 ? '(' : ')'));
				break;
		case GRAPHICS:	
				printf("\033%c0", (charset == G0 ? '(' : ')'));
				break;
		default:	break;
	}
}
void vt_setscroll(int upper, int lower)
{
	if ( !upper && !lower ) {
		printf("\033[r");
	} else {
		printf("\033[%d;%dr", upper, lower);
	}
}
void vt_revscroll(void)
{
	printf("\033M");
}
void vt_keystate(int application)
{
	/* Set and reset the numeric keypad and arrowkeys */
	if ( application )
		printf("\033=\033[?1h");
	else
		printf("\033>\033[?1l");
}
void vt_insertchar(int numcols)
{
	printf("\033[%d@", numcols);
}
void vt_reset(void)
{
	printf("\033c");
}
void vt_update(void)
{
	fflush(stdout);
}
