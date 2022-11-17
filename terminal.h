
/* These are the screen manipulation routines in terminal.c that are 
   exported for use in vt100.c
*/

extern char *vt_initterm(char *terminal_type, int *rows, int *cols);	/* Initialize the termcap, return NULL if successful */
extern void vt_bell(void);		/* Sound the terminal bell */
extern void vt_goto(int row, int col);		/* Goto a specific x y coordinate */
extern void vt_up(int numrows);		/* Move cursor up */
extern void vt_down(int numrows);		/* Move cursor down */
extern void vt_right(int numcols);		/* Move cursor right */
extern void vt_left(int numcols);		/* Move cursor left */
extern void vt_clrscr(void);	/* Clear whole screen */
extern void vt_clreos(void);	/* Clear to end of screen, include currline */
extern void vt_clrbgs(void);	/* Clear to beginning of screen, include currline */
extern void vt_clrline(void);	/* Clear line under cursor */
extern void vt_clreol(void);	/* Clear to the end of line */
extern void vt_clrbgl(void);	/* Clear to the beginning of line, including cursor */
extern void vt_delline(int num);	/* Delete line under cursor */
extern void vt_delunder(int num);	/* Delete character under cursor */
extern void vt_insline(int num);	/* Insert open line under cursor */
extern void vt_bold(int on);		/* Toggle bold display */
extern void vt_underline(int on);	/* Toggle underlined display */
extern void vt_blink(int on);		/* Toggle blinking display */
extern void vt_reverse(int on);	/* Toggle reversed display */
extern void vt_setattr(int textattr);	/* Set display attributes */
extern void vt_setfg(int color);		/* Set foreground color */
extern void vt_setbg(int color);		/* Set background color */
extern void vt_resetattr(void);	/* Reset display attributes */
extern void vt_setscroll(int upper, int lower);	/* Set scrolling region */
extern void vt_revscroll(void);	/* Reverse scroll */
extern void vt_altcharset(int charset, int type);	/* Toggle alternate character set (graphics) */
extern void vt_savecursor(void);	/* Save current hardware cursor position */
extern void vt_restcursor(void);	/* Restore saved hardware cursor position */
extern void vt_keystate(int application);	/* Set and reset application mode keys */
extern void vt_widemode(int on);	/* Set and reset 132 column mode */
extern void vt_rows_cols(char *termtype, int *rows, int *cols);	/* Return the rows and cols from termcap */
extern void vt_insertchar(int numcols);	/* Insert character in line */
extern void vt_update(void);	/* Flush any pending output */
