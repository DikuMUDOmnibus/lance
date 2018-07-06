/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@hypercube.org)				   *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> /* OLC -- for close read write etc */
#include <stdarg.h> /* printf_to_char */

#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
const	char	echo_off_str	[] = { '\0' };
const	char	echo_on_str	[] = { '\0' };
const	char 	go_ahead_str	[] = { '\0' };
#endif

#if	defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(apollo)
#include <unistd.h>
void	bzero		args( ( char *b, int length ) );
#endif

#if	defined(__hpux)
int	accept		args( ( int s, void *addr, int *addrlen ) );
int	bind		args( ( int s, const void *addr, int addrlen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, void *addr, int *addrlen ) );
int	getsockname	args( ( int s, void *name, int *addrlen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname,
 				const void *optval, int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if	defined(linux)
/* 
    Linux shouldn't need these. If you have a problem compiling, try
    uncommenting these functions.
*/
/*
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	listen		args( ( int s, int backlog ) );
*/

int	close		args( ( int fd ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
/* int	read		args( ( int fd, char *buf, int nbyte ) ); */
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	socket		args( ( int domain, int type, int protocol ) );
/* int	write		args( ( int fd, char *buf, int nbyte ) ); */ /* read,write in unistd.h */
#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
static	long			theKeys	[4];

int	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
extern	int		errno;
#endif

#if	defined(MSDOS)
int	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
int	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );

#if !defined(__SVR4)
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );

#if defined(SYSV)
int setsockopt		args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
#endif
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
char		    wlreason[MAX_INPUT_LENGTH];	/* Reason for wizlocking	*/
bool		    newlock;		/* Game is newlocked		*/
char		    nlreason[MAX_INPUT_LENGTH];	/* Reason for newlocking	*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* time of this pulse */	
bool		    MOBtrigger = TRUE;  /* act() switch                 */


/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( int port ) );
void	init_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );

CHAR_DATA*	check_multi		args( ( CHAR_DATA *ch ) );

/* Needs to be global because of do_copyover */
int port, control;

int main( int argc, char **argv )
{
    struct timeval now_time;
    bool fCopyOver = FALSE;

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time 	= (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 4000;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    exit( 1 );
	}
	
	/* Are we recovering from a copyover? */
 	if (argv[2] && argv[2][0])
 	{
 		fCopyOver = TRUE;
 		control = atoi(argv[3]);
 	}
 	else
 		fCopyOver = FALSE;
	
    }

    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db();
    log_string( "Merc is ready to rock." );
    game_loop_mac_msdos( );
#endif

#if defined(unix)

	if (!fCopyOver)
	    control = init_socket( port );
	    
    boot_db();
    sprintf( log_buf, "ROM is ready to rock on port %d.", port );
    log_string( log_buf );
    
    if (fCopyOver)
    	copyover_recover();

    
    game_loop_unix( control );
    close (control);
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}



#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close(fd);
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close(fd);
	    exit( 1 );
	}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror("Init socket: bind" );
	close(fd);
	exit(1);
    }


    if ( listen( fd, 3 ) < 0 )
    {
	perror("Init socket: listen");
	close(fd);
	exit(1);
    }

    return fd;
}
#endif



#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /*
     * New_descriptor analogue.
     */
    dcon.descriptor	= 0;
    dcon.connected	= CON_ANSI;
    dcon.host		= str_dup( "localhost" );
    dcon.outsize	= 2000;
    dcon.outbuf		= alloc_mem( dcon.outsize );
    dcon.next		= descriptor_list;
    dcon.showstr_head	= NULL;
    dcon.showstr_point	= NULL;
    dcon.pEdit		= NULL;			/* OLC */
    dcon.pString	= NULL;			/* OLC */
    dcon.editor		= 0;			/* OLC */
    descriptor_list	= &dcon;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;
	if ( help_greeting[0] == '.' )
	    write_to_buffer( &dcon, help_greeting+1, 0 );
	else
	    write_to_buffer( &dcon, help_greeting  , 0 );
    }

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    if ( d->character != NULL && d->connected == CON_PLAYING)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if (d->character != NULL && d->character->daze > 0)
		--d->character->daze;

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

	        /* OLC */
	        if ( d->showstr_point )
	            show_string( d, d->incomm );
	        else
	        if ( d->pString )
	            string_add( d->character, d->incomm );
	        else
	            switch ( d->connected )
	            {
	                case CON_PLAYING:
			    if ( !run_olc_editor( d ) )
				substitute_alias( d, d->incomm );
			    break;
	                default:
			    nanny( d, d->incomm );
			    break;
	            }

		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->connected == CON_PLAYING)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    int delta;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character != NULL )
		    dcon.character->timer = 0;
		if ( !read_from_descriptor( &dcon ) )
		{
		    if ( dcon.character != NULL && d->connected == CON_PLAYING)
			save_char_obj( d->character );
		    dcon.outtop	= 0;
		    close_socket( &dcon );
		}
#if defined(MSDOS)
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
		  + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void game_loop_unix( int control )
{
    static struct timeval null_time;
    struct timeval last_time;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !merc_down )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc;

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	    init_descriptor( control );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character && d->connected == CON_PLAYING)
		    save_char_obj( d->character );
		d->outtop	= 0;
		close_socket( d );
	    }
	}

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL && d->connected == CON_PLAYING)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if (d->character != NULL && d->character->daze > 0)
		--d->character->daze;

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

	/* OLC */
	if ( d->showstr_point )
	    show_string( d, d->incomm );
	else
	if ( d->pString )
	    string_add( d->character, d->incomm );
	else
	    switch ( d->connected )
	    {
	        case CON_PLAYING:
		    if ( !run_olc_editor( d ) )
    		        substitute_alias( d, d->incomm );
		    break;
	        default:
		    nanny( d, d->incomm );
		    break;
	    }

		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->connected == CON_PLAYING)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)

void init_descriptor( int control )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    int size;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    dnew = new_descriptor();

    dnew->descriptor	= desc;
    dnew->connected	= CON_ANSI;
    dnew->showstr_head	= NULL;
    dnew->showstr_point = NULL;
    dnew->outsize	= 2000;
    dnew->pEdit		= NULL;			/* OLC */
    dnew->pString	= NULL;			/* OLC */
    dnew->editor	= 0;			/* OLC */
    dnew->outbuf	= alloc_mem( dnew->outsize );

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

	addr = ntohl( sock.sin_addr.s_addr );
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	log_string( log_buf );
	from = gethostbyaddr( (char *) &sock.sin_addr,
	    sizeof(sock.sin_addr), AF_INET );
	dnew->host = str_dup( from ? from->h_name : buf );
    }
	
    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    if ( check_ban(dnew->host,BAN_ALL))
    {
	write_to_descriptor( desc,
	    "Your site has been banned from this mud.\n\r", 0 );
	close( desc );
	free_descriptor(dnew);
	return;
    }
    /*
     * Init descriptor data.
     */
    dnew->next			= descriptor_list;
    descriptor_list		= dnew;

    /*
     * Send the greeting.
     */

    write_to_buffer( dnew, "\n\rDo you want ANSI color? [Y/N] ", 0 );

    return;
}
#endif



void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf );
	/* cut down on wiznet spam when rebooting */
	if ( dclose->connected == CON_PLAYING && !merc_down)
	{
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    wiznet("Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,0);
	    ch->desc = NULL;
	}
	else
	{
	    free_char(dclose->original ? dclose->original : 
		dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_descriptor(dclose);
#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
	int c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	d->inbuf[iStart++] = c;
	if ( iStart > sizeof(d->inbuf) - 10 )
	    break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */

    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if (++d->repeat >= 25 && d->character
	    &&  d->connected == CON_PLAYING)
	    {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		wiznet("Spam spam spam $N spam spam spam spam spam!",
		       d->character,NULL,WIZ_SPAM,0,get_trust(d->character));
		if (d->incomm[0] == '!')
		    wiznet(d->inlast,d->character,NULL,WIZ_SPAM,0,
			get_trust(d->character));
		else
		    wiznet(d->incomm,d->character,NULL,WIZ_SPAM,0,
			get_trust(d->character));

		d->repeat = 0;
/*
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
		strcpy( d->incomm, "quit" );
*/
	    }
	}
    }


    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;

    /*
     * Bust a prompt.
     */
    if ( !merc_down )
    {
	if ( d->showstr_point )
	    write_to_buffer( d, "[Hit Return to continue]\n\r", 0 );
	else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
	    write_to_buffer( d, "> ", 2 );
	else if ( fPrompt && d->connected == CON_PLAYING )
	{
	    CHAR_DATA *ch;
	CHAR_DATA *victim;

	ch = d->character;

        /* battle prompt */
        if ((victim = ch->fighting) != NULL && can_see(ch,victim))
        {
            int percent;
            char wound[100];
            char *pbuff;
	    char buf[MAX_STRING_LENGTH];
            char buffer[MAX_STRING_LENGTH*2];
 
            if (victim->max_hit > 0)
                percent = victim->hit * 100 / victim->max_hit;
            else
                percent = -1;
 
            if (percent >= 100)
                sprintf(wound,"is in excellent condition.");
            else if (percent >= 90)
                sprintf(wound,"has a few scratches.");
            else if (percent >= 75)
                sprintf(wound,"has some small wounds and bruises.");
            else if (percent >= 50)
                sprintf(wound,"has quite a few wounds.");
            else if (percent >= 30)
                sprintf(wound,"has some big nasty wounds and scratches.");
            else if (percent >= 15)
                sprintf(wound,"looks pretty hurt.");
            else if (percent >= 0)
                sprintf(wound,"is in awful condition.");
            else
                sprintf(wound,"is bleeding to death.");
 
	
            sprintf(buf,"%s %s \n\r", 
	            IS_NPC(victim) ? victim->short_descr : victim->name,wound);
	    buf[0] = UPPER(buf[0]);
            pbuff  = buffer;
            colourconv( pbuff, buf, d->character );
            write_to_buffer( d, pbuff, 0);
        }


	ch = d->original ? d->original : d->character;
	if (!IS_SET(ch->comm, COMM_COMPACT) )
	    write_to_buffer( d, "\n\r", 2 );


        if ( IS_SET(ch->comm, COMM_PROMPT) )
            bust_a_prompt( d->character );

	if (IS_SET(ch->comm,COMM_TELNET_GA))
	    write_to_buffer(d,go_ahead_str,0);
	}
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	if (d->character != NULL)
	    write_to_buffer( d->snoop_by, d->character->name,0);
	write_to_buffer( d->snoop_by, "> ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    const char *str;
    const char *i;
    char *point;
    char *pbuff;
    char buffer[ MAX_STRING_LENGTH*2 ];
    char doors[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    const char *dir_name[] = {"N","E","S","W","U","D"};
    int door;
 
    point = buf;
    str = ch->prompt;
    if (str == NULL || str[0] == '\0')
    {
        sprintf( buf, "{p<%dhp %dm %dmv>{x %s",
	    ch->hit,ch->mana,ch->move,ch->prefix);
	send_to_char(buf,ch);
	return;
    }

   if (IS_SET(ch->comm,COMM_AFK))
   {
	send_to_char("{p<AFK>{x ",ch);
	return;
   }

   while( *str != '\0' )
   {
      if( *str != '%' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      switch( *str )
      {
         default :
            i = " "; break;
	case 'l': /* Shows level. */
	    sprintf(buf2, "%d", ch->level);
	    i = buf2; break;
	case 'e':
	    found = FALSE;
	    doors[0] = '\0';
	    for (door = 0; door < 6; door++)
	    {
		if ((pexit = ch->in_room->exit[door]) != NULL
		&&  pexit ->u1.to_room != NULL
		&&  (can_see_room(ch,pexit->u1.to_room)
		||   (IS_AFFECTED(ch,AFF_INFRARED) 
		&&    !IS_AFFECTED(ch,AFF_BLIND))) )
		{
		    char chbuf[0];

		    found = TRUE;
		    
		    
		    if( IS_SET(pexit->exit_info, EX_CLOSED) )
		    {
		    	if( !IS_SET(pexit->exit_info, EX_HIDDEN) )
		    	{
		    		strcpy( chbuf, dir_name[door] );
		    		sprintf( buf3, "%c", tolower(chbuf[0]) );
		    		strcat( doors, buf3 );
		    	}
		    }
		    else
		    {
		    	strcpy( chbuf, dir_name[door] );
		    	sprintf( buf3, "%c", toupper(chbuf[0]) );
		    	strcat( doors, buf3 );
		    }
		}
	    }
	    if (!found)
	 	strcat(buf,"none");
	    sprintf(buf2,"%s",doors);
	    i = buf2; break;
 	 case 'c' :
	    sprintf(buf2,"%s","\n\r");
	    i = buf2; break;
         case 'q' :
            sprintf( buf2, "%d", ch->questpoints );
            i = buf2; break;
         case 'Q' :
            sprintf( buf2, "%d", ch->nextquest );
            i = buf2; break;
         case 'h' :
            sprintf( buf2, "%d", ch->hit );
            i = buf2; break;
         case 'H' :
            sprintf( buf2, "%d", ch->max_hit );
            i = buf2; break;
         case 'm' :
            sprintf( buf2, "%d", ch->mana );
            i = buf2; break;
         case 'M' :
            sprintf( buf2, "%d", ch->max_mana );
            i = buf2; break;
         case 'v' :
            sprintf( buf2, "%d", ch->move );
            i = buf2; break;
         case 'V' :
            sprintf( buf2, "%d", ch->max_move );
            i = buf2; break;
         case 'x' :
            sprintf( buf2, "%d", ch->exp );
            i = buf2; break;
	 case 'X' :
	    sprintf(buf2, "%d", IS_NPC(ch) ? 0 :
	    (ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);
	    i = buf2; break;
         case 'F' :
            sprintf( buf2, "%d", (ch->played + (int)(current_time-ch->logon))/3600 );
            i = buf2; break;
         case 's' :
            sprintf( buf2, "%ld", ch->steel);
            i = buf2; break;
	 case 'g' :
	    sprintf( buf2, "%ld", ch->gold);
	    i = buf2; break;
         case 'a' :
            if( ch->level > 9 )
               sprintf( buf2, "%d", ch->alignment );
            else
               sprintf( buf2, "%s", IS_GOOD(ch) ? "good" : IS_EVIL(ch) ?
                "evil" : "neutral" );
            i = buf2; break;
         case 'r' :
            if( ch->in_room != NULL )
               sprintf( buf2, "%s", 
		((!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT)) ||
		 (!IS_AFFECTED(ch,AFF_BLIND) && !room_is_dark( ch->in_room )))
		? ch->in_room->name : "darkness");
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case 'R' :
            if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
               sprintf( buf2, "%d", ch->in_room->vnum );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case 'z' :
            if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
               sprintf( buf2, "%s", ch->in_room->area->name );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case '%' :
            sprintf( buf2, "%%" );
            i = buf2; break;
         case 'o' :
            if( IS_IMMORTAL( ch ) )
               sprintf( buf2, "%s", olc_ed_name(ch) );
            else
               sprintf( buf2, " " );
            i = buf2; break;
	 case 'O' :
	    if( IS_IMMORTAL( ch ) )
	       sprintf( buf2, "%s", olc_ed_vnum(ch) );
	    else
	       sprintf( buf2, " " );
	    i = buf2; break;
	 case 'w' :
	    if( IS_IMMORTAL( ch ) && ch->invis_level != 0 )
	       sprintf( buf2, "%d", ch->invis_level );
	    else
	       sprintf( buf2, " " );
	    i = buf2; break;
	 case 'i' :
	    if( IS_IMMORTAL( ch ) && ch->incog_level != 0 )
	       sprintf( buf2, "%d", ch->incog_level );
	    else
	       sprintf( buf2, " " );
	    i = buf2; break;
      }
      ++str;
      while( (*point = *i) != '\0' )
         ++point, ++i;
   }
   *point	= '\0';
   pbuff	= buffer;
   colourconv( pbuff, buf, ch );
   send_to_char( "{p", ch );
   write_to_buffer( ch->desc, buffer, 0 );
   send_to_char( "{x", ch );

   if (ch->prefix[0] != '\0')
        write_to_buffer(ch->desc,ch->prefix,0);
   return;
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

        if (d->outsize >= 32000)
	{
	    bug("Buffer overflow. Closing.\n\r",0);
	    close_socket(d);
	    return;
 	}
	outbuf      = alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
    strncpy( d->outbuf + d->outtop, txt, length );
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    } 

    return TRUE;
}



 void logf (char * fmt, ...)
 {
       char buf [2*MSL];
       va_list args;
       va_start (args, fmt);
       vsprintf (buf, fmt, args);
       va_end (args);

       log_string (buf);
 }
             

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    DESCRIPTOR_DATA *d_old, *d_next;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch, *cch;
    char *pwdnew;
    char *p;
    int iClass,race,i,weapon,dice;
    bool fOld;
    bool retry = FALSE;
	bool hairChoice, eyeChoice, heightChoice, sizeChoice;

    while ( isspace(*argument) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;
	
    case CON_ANSI:
        if ( argument[0] == '\0' || UPPER(argument[0]) == 'Y' )
        {
            d->ansi = TRUE;
            send_to_desc( "\n\r{RAnsi enabled!{x\n\r", d );
            d->connected = CON_GET_NAME;
            {
                extern char * help_greeting;
                if ( help_greeting[0] == '.' )
                  send_to_desc( help_greeting+1, d );
                else
                  send_to_desc( help_greeting , d );
            }
            break;
        }

        if (UPPER(argument[0]) == 'N')
        {
            d->ansi = FALSE;
            send_to_desc( "\n\rAnsi disabled!\n\r", d );
            d->connected = CON_GET_NAME;
            {
                extern char * help_greeting;
                if ( help_greeting[0] == '.' )
                  send_to_desc( help_greeting+1, d );
                else
                  send_to_desc( help_greeting, d );
            }
            break;
        }

            write_to_buffer( d, "Do you want ANSI color? [Y/N] ", 0 );
            return;

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	    return;
	}

	fOld = load_char_obj( d, argument );
	ch   = d->character;

	if (IS_SET(ch->act, PLR_DENY))
	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "Character name not approved, tryagain.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	if (check_ban(d->host,BAN_PERMIT) && !IS_SET(ch->act,PLR_PERMIT))
	{
	    write_to_buffer(d,"Your site has been banned from this mud.\n\r",0);
	    close_socket(d);
	    return;
	}

	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_IMMORTAL(ch)) 
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		if(wlreason[0] != '\0')
		{
		    write_to_buffer( d, "The following reason is given: ", 0);
		    write_to_buffer( d, wlreason, 0 );
		}
		close_socket( d );
		return;
	    }
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
 	    if (newlock)
	    {
                write_to_buffer( d, "The game is newlocked.\n\r", 0 );
                if(nlreason[0] != '\0')
                {
                    write_to_buffer( d, "The following reason is given: ", 0);
                    write_to_buffer( d, nlreason, 0 );
                }
                close_socket( d );
                return;
            }

	    if (check_ban(d->host,BAN_NEWBIES))
	    {
		write_to_buffer(d,
		    "New players are not allowed from your site.\n\r",0);
		close_socket(d);
		return;
	    }
	
	    sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_OLD_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    close_socket( d );
	    return;
	}
 
	write_to_buffer( d, echo_on_str, 0 );

	if (check_playing(d,ch->name))
	    return;

	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
	log_string( log_buf );
	wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

        if (ch->desc->ansi)
          SET_BIT(ch->act, PLR_COLOUR);
        else REMOVE_BIT(ch->act, PLR_COLOUR);


	if ( IS_IMMORTAL(ch) )
	{
	    do_function(ch, &do_help, "imotd" );
	    d->connected = CON_READ_IMOTD;
 	}
	else
	{
	    do_function(ch, &do_help, "motd" );
	    d->connected = CON_READ_MOTD;
	}
	break;

/* RT code for breaking link */
 
    case CON_BREAK_CONNECT:
	switch( *argument )
	{
	case 'y' : case 'Y':
            for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
	    {
		d_next = d_old->next;
		if (d_old == d || d_old->character == NULL)
		    continue;

		if (str_cmp(ch->name,d_old->original ?
		    d_old->original->name : d_old->character->name))
		    continue;

		close_socket(d_old);
	    }
	    if (check_reconnect(d,ch->name,TRUE))
	    	return;
	    write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	case 'n' : case 'N':
	    write_to_buffer(d,"Name: ",0);
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d,"Please type Y or N? ",0);
	    break;
	}
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "New character.\n\rGive me a password for %s: %s",
		ch->name, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );
	write_to_buffer(d,"The following races are available:\n\r",0);
	send_to_desc("{D+------------+----+------------------------------------------+{x\n\r",d);
	send_to_desc("{D| {cRace{D       | {gCP{D |            {xBrief Description{D             |\n\r{x",d);
	send_to_desc("{D+------------+----+------------------------------------------+{x\n\r",d);

	for ( race = 1; race <= MAX_PC_RACE; race++ )
	{
	    sprintf( buf, "{D| {c%-10s{D | {g%d{D  | {x%-40s{D |{x\n\r",
	    	capitalize(pc_race_table[race].name),
	    	pc_race_table[race].points,
	    	pc_race_table[race].desc );
	    	
	    send_to_desc(buf, d );
	}
	
	send_to_desc("{D+------------+----+------------------------------------------+{x\n\r\n\r",d);
	write_to_buffer(d,"What is your race (help for more information)? ",0);
	d->connected = CON_GET_NEW_RACE;
	break;

    case CON_GET_NEW_RACE:
    
	one_argument(argument,arg);

	if (!strcmp(arg,"help"))
	{
	    argument = one_argument(argument,arg);
	    if (argument[0] == '\0')
		do_function(ch, &do_help, "race help");
	    else
		do_function(ch, &do_help, argument);
            write_to_buffer(d,
		"What is your race (help for more information)? ",0);
	    break;
  	}

	race = race_lookup(argument);

	if (race == 0 || !race_table[race].pc_race)
	{
	    write_to_buffer(d,"That is not a valid race.\n\r",0);
            write_to_buffer(d,"The following races are available:\n\r",0);
	send_to_desc("{D+------------+----+------------------------------------------+{x\n\r",d);
	send_to_desc("{D| {cRace{D       | {gCP{D |            {xBrief Description{D             |\n\r{x",d);
	send_to_desc("{D+------------+----+------------------------------------------+{x\n\r",d);

	for ( race = 1; race <= MAX_PC_RACE; race++ )
	{
	    sprintf( buf, "{D| {c%-10s{D |  {g%d{D | {x%-40s{D |{x\n\r",
	    	capitalize(pc_race_table[race].name),
	    	pc_race_table[race].points,
	    	pc_race_table[race].desc );
	    	
	    send_to_desc(buf, d );
	}

	send_to_desc("{D+------------+----+------------------------------------------+{x\n\r\n\r",d);
            write_to_buffer(d,
		"What is your race? (help for more information) ",0);
	    break;
	}

        ch->race = race;
	/* initialize stats */
		dice = 0;
	for (i = 0; i < MAX_STATS; i++)
		{
        dice = number_range(pc_race_table[race].stats_min[i], pc_race_table[race].stats_max[i]);
		ch->perm_stat[i] = dice; /*pc_race_table[race].stats[i];*/
		}
	ch->affected_by = ch->affected_by|race_table[race].aff;
	ch->imm_flags	= ch->imm_flags|race_table[race].imm;
	ch->res_flags	= ch->res_flags|race_table[race].res;
	ch->vuln_flags	= ch->vuln_flags|race_table[race].vuln;
	ch->form	= race_table[race].form;
	ch->parts	= race_table[race].parts;
	
	/* add skills */
	for (i = 0; i < 5; i++)
	{
	    if (pc_race_table[race].skills[i] == NULL)
	 	break;
	    group_add(ch,pc_race_table[race].skills[i],FALSE);
	}
	
	if (race == race_lookup("human"))
	    ch->pcdata->learned[gsn_human] = 100;
	else if(race == race_lookup("silvanesti"))
	    ch->pcdata->learned[gsn_silvanesti] = 100;
	else if(race == race_lookup("qualinesti"))
	    ch->pcdata->learned[gsn_qualinesti] = 100;
	else if(race == race_lookup("kagonesti"))
	    ch->pcdata->learned[gsn_kagonesti] = 100;
	else if(race == race_lookup("dimernesti"))
	    ch->pcdata->learned[gsn_dimernesti] = 100;
	else if(race == race_lookup("dargonesti"))
	    ch->pcdata->learned[gsn_dargonesti] = 100;
	else if(race == race_lookup("hylar"))
	    ch->pcdata->learned[gsn_hylar] = 100;
	else if(race == race_lookup("neidar"))
	    ch->pcdata->learned[gsn_neidar] = 100;
	else if(race == race_lookup("aghar"))
	    ch->pcdata->learned[gsn_aghar] = 100;
	else if(race == race_lookup("gnome"))
	    ch->pcdata->learned[gsn_gnome] = 100;
	else if(race == race_lookup("kender"))
	    ch->pcdata->learned[gsn_kender] = 100;
	else if(race == race_lookup("minotaur"))
	    ch->pcdata->learned[gsn_minotaur] = 100;
	else if(race == race_lookup("draconian"))
	    ch->pcdata->learned[gsn_draconian] = 100;
	/* add cost */
	ch->pcdata->points = pc_race_table[race].points;
	ch->size = pc_race_table[race].size;

        write_to_buffer( d, "What is your sex (M/F)? ", 0 );
        d->connected = CON_GET_NEW_SEX;
        break;
        

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    
			    ch->pcdata->true_sex = SEX_MALE;
			    d->connected = CON_GET_PK;
			    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE; 
			    ch->pcdata->true_sex = SEX_FEMALE;
			    d->connected = CON_GET_PK;
			    break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
	    return;
	}

case CON_GET_PK:
	write_to_buffer(d,"Please chose wheather or not to participate in PK\n\r",0);
        switch ( argument[0] )
        {
        case 'y': case 'Y': 
		SET_BIT(ch->act, PLR_PROKILLER);
	    break;
        case 'n': case 'N': 
		REMOVE_BIT(ch->act,PLR_PROKILLER);
		break;
	  default:
		write_to_buffer( d, "Please Choose <Y/N>", 0 );
		return;
		
}
	write_to_buffer(d,"The following classes are available:\n\r",0);
	send_to_desc("{D+---------+--------------------------------------------------+{x\n\r",d);
	send_to_desc("{D| {cClass{D   |               {xBrief Description{D                  |\n\r{x",d);
	send_to_desc("{D+---------+--------------------------------------------------+{x\n\r",d);

	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    if(pc_race_table[ch->race].bit & class_table[iClass].race_restrict)
	        continue;
	        
	    if(class_table[iClass].remort > 0)
	        continue;
	        
	    sprintf( buf, "{D| {c%-7s{D | {x%-48s{D |{x\n\r",
	    capitalize(class_table[iClass].name),
	    class_table[iClass].desc );
	    		    	
	    send_to_desc(buf, d );
	}
	
	send_to_desc("{D+---------+--------------------------------------------------+{x\n\r\n\r",d);
	write_to_buffer(d,"What is your class: ",0);
	d->connected = CON_GET_NEW_CLASS;
	break;

    case CON_GET_NEW_CLASS:
	iClass = class_lookup(argument);

	if ( iClass == -1 )
	{
		retry = TRUE;
	    	write_to_buffer( d, "That's not a valid class.\n\r", 0 );
	}
	if(pc_race_table[ch->race].bit & class_table[iClass].race_restrict
	|| (class_table[ch->class].bit & class_table[iClass].previous_class
	&& ch->pcdata->remorts <= class_table[iClass].remort))
	{
	    retry = TRUE;
	    write_to_buffer( d, "You are not allowed to be that class.\n\r", 0);
	}
	
	if(retry)
	{
		write_to_buffer(d,"The following classes are available:\n\r",0);
		send_to_desc("{D+---------+--------------------------------------------------+{x\n\r",d);
		send_to_desc("{D| {cClass{D   |               {xBrief Description{D                  |\n\r{x",d);
		send_to_desc("{D+---------+--------------------------------------------------+{x\n\r",d);

		for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
		{
	            if(pc_race_table[ch->race].bit & class_table[iClass].race_restrict)
	                continue;
	            
	            if(ch->pcdata->remorts < class_table[iClass].remort)
	                continue;
	                
	            if(ch->pcdata->remorts <= class_table[iClass].remort
	            && class_table[ch->class].bit & class_table[iClass].previous_class)
	                continue;
	            
		    sprintf( buf, "{D| {c%-7s{D | {x%-48s{D |{x\n\r",
		    capitalize(class_table[iClass].name),
		    class_table[iClass].desc );
		    		    	
		    send_to_desc(buf, d );
		}
	
		send_to_desc("{D+---------+--------------------------------------------------+{x\n\r\n\r",d);
		write_to_buffer(d,"What is your class: ",0);
		break;

	}

        ch->class = iClass;

	sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
	log_string( log_buf );
	sprintf(buf, "NEWBIE ALERT! %s the %s sighted.", ch->name, capitalize(race_table[ch->race].name));
//	wiznet("Newbie alert!  $N sighted.",ch,NULL,WIZ_NEWBIE,0,0);
	wiznet(buf, NULL, NULL, WIZ_NEWBIE, 0, 0);
        wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

	write_to_buffer( d, "\n\r", 2 );

	if(ch->race >=3 && ch->race <= 6)
	{
		write_to_buffer( d, "You may be good.\n\r", 0 );
		write_to_buffer( d, "Which alignment (G)? ", 0 );
	}
	else if( ch->race == 13 )
	{
		write_to_buffer( d, "You may be good or neutral.\n\r", 0 );
		write_to_buffer( d, "Which alignment (G/N)? ", 0 );
	}
	else if( ch->race == 15 || ch->race == 11 )
	{
		write_to_buffer( d, "You may be evil.\n\r", 0 );
		write_to_buffer( d, "Which alignment (E)? ", 0 );
	}
	else
	{
		write_to_buffer( d, "You may be good, neutral, or evil.\n\r",0);
		write_to_buffer( d, "Which alignment (G/N/E)? ",0);
	}
	
	d->connected = CON_GET_ALIGNMENT;
	break;
	
case CON_GET_ALIGNMENT:

	strcpy( arg, argument );

	arg[0] = toupper( arg[0] );

	sprintf( buf, "(G/N/E)" );

	if( ch->race >= 2 && ch->race <= 5 && arg[0] != 'G' )
	{
		arg[0] = 'Z';
		sprintf( buf, "(G)" );
	}
	if( ch->race == 12 && arg[0] == 'E' )
	{
		arg[0] = 'Z';
		sprintf( buf, "(G/N)" );
	}
	if( ch->race == 14 && arg[0] != 'E' )
	{
		arg[0] = 'Z';
		sprintf( buf, "(E)" );
	}
	if( ch->race == 11 && arg[0] != 'E' )
	{
		arg[0] = 'Z';
		sprintf( buf, "(E)" );
	}

	switch( arg[0])
	{
	    case 'G' : ch->alignment = 750;  break;
	    case 'N' : ch->alignment = 0;    break;
	    case 'E' : ch->alignment = -750; break;
	    default:
		write_to_buffer( d, "That's not a valid alignment.\n\r" ,0 );
		
		sprintf( buf1, "Which alignment %s? ", buf );
		write_to_buffer( d, buf1 ,0 );
		return;
	}

	write_to_buffer(d,"\n\r",0);

	case	CON_SHOW_GODS:

       write_to_buffer(d,"The following gods are available:\n\r  ",0);
        for ( god = 0; god < MAX_GOD; god++ )
        {
			  if (((god_table[god].pc_evil && ch->alignment == -750)
               || (god_table[god].pc_neutral && ch->alignment == 0)
               || (god_table[god].pc_good && ch->alignment == 750)))
              {
	                write_to_buffer(d,god_table[god].name,0);
	                write_to_buffer(d," ",1);
              }
        }
        write_to_buffer(d,"\n\r",0);
        write_to_buffer(d,"Who do you want to worship (help for more information)? ",0);
        d->connected = CON_GET_GOD;
        break;

    case CON_GET_GOD:
        one_argument(argument,arg);

        if (!strcmp(arg,"help"))
        {
            argument = one_argument(argument,arg);
            if (argument[0] == '\0')
                do_help(ch,"gods");
            else
                do_help(ch,argument);
                write_to_buffer(d,"Who do you want to worship (help for more information)? ",0);
            break;
        }
        god = god_lookup(argument);

   
			if ( god == 0   || god > 22
	|| (!god_table[god].pc_evil		&& ch->alignment == -750)
        || (!god_table[god].pc_neutral		&& ch->alignment == 0	)
        || (!god_table[god].pc_good		&& ch->alignment == 750))
        {
            write_to_buffer(d,"That is not a valid god.\n\r",0);
	        d->connected = CON_SHOW_GODS;
			break;
        }


        ch->god = god;

        write_to_buffer(d,"\n\r",0);

        group_add(ch,"rom basics",FALSE);
        group_add(ch,class_table[ch->class].base_group,FALSE);
        ch->pcdata->learned[gsn_recall] = 50;
        write_to_buffer( d, echo_on_str, 0 );
        write_to_buffer(d,"Which more suits your hair color for your character?\n\rBlonde, Red, Black? [B/R/L]: ",0);
        d->connected = CON_GET_HAIR;
        break;

    case CON_GET_HAIR:
#if defined(unix)
        write_to_buffer( d, "\n\r", 2 );
#endif
        switch ( *argument )
          {
          case 'b': case 'B':
            SET_BIT( ch->pcdata->hair, HAIR_BLOND );
            REMOVE_BIT(ch->pcdata->hair, HAIR_RED);
            REMOVE_BIT(ch->pcdata->hair, HAIR_BLACK);
            hairChoice = TRUE;
            break;
          case 'r': case 'R':
            SET_BIT( ch->pcdata->hair, HAIR_RED );
            REMOVE_BIT(ch->pcdata->hair, HAIR_BLACK);
            REMOVE_BIT(ch->pcdata->hair, HAIR_BLOND);
            hairChoice = TRUE;
            break;
          case 'l': case 'L':
            SET_BIT(ch->pcdata->hair,HAIR_BLACK);
            REMOVE_BIT(ch->pcdata->hair, HAIR_RED);
            REMOVE_BIT(ch->pcdata->hair, HAIR_BLOND);
            hairChoice = TRUE;
            break;
          default:
            write_to_buffer( d, "Please type either B/R/L? ", 0 );
            hairChoice = FALSE;
            break;
          }
        if(!hairChoice)
          break;

        write_to_buffer(d,echo_on_str, 0);
        write_to_buffer(d,"Which eye color do you prefer for your character?\n\rBlue, Green, Brown? [B/G/R]: ",0);
        d->connected = CON_GET_EYE;
        break;

    case CON_GET_EYE:
#if defined(unix)
        write_to_buffer( d, "\n\r", 2 );
#endif
        switch ( *argument )
          {
          case 'b': case 'B':
            SET_BIT( ch->pcdata->eye, EYE_BLUE );
            REMOVE_BIT(ch->pcdata->eye, EYE_GREEN);
            REMOVE_BIT(ch->pcdata->eye, EYE_BROWN);
            eyeChoice = TRUE;
            break;
          case 'g': case 'G':
            SET_BIT( ch->pcdata->eye, EYE_GREEN );
            REMOVE_BIT(ch->pcdata->eye, EYE_BLUE);
            REMOVE_BIT(ch->pcdata->eye, EYE_BROWN);
            eyeChoice = TRUE;
            break;
          case 'r': case 'R':
            SET_BIT(ch->pcdata->eye,EYE_BROWN);
            REMOVE_BIT(ch->pcdata->eye, EYE_BLUE);
            REMOVE_BIT(ch->pcdata->eye, EYE_GREEN);
            eyeChoice = TRUE;
	    break;
          default:
            write_to_buffer( d, "Please type either B/G/R? ", 0 );
            eyeChoice = FALSE;
            break;
          }
        if(!eyeChoice)
          break;


        write_to_buffer(d,echo_on_str, 0);
        write_to_buffer(d,"Which height would best describe your character?\n\rShort, Midget, Average, Tall, Giant? [S/M/A/T/G]: ",0);
        d->connected = CON_GET_HEIGHT;
        break;

    case CON_GET_HEIGHT:
#if defined(unix)
        write_to_buffer( d, "\n\r", 2 );
#endif
        switch ( *argument )
          {
          case 's': case 'S':
            SET_BIT( ch->pcdata->height, HEIGHT_SHORT );
            REMOVE_BIT(ch->pcdata->height, HEIGHT_MIDGET);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_AVERAGE);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_TALL);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_GIANT);
            heightChoice = TRUE;
            break;
          case 'm': case 'M':
            SET_BIT( ch->pcdata->height, HEIGHT_MIDGET );
            REMOVE_BIT(ch->pcdata->height, HEIGHT_SHORT );
            REMOVE_BIT(ch->pcdata->height, HEIGHT_AVERAGE);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_TALL);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_GIANT);
            heightChoice = TRUE;
            break;
          case 'a': case 'A':
            SET_BIT(ch->pcdata->height,HEIGHT_AVERAGE);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_SHORT );
            REMOVE_BIT(ch->pcdata->height, HEIGHT_MIDGET);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_TALL);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_GIANT);
            heightChoice = TRUE;
            break;
          case 't': case 'T':
            SET_BIT(ch->pcdata->height,HEIGHT_TALL);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_SHORT );
            REMOVE_BIT(ch->pcdata->height, HEIGHT_MIDGET);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_AVERAGE);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_GIANT);
            heightChoice = TRUE;
            break;
          case 'g': case 'G':
            SET_BIT(ch->pcdata->height,HEIGHT_GIANT);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_SHORT );
            REMOVE_BIT(ch->pcdata->height, HEIGHT_MIDGET);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_TALL);
            REMOVE_BIT(ch->pcdata->height, HEIGHT_AVERAGE);
            heightChoice = TRUE;
            break;
          default:
            write_to_buffer( d, "Please type either S/M/A/T/G? ", 0 );
            heightChoice = FALSE;
            break;
          }
        if(!heightChoice)
          break; 
        write_to_buffer(d,"Do you wish to customize this character?\n\r",0);
        write_to_buffer(d,"Customization takes time, but allows a wider range of skills and abilities.\n\r",0);
        write_to_buffer(d,"Customize (Y/N)? ",0);
        d->connected = CON_DEFAULT_CHOICE;
        break;



case CON_DEFAULT_CHOICE:
	write_to_buffer(d,"\n\r",2);
        switch ( argument[0] )
        {
        case 'y': case 'Y': 
	    ch->gen_data = new_gen_data();
	    ch->gen_data->points_chosen = ch->pcdata->points;
	    do_function(ch, &do_help, "group header");
	    list_group_costs(ch);
	    write_to_buffer(d,"You already have the following skills:\n\r",0);
	    do_function(ch, &do_skills, "");
	    /*do_function(ch, &do_help, "menu choice");*/
	    send_to_char("Choice (add,drop,list,help)? ", ch);
	    d->connected = CON_GEN_GROUPS;
	    break;
        case 'n': case 'N': 
	    group_add(ch,class_table[ch->class].default_group,TRUE);
            write_to_buffer( d, "\n\r", 2 );
            write_to_buffer(d,
            	"Which hand would you like as your dominant hand? (R/L)",0);
            d->connected = CON_GET_HAND;
            break;
        default:
            write_to_buffer( d, "Please answer (Y/N)? ", 0 );
            return;
        }
	break;

    case CON_PICK_WEAPON:
	write_to_buffer(d,"\n\r",2);
	weapon = weapon_lookup(argument);
	if (weapon == -1 || ch->pcdata->learned[*weapon_table[weapon].gsn] <= 0
	||  skill_table[*weapon_table[weapon].gsn].skill_level[ch->class] != 1)
	{
	    write_to_buffer(d,
		"That's not a valid selection. Choices are:\n\r",0);
            buf[0] = '\0';
            for ( i = 0; weapon_table[i].name != NULL; i++)
                if (ch->pcdata->learned[*weapon_table[i].gsn] > 0
                &&  skill_table[*weapon_table[i].gsn].skill_level[ch->class] == 1 )
                {
                    strcat(buf,weapon_table[i].name);
		    strcat(buf," ");
                }
            strcat(buf,"\n\rYour choice? ");
            write_to_buffer(d,buf,0);
	    return;
	}

	ch->pcdata->learned[*weapon_table[weapon].gsn] = 40;
	write_to_buffer(d,"\n\r",2);
        if (ch->desc->ansi)
          SET_BIT(ch->act, PLR_COLOUR);
        else REMOVE_BIT(ch->act, PLR_COLOUR);
	do_function(ch, &do_help, "motd");
	d->connected = CON_READ_MOTD;
	break;

    case CON_GEN_GROUPS:
	send_to_char("\n\r",ch);

       	if (!str_cmp(argument,"done"))
       	{
	    if (ch->pcdata->points == pc_race_table[ch->race].points)
	    {
	        send_to_char("You didn't pick anything.\n\r",ch);
		break;
	    }

	    if (ch->pcdata->points < 40 + pc_race_table[ch->race].points)
	    {
		sprintf(buf,
		    "You must take at least %d points of skills and groups",
		    40 + pc_race_table[ch->race].points);
		send_to_char(buf, ch);
		break;
	    }

	    sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
	    send_to_char(buf,ch);
	    sprintf(buf,"Experience per level: %d\n\r",
	            exp_per_level(ch,ch->gen_data->points_chosen));
	    if (ch->pcdata->points < 40)
		ch->train = (40 - ch->pcdata->points + 1) / 2;
	    free_gen_data(ch->gen_data);
	    ch->gen_data = NULL;
	    send_to_char(buf,ch);
            write_to_buffer( d, "\n\r", 2 );
            write_to_buffer(d,
            	"Which hand would you like as your dominant hand? (R/L)",0);
            d->connected = CON_GET_HAND;
            break;
        }

        if (!parse_gen_groups(ch,argument))
        send_to_char(
        "Choices are: list,learned,premise,add,drop,info,help, and done.\n\r"
        ,ch);

        /*do_function(ch, &do_help, "menu choice");*/
        send_to_char("Choice (add,drop,list,help)? ", ch);
        break;

    case CON_GET_HAND:
    	write_to_buffer(d, "\n\r", 2);
    	d->connected = CON_PICK_WEAPON;
    	switch(argument[0])
    	{
    	case 'r': case 'R':
    	    ch->pcdata->learned[gsn_right_hand] = 85;
    	    break;
    	case 'l': case 'L':
    	    ch->pcdata->learned[gsn_left_hand] = 85;
    	    break;
    	default:
    	    write_to_buffer(d,"Please choose R or L: ",0);
    	    d->connected = CON_GET_HAND;
    	    break;
    	}
    	
    	if(d->connected == CON_GET_HAND)
    	    break;
    	
        write_to_buffer(d,
            "Please pick a weapon from the following choices:\n\r",0);
        buf[0] = '\0';
        for ( i = 0; weapon_table[i].name != NULL; i++)
            if (ch->pcdata->learned[*weapon_table[i].gsn] > 0
            &&  skill_table[*weapon_table[i].gsn].skill_level[ch->class] == 1 )
            {
                strcat(buf,weapon_table[i].name);
                strcat(buf," ");
            }
        strcat(buf,"\n\rYour choice? ");
        write_to_buffer(d,buf,0);
        break;
    case CON_READ_IMOTD:
	write_to_buffer(d,"\n\r",2);
        do_function(ch, &do_help, "motd");
        d->connected = CON_READ_MOTD;
	break;

    case CON_READ_MOTD:
        if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
        {
            write_to_buffer( d, "Warning! Null password!\n\r",0 );
            write_to_buffer( d, "Please report old password with bug.\n\r",0);
            write_to_buffer( d,
                "Type 'password null <new password>' to fix.\n\r",0);
        }

	write_to_buffer( d, 
    "\n\rWelcome to ROM 2.4.  Please do not feed the mobiles.\n\r",
	    0 );
	ch->next	= char_list;
	char_list	= ch;
	d->connected	= CON_PLAYING;
	reset_char(ch);

	if ( ch->level == 0 )
	{

	    ch->perm_stat[class_table[ch->class].attr_prime] += 3;

	    ch->level	= 1;
	    ch->exp	= exp_per_level(ch,ch->pcdata->points);
	    ch->hit	= ch->max_hit;
	    ch->mana	= ch->max_mana;
	    ch->move	= ch->max_move;
	    ch->train	 = 3;
	    ch->practice = 5;
	    
	    sprintf( buf, "the %s",
		title_table [ch->class] [ch->level]
		[ch->sex == SEX_FEMALE ? 1 : 0] );
	    set_title( ch, buf );

	    do_function (ch, &do_outfit,"");
/*	    obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP),0),ch);*/

	    char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	    send_to_char("\n\r",ch);
	    do_function(ch, &do_help, "newbie info");
	    send_to_char("\n\r",ch);
	}
	else if ( ch->in_room != NULL )
	{
	    char_to_room( ch, ch->in_room );
	}
	else if ( IS_IMMORTAL(ch) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
	else
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}
	
/*	if( get_room_index( ch->recall ) == NULL )
	{
		if( is_clan(ch) )
			ch->recall = clan_table[ch->clan].recall;
		else
			ch->recall = god_table[ch->god].recall;
	}
*/
	if(IS_IMMORTAL(ch))
	{
	    do_function(ch, &do_mudstatus, "");
	    send_to_char("\n\r", ch);
	}
	
	act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
	do_function(ch, &do_look, "auto" );
	
	sprintf(buf, "$N has left real life behind. {r[{RRoom %d{r]{x", ch->in_room->vnum);
	if(ch->pcdata->orig_host[0] == '\0')
	{
	    ch->pcdata->orig_host = str_dup(ch->desc->host);
	}
        ch->pcdata->fngr_lasthost = str_dup(ch->desc->host);
	wiznet(buf,ch,NULL,WIZ_LOGINS,0,get_trust(ch));
	if(IS_IMMORTAL(ch) && (ch->invis_level > 0 || ch->incog_level > 0))     
        {     
            sprintf(buf, "Wizi: {c%4d{g Incog: {c%4d{x", ch->invis_level, ch->incog_level);
            wiznet(buf, ch, NULL, WIZ_LOGINS, 0, get_trust(ch));
        }     
	if((cch = check_multi(ch)) != NULL && !IS_IMMORTAL(ch) && !IS_IMMORTAL(cch))
	{
	    sprintf(buf, "%s is connected multiple times.", ch->name);
	    wiznet(buf, NULL, NULL, WIZ_MULTI, 0, 0);
	    sprintf(buf, "Host %s shared with %s", ch->desc->host, cch->name);
	    wiznet(buf, NULL, NULL, WIZ_MULTI, 0, 0);
	}

	if (ch->pet != NULL)
	{
	    char_to_room(ch->pet,ch->in_room);
	    act("$n has entered the game.",ch->pet,NULL,NULL,TO_ROOM);
	}
	
	if( ch->pet == NULL && (IS_SET( ch->act, PLR_MOUNTED ) || ch->position == POS_RIDING) )
	{
	    REMOVE_BIT( ch->act, PLR_MOUNTED );
	    ch->position = POS_STANDING;
	}
	    

	do_function(ch, &do_unread, "");
//	web_update();
	break;
    }

    return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    int clan;



    if (check_illegal_name( name )==TRUE)
	return FALSE;


    /*
     * Reserved words.
     */
    if (is_exact_name(name,
	"all auto immortal self someone something the you loner none set"))
    {
	return FALSE;
    }

    /* check clans */
    for (clan = 0; clan < MAX_CLAN; clan++)
    {
	if (LOWER(name[0]) == LOWER(clan_table[clan].name[0])
	&&  !str_cmp(name,clan_table[clan].name))
	   return FALSE;
    }
	
    if (str_cmp(capitalize(name),"Alander") && (!str_prefix("Alan",name)
    || !str_suffix("Alander",name)))
	return FALSE;

    /*
     * Length restrictions.
     */
     
    if ( strlen(name) <  2 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
 	int total_caps = 0;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;

	    if ( isupper(*pc)) /* ugly anti-caps hack */
	    {
		if (adjcaps)
		    cleancaps = TRUE;
		total_caps++;
		adjcaps = TRUE;
	    }
	    else
		adjcaps = FALSE;

	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;

	if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		send_to_char(
		    "Reconnecting. Type replay to see missed tells.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );

		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
		wiznet("$N groks the fullness of $S link.",
		    ch,NULL,WIZ_LINKS,0,0);
		d->connected = CON_PLAYING;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "That character is already playing.\n\r",0);
	    write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}



/*
 * Write to one char.
 */
void send_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
        write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

/*
 * Send a page to one char.
 */
void page_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt == NULL || ch->desc == NULL)
	return;

    if (ch->lines == 0 )
    {
	send_to_char_bw(txt,ch);
	return;
    }
	
#if defined(macintosh)
	send_to_char_bw(txt,ch);
#else
    ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
    strcpy(ch->desc->showstr_head,txt);
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string(ch->desc,"");
#endif
}

/*
 * Page to one char, new colour version, by Lope.
 */
 void send_to_char( const char *txt, CHAR_DATA *ch )
 {
     const	char 	*point;
     		char 	*point2;
     		char 	buf[ MAX_STRING_LENGTH*4 ];
 		int	skip = 0;
 
     buf[0] = '\0';
     point2 = buf;
     if( txt && ch->desc )
 	{
 	    if( IS_SET( ch->act, PLR_COLOUR ) )
 	    {
 		for( point = txt ; *point ; point++ )
 	        {
 		    if( *point == '{' )
 		    {
 			point++;
 			skip = colour( *point, ch, point2 );
 			while( skip-- > 0 )
 			    ++point2;
 			continue;
 		    }
 		    *point2 = *point;
 		    *++point2 = '\0';
 		}			
 		*point2 = '\0';
         	write_to_buffer( ch->desc, buf, point2 - buf );
 	    }
 	    else
 	    {
 		for( point = txt ; *point ; point++ )
 	        {
 		    if( *point == '{' )
 		    {
 			point++;
 			continue;
 		    }
 		    *point2 = *point;
 		    *++point2 = '\0';
 		}
 		*point2 = '\0';
         	write_to_buffer( ch->desc, buf, point2 - buf );
 	    }
 	}
     return;
 }


/*
 * Page to descriptor, new colour version, by Lope.
 */
 void send_to_desc( const char *txt, DESCRIPTOR_DATA *d )
 {
     const	char 	*point;
     		char 	*point2;
     		char 	buf[ MAX_STRING_LENGTH*4 ];
 		int	skip = 0;
 
     buf[0] = '\0';
     point2 = buf;
     if( txt && d )
 	{
 		for( point = txt ; *point ; point++ )
 	        {
 		    if( *point == '{' )
 		    {
 			point++;
 			skip = d_colour( *point, point2 );
 			while( skip-- > 0 )
 			    ++point2;
 			continue;
 		    }
 		    *point2 = *point;
 		    *++point2 = '\0';
 		}			
 		*point2 = '\0';
         	write_to_buffer( d, buf, point2 - buf );
 	}
     return;
 }



void page_to_char( const char *txt, CHAR_DATA *ch )
{
    const	char	*point;
    		char	*point2;
    		char	buf[ MAX_STRING_LENGTH * 4 ];
		int	skip = 0;

#if defined(macintosh)
    send_to_char( txt, ch );
#else
    buf[0] = '\0';
    point2 = buf;
    if( txt && ch->desc )
	{
	    if( IS_SET( ch->act, PLR_COLOUR ) )
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			skip = colour( *point, ch, point2 );
			while( skip-- > 0 )
			    ++point2;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}			
		*point2 = '\0';
		ch->desc->showstr_head  = alloc_mem( strlen( buf ) + 1 );
		strcpy( ch->desc->showstr_head, buf );
		ch->desc->showstr_point = ch->desc->showstr_head;
		show_string( ch->desc, "" );
	    }
	    else
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}
		*point2 = '\0';
		ch->desc->showstr_head  = alloc_mem( strlen( buf ) + 1 );
		strcpy( ch->desc->showstr_head, buf );
		ch->desc->showstr_point = ch->desc->showstr_head;
		show_string( ch->desc, "" );
	    }
	}
#endif
    return;
}

/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
    char buffer[4*MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument(input,buf);
    if (buf[0] != '\0')
    {
	if (d->showstr_head)
	{
	    free_mem(d->showstr_head,strlen(d->showstr_head));
	    d->showstr_head = 0;
	}
    	d->showstr_point  = 0;
	return;
    }

    if (d->character)
	show_lines = d->character->lines;
    else
	show_lines = 0;

    for (scan = buffer; ; scan++, d->showstr_point++)
    {
	if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
	    && (toggle = -toggle) < 0)
	    lines++;

	else if (!*scan || (show_lines > 0 && lines >= show_lines))
	{
	    *scan = '\0';
	    write_to_buffer(d,buffer,strlen(buffer));
	    for (chk = d->showstr_point; isspace(*chk); chk++);
	    {
		if (!*chk)
		{
		    if (d->showstr_head)
        	    {
            		free_mem(d->showstr_head,strlen(d->showstr_head));
            		d->showstr_head = 0;
        	    }
        	    d->showstr_point  = 0;
    		}
	    }
	    return;
	}
    }
    return;
}
	

/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
    	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void act_new( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type, int min_pos)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
     char              *pbuff;
char              buffer[ MAX_STRING_LENGTH*2 ];
 bool              fColour = FALSE;                    

 
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
        if ( vch == NULL )
        {
            bug( "Act: null vch with TO_VICT.", 0 );
            return;
        }

	if (vch->in_room == NULL)
	    return;

        to = vch->in_room->people;
    }
 
    for ( ; to != NULL; to = to->next_in_room )
    {
	if ( (!IS_NPC(to) && to->desc == NULL )
	||   ( IS_NPC(to) && !HAS_TRIGGER(to, TRIG_ACT) )
	||    to->position < min_pos )
            continue;
 
        if ( (type == TO_CHAR) && to != ch )
            continue;
        if ( type == TO_VICT && ( to != vch || to == ch ) )
            continue;
        if ( type == TO_ROOM && to == ch )
            continue;
        if ( type == TO_NOTVICT && (to == ch || to == vch) )
            continue;
 
        point   = buf;
        str     = format;
        while ( *str != '\0' )
        {
            if ( *str != '$' )
            {
                *point++ = *str++;
                continue;
            }
 fColour = TRUE;      
            ++str;
  i = " <@@@> "; 

            if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
            {
                bug( "Act: missing arg2 for code %d.", *str );
                i = " <@@@> ";
            }
            else
            {
                switch ( *str )
                {
                default:  bug( "Act: bad code %d.", *str );
                          i = " <@@@> ";                                break;
                /* Thx alex for 't' idea */
                case 't': i = (char *) arg1;                            break;
                case 'T': i = (char *) arg2;                            break;
                case 'n': i = PERS( ch,  to  );                         break;
                case 'N': i = PERS( vch, to  );                         break;
                case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];        break;
                case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];        break;
                case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];        break;
                case 'M': i = him_her [URANGE(0, vch ->sex, 2)];        break;
                case 's': i = his_her [URANGE(0, ch  ->sex, 2)];        break;
                case 'S': i = his_her [URANGE(0, vch ->sex, 2)];        break;
		case 'g': i = god_table[ch->god].name;			break; 

                case 'p':
                    i = can_see_obj( to, obj1 )
                            ? obj1->short_descr
                            : "something";
                    break;
 
                case 'P':
                    i = can_see_obj( to, obj2 )
                            ? obj2->short_descr
                            : "something";
                    break;
 
                case 'd':
                    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
                    {
                        i = "door";
                    }
                    else
                    {
                        one_argument( (char *) arg2, fname );
                        i = fname;
                    }
                    break;
                }
            }
 
            ++str;
            while ( ( *point = *i ) != '\0' )
                ++point, ++i;
        }
 
        *point++ = '\n';
        *point++ = '\r';
	*point   = '\0';
pbuff    = buffer; 
 colourconv( pbuff, buf, to );
        buf[0]   = UPPER(buf[0]);
	if ( to->desc != NULL )/*
        write_to_buffer( to->desc, buf, point - buf );*/
 write_to_buffer( to->desc, buffer, 0 );            
	else
	if ( MOBtrigger )
	    mp_act_trigger( buf, to, ch, arg1, arg2, TRIG_ACT );
    }
    return;
}


 int colour( char type, CHAR_DATA *ch, char *string )
 {
     PC_DATA	*col;
     char	code[ 20 ];
     char	*p = '\0';
 
     if( IS_NPC( ch ) )
 	return( 0 );
 
     col = ch->pcdata;
 
     switch( type )
     {
 	default:
 	    strcpy( code, CLEAR );
 	    break;
 	case 'x':
 	    strcpy( code, CLEAR );
 	    break;
 	case 'b':
 	    strcpy( code, C_BLUE );
 	    break;
 	case 'c':
 	    strcpy( code, C_CYAN );
 	    break;
 	case 'g':
 	    strcpy( code, C_GREEN );
 	    break;
 	case 'm':
 	    strcpy( code, C_MAGENTA );
 	    break;
 	case 'r':
 	    strcpy( code, C_RED );
 	    break;
 	case 'w':
 	    strcpy( code, C_WHITE );
 	    break;
 	case 'y':
 	    strcpy( code, C_YELLOW );
 	    break;
 	case 'B':
 	    strcpy( code, C_B_BLUE );
 	    break;
 	case 'C':
 	    strcpy( code, C_B_CYAN );
 	    break;
 	case 'G':
 	    strcpy( code, C_B_GREEN );
 	    break;
 	case 'M':
 	    strcpy( code, C_B_MAGENTA );
 	    break;
 	case 'R':
 	    strcpy( code, C_B_RED );
 	    break;
 	case 'W':
 	    strcpy( code, C_B_WHITE );
 	    break;
 	case 'Y':
 	    strcpy( code, C_B_YELLOW );
 	    break;
 	case 'D':
 	    strcpy( code, C_D_GREY );
 	    break;
 	case '*':
 	    sprintf( code, "%c", '\a' );
 	    break;
 	case '/':
 	    strcpy( code, "\n\r" );
 	    break;
 	case '-':
 	    sprintf( code, "%c", '~' );
 	    break;
 	case '{':
 	    sprintf( code, "%c", '{' );
 	    break;
     }
 
     p = code;
     while( *p != '\0' )
     {
 	*string = *p++;
 	*++string = '\0';
     }
 
     return( strlen( code ) );
 }


 int d_colour( char type, char *string )
 {
     char	code[ 20 ];
     char	*p = '\0';
 
     switch( type )
     {
 	default:
 	    strcpy( code, CLEAR );
 	    break;
 	case 'x':
 	    strcpy( code, CLEAR );
 	    break;
 	case 'b':
 	    strcpy( code, C_BLUE );
 	    break;
 	case 'c':
 	    strcpy( code, C_CYAN );
 	    break;
 	case 'g':
 	    strcpy( code, C_GREEN );
 	    break;
 	case 'm':
 	    strcpy( code, C_MAGENTA );
 	    break;
 	case 'r':
 	    strcpy( code, C_RED );
 	    break;
 	case 'w':
 	    strcpy( code, C_WHITE );
 	    break;
 	case 'y':
 	    strcpy( code, C_YELLOW );
 	    break;
 	case 'B':
 	    strcpy( code, C_B_BLUE );
 	    break;
 	case 'C':
 	    strcpy( code, C_B_CYAN );
 	    break;
 	case 'G':
 	    strcpy( code, C_B_GREEN );
 	    break;
 	case 'M':
 	    strcpy( code, C_B_MAGENTA );
 	    break;
 	case 'R':
 	    strcpy( code, C_B_RED );
 	    break;
 	case 'W':
 	    strcpy( code, C_B_WHITE );
 	    break;
 	case 'Y':
 	    strcpy( code, C_B_YELLOW );
 	    break;
 	case 'D':
 	    strcpy( code, C_D_GREY );
 	    break;
 	case '*':
 	    sprintf( code, "%c", '\a' );
 	    break;
 	case '/':
 	    strcpy( code, "\n\r" );
 	    break;
 	case '-':
 	    sprintf( code, "%c", '~' );
 	    break;
 	case '{':
 	    sprintf( code, "%c", '{' );
 	    break;
     }
 
     p = code;
     while( *p != '\0' )
     {
 	*string = *p++;
 	*++string = '\0';
     }
 
     return( strlen( code ) );
 }


 void colourconv( char *buffer, const char *txt, CHAR_DATA *ch )
 {
     const	char	*point;
 		int	skip = 0;
 
     if( ch->desc && txt )
     {
 	if( IS_SET( ch->act, PLR_COLOUR ) )
 	{
 	    for( point = txt ; *point ; point++ )
 	    {
 		if( *point == '{' )
 		{
 		    point++;
 		    skip = colour( *point, ch, buffer );
 		    while( skip-- > 0 )
 			++buffer;
 		    continue;
 		}
 		*buffer = *point;
 		*++buffer = '\0';
 	    }			
 	    *buffer = '\0';
 	}
 	else
 	{
 	    for( point = txt ; *point ; point++ )
 	    {
 		if( *point == '{' )
 		{
 		    point++;
 		    continue;
 		}
 		*buffer = *point;
 		*++buffer = '\0';
 	    }
 	    *buffer = '\0';
 	}
     }
     return;
 }
 


/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

/* source: EOD, by John Booth <???> */

void printf_to_char (CHAR_DATA *ch, char *fmt, ...)
{
	char buf [MAX_STRING_LENGTH];
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);
	
	send_to_char (buf, ch);
}

void bugf (char * fmt, ...)
{
	char buf [2*MSL];
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	bug (buf, 0);
}

CHAR_DATA* check_multi (CHAR_DATA *ch)
{
    DESCRIPTOR_DATA *d;
    
    for(d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *dch;
    	if(d == ch->desc)
    	    continue;
    	
    	dch = d->character ? d->character : d->original;
    	if(!str_cmp(ch->desc->host, d->host))
    	    return dch;
    }
    return NULL;
}

#define ILLEGAL_NAME_FILE	"illegalnames.txt"

bool check_illegal_name( char *name )
{
    char strsave[MIL], nameread[MSL];
    FILE *fp;
    fclose( fpReserve );
    sprintf( strsave, "%s", ILLEGAL_NAME_FILE );
    if ( (fp = fopen( strsave, "r" ) ) != NULL )
    {
        for ( ; ; )
	{
            fscanf (fp, " %s", nameread);
            if ( !str_cmp( nameread, "END" ) )
                break;  
            else if (is_name(name,nameread))
		return TRUE;
        }
    }
    else
	fp = fopen( NULL_FILE, "r" );
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );  
    return FALSE;  
}
