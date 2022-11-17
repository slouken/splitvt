#include	<sys/types.h>
#include	<stdio.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include    <unistd.h>

/*#define DEBUG		/* Provides extra debugging info */

#define VERBOSE_PRINT(X)	if ( verbose ) printf(X)

#define F_OK	0
#define X_OK	1
#define W_OK	2
#define R_OK	4

int grep(char *dir, char *file, char *word);

int exists(char *dir, char *file)
{
	struct stat statbuf;
	char buffer[BUFSIZ];

	sprintf(buffer, "%s/%s", dir, file);
	if ( stat(buffer, &statbuf) == 0 )
		return(1);
	else
		return(0);
}

void finddeblibmultiarch(deblibmultiarch)
char *deblibmultiarch;
{
   FILE *pipe_fp;
   char debhostmultiarch[BUFSIZ];
   // Get host arch (i386-linux-gnu, amd64-linux-gnu,...)
   if (( pipe_fp = popen("dpkg-architecture -qDEB_HOST_MULTIARCH", "r")) == NULL){
      perror("popen error: Can't finish creating Makefile");
      printf("Configuration aborted. Exiting.\n");
      exit(2);
   }
   fscanf(pipe_fp,"%s",debhostmultiarch);
   pclose(pipe_fp);
   // Build library's path
   strcat(deblibmultiarch,"/usr/lib/");
   strcat(deblibmultiarch, debhostmultiarch);
   strcat(deblibmultiarch, "/");
}

int main(int argc, char *argv[])
{
	int  irix=0, solaris=0, verbose=0, write_utmp;
	char cflags[BUFSIZ], ldflags[BUFSIZ];
	char line[BUFSIZ];
	FILE *makefile;

	if ( argv[1] && strncmp(argv[1], "-v", 2) == 0 )
		verbose=1;

	if ( exists(".", "Makefile") )
	{
		fprintf(stderr, "Makefile already exists.  Continue?  [y/n] ");
		fgets(line, BUFSIZ-1, stdin);
		if ( line[0] == 'y' )
			printf("Continuing...\n");
		else
		{
			printf("Configuration aborted. Exiting.\n");
			exit(0);
		}
	}

	if ( (makefile=fopen("Makefile", "w")) == NULL )
	{
		perror("Can't create Makefile");
		printf("Configuration aborted. Exiting.\n");
		exit(2);
	}

#ifdef DEBUG
	strcpy(cflags, "-g -DDEBUG");
#else
	cflags[0]='\0';
#endif
	ldflags[0]='\0';

#define INCLUDE "/usr/include/"
#ifdef linux
	printf("Aha!  You are running Linux!\n");
	printf("\tCongratulations on the choice of a GNU generation.\n");

	strcat(cflags, " -O2");
#else

#ifdef NeXT
	printf("NeXTStep!  I'm assuming 3.0 or greater...\n");
	strcat(cflags, " -O2");
#undef INCLUDE
#define INCLUDE "/usr/include/bsd/"
#else
	/* Check for HP-UX */
	if ( exists("/", "hp-ux") ) {
		VERBOSE_PRINT("\tI see you are running HP-UX.\n");
		strcat(cflags, " -DHP_UX");
	} else if ( exists("/usr/lib", "libsocket.a") ) {   /* Solaris 2.1 */
		strcat(ldflags, " -lsocket");
		strcat(cflags, " -g -DSOLARIS");
		solaris=1;
		VERBOSE_PRINT("\tI see you are running Solaris.\n");
	}
	else
#if defined(__GLIBC__)
		VERBOSE_PRINT("\tA bold user of GNU/kFreeBSD or GNU/Hurd.\n");
		strcat(cflags, " -O2");
#else /* !__GLIBC__ */
		strcat(cflags, " -O");
#endif

	/* Check for IRIX */
	if ( grep("/usr/include", "unistd.h", "_getpty") )
	{
		strcat(cflags, " -DIRIX");
		irix=1;
		VERBOSE_PRINT("\tI see you are running IRIX.\n");
	}
#endif /* NeXT */
#endif /* linux */

	/* Check for the termcap library */
        char deblibmultiarch[BUFSIZ];
        finddeblibmultiarch(deblibmultiarch);
	if ( exists(deblibmultiarch, "libtermcap.a") ) {
		strcat(cflags, " -DTERMCAP");
		strcat(ldflags, " -ltermcap");
		VERBOSE_PRINT("\tUsing the termcap library for terminal support.\n");
	}

	/* Check for the ut_host field in the utmp file */
	if ( grep(INCLUDE, "utmp.h", "ut_host") ) {
		strcat(cflags, " -DHAVE_UTHOST");
		strcat(ldflags, " -lutil");
		VERBOSE_PRINT("\tYour utmp file uses the host field.\n");
	}

	/* Check for termio[s].h */
	if ( exists(INCLUDE, "termios.h") ) {
		strcat(cflags, " -DHAVE_TERMIOS_H");
		VERBOSE_PRINT("\tI will use termios tty structures.\n");
	} else if ( exists(INCLUDE, "termio.h") ) {
		strcat(cflags, " -DHAVE_TERMIO_H");
		VERBOSE_PRINT("\tI will use termio tty structures.\n");
	} else 
		VERBOSE_PRINT("\tI will use sgttyb tty structures.\n");

	/* Check for unistd.h */
	if ( exists(INCLUDE, "unistd.h") )
		strcat(cflags, " -DHAVE_UNISTD_H");

	/* Check for BSD tty compatibility. (HP-UX) */
	/* Check for BSD tty compatibility. (HP-UX) */
	if ( exists(INCLUDE, "sys/bsdtty.h") ) {
		strcat(cflags, " -DHAVE_BSDTTY_H");
		VERBOSE_PRINT("\tI see you have BSD tty support.\n");
	}

#if !__GLIBC__	/* False positive for GNU/kFreeBSD */
	/* Check for ioctl compatibility.  (FreeBSD) */
	if ( exists(INCLUDE, "sys/ioctl_compat.h") ) {
		strcat(cflags, " -DNEED_COMPAT_H");
		VERBOSE_PRINT(
			"\tI will use your ioctl compatibility header.\n");
	}
#endif

	/* Check for BSD socket library header (AT&T) */
	if ( exists(INCLUDE, "sys/inet.h") ) {
		strcat(cflags, " -DNEED_INET_H");
		VERBOSE_PRINT("\tI will use your inet compatibility header.\n");
	}

	/* Check for BSD socket library header (AIX) */
	if ( exists(INCLUDE, "sys/select.h") ) {
		strcat(cflags, " -DNEED_SELECT_H");
		VERBOSE_PRINT("\tI will use your sockets definition header.\n");
	}

	/* Check for wait4() (This always seems to be true.) */
	if ( grep(INCLUDE, "sys/wait.h", "wait4") ) {
		strcat(cflags, " -DHAVE_WAIT4");
		VERBOSE_PRINT("\tI will use wait4() instead of waitpid().\n");
	}

#if 0
	if ( exists("/usr/lib", "libnet.a") )	/* Socket routines */
		strcat(ldflags, " -lnet");
	if ( exists("/usr/lib", "libnsl.a") )	/* AT&T socket library */
		strcat(ldflags, " -lnsl");
	if ( exists("/usr/lib", "libnsl_s.a") )	/* AT&T socket library */
		strcat(ldflags, " -lnsl_s");
	if ( exists("/usr/lib", "libsun.a") )	/* IRIX yp routines */
		strcat(ldflags, " -lsun");
#endif

	/* Tell the user what kind of configuration to do */
	if ( (access("/var/run/utmp", (R_OK|W_OK)) == 0) && getuid() )
		write_utmp=1;
	else
		write_utmp=0;

	VERBOSE_PRINT("\n");
	if ( solaris || irix ) {
		if ( write_utmp ) {
			VERBOSE_PRINT(
	"This program doesn't need to be installed set-uid root.\n");
			VERBOSE_PRINT(
	"\nThis program will put entries for its windows in /var/run/utmp.\n");
		} else {
			VERBOSE_PRINT(
	"If installed set-uid root, this program will put entries for its windows\nin /var/run/utmp.\n");
		}
	} else if ( write_utmp ) {
		VERBOSE_PRINT(
	"This program will put entries for its windows in /var/run/utmp.\n");
		VERBOSE_PRINT(
	"\nIf installed set-uid root, this program will change ownership of the\n");
		VERBOSE_PRINT(
	"ttys acquired to the user running this program.\n");
	} else {
		VERBOSE_PRINT(
	"If installed set-uid root, this program will put entries for its windows\n");
		VERBOSE_PRINT(
	"in /var/run/utmp, and will also change ownership of the ttys it acquires to the\n");
		VERBOSE_PRINT(
	"user running this program.\n");
	}
	VERBOSE_PRINT("\n");


	if ( exists("/bin", "csh") )
		strcat(cflags, " -DSHELL=\\\"/bin/csh\\\"");
	else
		strcat(cflags, " -DSHELL=\\\"/bin/sh\\\"");

	fprintf(makefile, 
"# This Makefile has been generated from the Configure script.\n");
	fprintf(makefile, 
"# Shareware copyright 1993, by Sam Lantinga\n\n");
#ifdef linux
	fprintf(makefile, "\nCC = gcc\n");
#endif
	fclose(makefile);
	system("sh scanpty >>Makefile");
	if ( (makefile=fopen("Makefile", "a")) == NULL )
	{
		perror("Can't finish creating Makefile");
		printf("Configuration aborted. Exiting.\n");
		exit(2);
	}
	fprintf(makefile, 
		"PTYOPTS = -DPTYCHAR=$(PTYCHAR) -DHEXDIGIT=$(HEXDIGIT)\n");
	sprintf(line, "\nCFLAGS += -Wall %s $(PTYOPTS)\nLIBS = %s\n", cflags, ldflags);
	fprintf(makefile, "%s", line);
	fprintf(makefile, "OBJS = splitvt.o misc.o utmp.o vt100.o videomem.o terminal.o vttest.o vtmouse.o \\\n");
	fprintf(makefile, "       parserc.o lock.o cut-paste.o\n\n");
	fprintf(makefile, "splitvt: $(OBJS)\n");
#if defined(linux) && !defined(DEBUG)
	fprintf(makefile, "\t$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) \n");
#else
	fprintf(makefile, "\t$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)\n");
#endif
	fprintf(makefile, "\nclean: \n\trm -f *.o core \n");
	fprintf(makefile, "\ndistclean: clean\n\trm -f splitvt Makefile\n");
	fprintf(makefile, "\ninstall: install-man\n");
	fprintf(makefile, "\tmv splitvt /usr/local/bin/splitvt\n");
	fprintf(makefile, "\tmv examples/xsplitvt /usr/local/bin/xsplitvt\n");
	fprintf(makefile, "\ninstall-man:\n");
	fprintf(makefile, "\tcp splitvt.man /usr/local/man/man1/splitvt.1\n");

	fclose(makefile);
	exit(0);
}


/* Yeesh.  I have to write a word grep function.... */

int grep(char *dir, char *file, char *word)
{
	FILE *fp;
	char *wptr, *ptr, buffer[BUFSIZ];

	sprintf(buffer, "%s/%s", dir, file);
	if ( (fp=fopen(buffer, "r")) == NULL )
		return(0);
	ptr=word;

	while ( fgets(buffer, BUFSIZ-1, fp) != NULL ) {
		for ( wptr=buffer; *wptr; ++wptr) {
			if ( *wptr == *ptr ) {
				++ptr;
				if ( *ptr == '\0' ) {
					(void) fclose(fp);
					return(1);
				}
			}
			else
				ptr=word;
		}
	}
	(void) fclose(fp);
	return(0);
}

