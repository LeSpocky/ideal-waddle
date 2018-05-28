/*
 *	Copyright 2018 Alexander Dahl
 *
 *	SPDX-License-Identifier: GPL-3.0-or-later
 *	License-Filename: LICENSE
 */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFSIZE		1500
#define MAXPENDING	100				/*	maximum outstanding connection requests	*/

#define CLIENT_THREAD_KEEPALIVE	(1 << 0)

/**
 *	Structure to pass to client thread as argument.
 */
struct thread_args {
	int	client_sock;
	int	flags;
};

static int accept_tcp_connect( int server_sock );

static void handle_tcp_client( int client_sock );

static void print_sock_addr( FILE *stream, const struct sockaddr *addr );

static void print_usage( FILE * stream, const char *appname );

static int setup_tcp_server_sock( void );

static void *thread_client( void *arg );

int main( int argc, char *argv[] )
{
	int client_flags = 0, opt, server_sock;

	while ( (opt = getopt( argc, argv, "hk" )) != -1 )
	{
		switch ( opt )
		{
			case 'h':
				print_usage( stdout, argv[0] );
				return EXIT_SUCCESS;
				break;
			case 'k':
				client_flags |= CLIENT_THREAD_KEEPALIVE;
				break;
			default:
				print_usage( stderr, argv[0] );
				return EXIT_FAILURE;
				break;
		}
	}

	/*	setup server socket	*/
	if ( (server_sock = setup_tcp_server_sock()) < 0 )
	{
		return EXIT_FAILURE;
	}

	/*	enter endless loop	*/
	while ( 1 )
	{
		int client_sock = accept_tcp_connect( server_sock );
		if ( client_sock < 0 ) continue;

		struct thread_args *ta =
				(struct thread_args *) malloc( sizeof(struct thread_args) );
		if ( ta == NULL )
		{
			fprintf( stderr, "malloc() failed!\n" );
			break;
		}

		ta->client_sock = client_sock;

		pthread_t tid;
		int retval = pthread_create( &tid, NULL, thread_client, ta );
		if ( retval != 0 )
		{
			fprintf( stderr, "pthread_create() failed: %s\n",
					strerror(retval) );
			break;
		}

		printf( "thread (%lu) handling client socket %i ...\n",
				tid, client_sock );
	}

	close( server_sock );

	return EXIT_SUCCESS;
}

int accept_tcp_connect( int server_sock )
{
	int client_sock;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	client_sock = accept( server_sock, (struct sockaddr *) &client_addr,
			&client_addr_len );

	if ( client_sock < 0 )
	{
		fprintf( stderr, "accept() failed: %s\n", strerror( errno ) );
		return client_sock;
	}

	fprintf( stdout, "Accepted connection from " );
	print_sock_addr( stdout, (struct sockaddr *) &client_addr );
	fprintf( stdout, " ...\n" );

	return client_sock;
}

void handle_tcp_client( int client_sock )
{
	printf( "Handling client socket fd %i ...\n", client_sock );

	while ( 1 )
	{
		char buf[BUFSIZE];

		ssize_t bytes_received = recv( client_sock, buf, sizeof(buf), 0 );

		if ( bytes_received < 0 )
		{
			fprintf( stderr, "Error receiving on fd %i: %s\n",
					client_sock, strerror( errno ) );
			break;
		}

		if ( bytes_received > 0 )
		{
			/*	Sent things back!	*/
			ssize_t bytes_sent = send( client_sock, buf, bytes_received, 0 );

			if ( bytes_sent < 0 )
			{
				fprintf( stderr, "Error sending on fd %i: %s\n",
						client_sock, strerror( errno ) );
				break;
			}

			if ( bytes_sent != bytes_received )
			{
				fprintf( stderr,
						"On fd %i sent %zi bytes, but received %zi!\n",
						client_sock, bytes_sent, bytes_received );
			}
		}
		else	/*	== 0	*/
		{
			printf( "Peer shutdown on fd %i\n", client_sock );
			break;
		}
	}

	close( client_sock );
}

void print_sock_addr( FILE *stream, const struct sockaddr *addr )
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

void print_usage( FILE * stream, const char *appname )
{
	fprintf( stream, "Usage: %s [-k]\n", appname );
}

int setup_tcp_server_sock( void )
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
			print_sock_addr( stdout, addr->ai_addr );
			fprintf( stdout, "\n" );
			break;
		}

		fprintf( stderr, "Could not bind to/listen on " );
		print_sock_addr( stderr, addr->ai_addr );
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

void *thread_client( void *arg )
{
	char tn[16];
	pthread_t tid = pthread_self();

	pthread_detach( tid );

	int client_sock = ((struct thread_args *) arg)->client_sock;
	free( arg );

	snprintf( tn, sizeof(tn), "client fd %i", client_sock );
	(void) pthread_setname_np( tid, tn );

	handle_tcp_client( client_sock );

	return NULL;
}
