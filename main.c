#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXPENDING	100				/*	maximum outstanding connection requests	*/

static void print_sock_addr( const struct sockaddr *addr, FILE *stream );

static int setup_tcp_server_socket( void );

int main( int argc, char *argv[] )
{
	int server_socket;

	if ( (server_socket = setup_tcp_server_socket()) < 0 )
	{
		return EXIT_FAILURE;
	}

	while ( 1 )
	{

	}

	close( server_socket );

	return EXIT_SUCCESS;
}

void print_sock_addr( const struct sockaddr *addr, FILE *stream )
{
	char		buf[INET6_ADDRSTRLEN];
	void		*naddr;
	in_port_t	port;

	if ( !addr || !stream ) return;

	switch ( addr->sa_family )
	{
	case AF_INET:
		naddr = &((struct sockaddr_in *) addr)->sin_addr;
		port = ntohs( ((struct sockaddr_in *) addr)->sin_port );
		break;
	case AF_INET6:
		naddr = &((struct sockaddr_in6 *) addr)->sin6_addr;
		port = ntohs( ((struct sockaddr_in6 *) addr)->sin6_port );
		break;
	default:
		fprintf( stream, "[unknown type]" );
		return;
		break;
	}

	if ( inet_ntop( addr->sa_family, naddr, buf, sizeof(buf) ) )
	{
		fprintf( stream, "%s", buf );
		if ( port ) fprintf( stream, "-%u", port );
	}
	else
	{
		fprintf( stream, "[invalid address]" );
	}
}

int setup_tcp_server_socket( void )
{
	int retval, server_socket;
	struct addrinfo *addr, hints, *result;

	memset( &hints, 0, sizeof(hints) );	/*	some fields need to be 0 or NULL	*/
	hints.ai_family = AF_UNSPEC;		/*	any address family (IPv4, IPv6, …	*/
	hints.ai_socktype = SOCK_STREAM;	/*	TCP uses stream socket				*/
	hints.ai_protocol = IPPROTO_TCP;	/*	we only want tcp					*/
	hints.ai_flags = AI_PASSIVE;		/*	suitable for bind and accept		*/

	if ( (retval = getaddrinfo( NULL, "echo", &hints, &result )) )
	{
		fprintf( stderr, "getaddrinfo() failed: %s\n", gai_strerror( retval ) );
		return -1;
	}

	/*
	 *	getaddrinfo() returns a list of address structures.
	 *	Try each address until we successfully bind(2).
	 *	If socket(2) (or bind(2)) fails, we (close the socket and) try
	 *	the next address.
	 */
	for ( addr = result; addr != NULL; addr = addr->ai_next )
	{
		if ( (server_socket = socket( addr->ai_family, addr->ai_socktype,
				addr->ai_protocol )) < 0 )
		{
			/*	Socket creation failed, try next address …	*/
			continue;
		}

		if ( (bind( server_socket, addr->ai_addr, addr->ai_addrlen ) == 0)
				&& (listen( server_socket, MAXPENDING ) == 0) )
		{
			/*	Successful bind and listen!	*/
			fprintf( stdout, "Bound to and listening on " );
			print_sock_addr( addr->ai_addr, stdout );
			fprintf( stdout, "\n" );
			break;
		}

		fprintf( stderr, "Could not bind to/listen on " );
		print_sock_addr( addr->ai_addr, stderr );
		fprintf( stderr, ": %s\n", strerror( errno ) );

		close( server_socket );
	}

    if ( addr == NULL )
	{
		fprintf( stderr, "Could not bind/listen!\n" );
		server_socket = -1;
	}

	freeaddrinfo( result );

	return server_socket;
}
