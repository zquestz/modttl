/*
 modttl.h
 Copyright (C) 2006 quest

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define MODTTL_VERSION "0.1.5"

// Global includes
#include <stdio.h>
#ifdef __FreeBSD__
#include <sys/types.h>
#endif
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/resource.h>
#include <err.h>
#include <errno.h>
#include <sysexits.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/socket.h>

// Definitions for compiler variables
#define BUFSIZE 65535
#define TRUE 1
#define FALSE 0

// Define your application priority (-20 through 20)
#define NICEVALUE -15

// Global variables
char keeplooping = TRUE;
long rulenum = 0;

// I don't know where to find this otherwise... the full tcpip header info
struct tcpipfullheader {
    struct ip		ipheader;
    struct tcphdr	tcpheader;
};

// Putting all our data in a struct so it can be easily located
struct modttldata
{
    int sockid, bindid;
    socklen_t sockaddrsize;
    struct sockaddr_in sockport;
    struct in_addr addr;
    int bindport, thettl;
    pthread_t handlepacketsid;
    pthread_attr_t pattr;
    size_t ssize;
    FILE * console;
};

//Global pointer to modttldata
modttldata *gmodttl;

// Defining our functions
void checkccargs(int argc, char** argv, struct modttldata *ourdata);
void makesocket(struct modttldata *ourdata);
void deleterules();
void sigquitproc(int signal);
void usage(char *appname);

// Functions for pthreads
void* handlepackets(void *dataarg);
