/*
 modttl.cpp
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

#include "modttl.h"

/*
 Main function - start program here.
*/
int main(int argc, char** argv)
{
    struct modttldata ourdata;
    char str[45];

    // Open the console for syslog()
    openlog(argv[0], LOG_CONS | LOG_PID, LOG_DAEMON);
    // Check commandline args
    checkccargs(argc, argv, &ourdata);

    // Make sure euid is root
    if (geteuid() != 0) {
        fprintf(stderr, "You must run this as root - use sudo\n");
        exit(1);
    }

    // Signals so we can quit gracefully
    signal(SIGHUP, sigquitproc);
    signal(SIGINT, sigquitproc);
    signal(SIGKILL, sigquitproc);
    signal(SIGTERM, sigquitproc);
    signal(SIGQUIT, sigquitproc);
    
    // Create and bind the socket
    makesocket(&ourdata);
    
    //Setup global pointer to data
    gmodttl = &ourdata;

    // Throw the app into the background
    if (fork() > 0) {
        exit(0);
    }

    // Make sure per-thread stack is big enough for threads
    pthread_attr_init(&(ourdata.pattr));
    pthread_attr_getstacksize(&(ourdata.pattr), &(ourdata.ssize));

    if (ourdata.ssize < BUFSIZE + 1024) {
        ourdata.ssize = BUFSIZE + 1024;
        pthread_attr_setstacksize(&(ourdata.pattr), ourdata.ssize);
    }

    // Create the pthreads
    pthread_create(&(ourdata.handlepacketsid), &(ourdata.pattr), handlepackets, (void *)&ourdata);

    // Join the pthreads (this is when the work begins)
    pthread_join(ourdata.handlepacketsid, NULL);

    syslog(LOG_NOTICE, "Program Terminated");
    closelog();

    return 0;
}

/*
 Function to check commandline args
*/
void checkccargs(int argc, char** argv, struct modttldata *ourdata)
{
    //initialize variables to be used in getopt
    int c;
    ourdata->bindport = 17780;
    ourdata->thettl = 128;

    //actual getopt call, this should be straightfoward
    while ((c = getopt(argc, argv, "r:hd:t:")) != EOF) {
       switch (c) {
           case 'h':
             usage(argv[0]);
             exit(1);
             break;
           case 'd':
             ourdata->bindport = atoi(optarg);
             if (ourdata->bindport <= 0 || ourdata->bindport > 65535) {
                fprintf(stderr, "Divert port must be between 1 and 65535... Exiting...\n");
                exit(1);
             }
             break;
           case 't':
             ourdata->thettl = atoi(optarg);
             if (ourdata->thettl <= 0 || ourdata->thettl > 255) {
                fprintf(stderr, "TTL must be between 1 and 255... Exiting...\n");
                exit(1);
             }
             break;
            case 'r':
             rulenum = atoi(optarg);
             if (rulenum <= 0 || rulenum > 65535) {
                fprintf(stderr, "IPFW rule number must be between 1 and 65535... Exiting...\n");
                exit(1);
             }
             break;
           default:
             usage(argv[0]);
             exit(1); 
       }
    }

    // IPFW rule number is required, test for it here.
    if (rulenum == 0) {
       usage(argv[0]);
       exit(1);
    }

    // Display divert port that is used
    fprintf(stderr, "Using divert port %i\n", ourdata->bindport);
    // Display the TTL we are modifying packets to
    fprintf(stderr, "TTL of packets will be set to %i\n", ourdata->thettl);
}

/*
 Function for displaying program usage
*/
void usage(char *appname) {
   fprintf(stderr,
       "usage: %s -r rule [-t TTL] [-h] [-d port]\n"
       "-r rule\t\tIPFW rule number to remove when quit (required)\n"
       "-t TTL\t\tTTL of packet\n"
       "-d port\t\tDivert port (optional)\n"
       "-h\t\tThis help screen\n", appname);
}

/*
 Function to delete rules on exit.
*/
void deleterules()
{
        char buffer[30];
        int n;
        n = sprintf(buffer, "/sbin/ipfw del %i", rulenum);
        system(buffer);
}

/*
 Function to create and bind socket
*/
void makesocket (struct modttldata *ourdata)
{
    // Creating a raw divert socket
    fprintf(stderr, "Creating a socket\n");
    ourdata->sockid = socket(AF_INET, SOCK_RAW, IPPROTO_DIVERT);

    // Make sure create didn't error out
    if (ourdata->sockid == -1) {
        fprintf(stderr, "Failure creating a divert socket\n");
        exit(1);
    }

    // Setup for binding the socket
    ourdata->sockport.sin_family = AF_INET;
    ourdata->sockport.sin_port = htons(ourdata->bindport);
    ourdata->sockport.sin_addr.s_addr = 0;
    memset(&(ourdata->sockport.sin_zero), '\0', 8);

    // Going to need the size of sockaddr_in for handlepackets
    ourdata->sockaddrsize = sizeof(struct sockaddr_in);

    fprintf(stderr, "Binding a socket\n");
    ourdata->bindid = bind(ourdata->sockid, (struct sockaddr*)&(ourdata->sockport), ourdata->sockaddrsize);

    // Make sure bind didn't error out
    if (ourdata->bindid != 0) {
        close (ourdata->sockid);
        fprintf(stderr, "Error binding socket to port %i - Error %s", ourdata->bindport, strerror(ourdata->bindid));
        exit(2);
    }

    // set application priority to the value specified in modttl.h
    setpriority(PRIO_PROCESS, 0, NICEVALUE);
    fprintf(stderr, "Priority has been set to %i\n", getpriority(PRIO_PROCESS, 0));

    fprintf(stderr, "Waiting for data...\n");
}

/*
 Allows app to quit gracefully on quit signal
 */
void sigquitproc(int signal)
{
    keeplooping = FALSE;
    deleterules();
}

/*
 Function to recieve the packets
 *WARNING: This is in it's own thread*
 */
void* handlepackets(void *dataarg)
{
    struct modttldata *ourdata = (struct modttldata *)dataarg;
    unsigned char packet[BUFSIZE];
    char *packettmp;
    struct sockaddr_in sendtowho;
    fd_set selectpoll, t_selectpoll;
    struct timeval s_timeout;
    long sizerecv, destip;

    // Initialize variables for loop
    FD_ZERO(&selectpoll);
    FD_SET(ourdata->sockid, &selectpoll);
    t_selectpoll = selectpoll;
    s_timeout.tv_sec = 5;
    s_timeout.tv_usec = 0;

    while (keeplooping) {
        if (select((ourdata->sockid)+1, &t_selectpoll, NULL, NULL, &s_timeout) > 0) {
            sizerecv = recvfrom(ourdata->sockid, &packet, BUFSIZE, 0, (struct sockaddr *)&sendtowho, &(ourdata->sockaddrsize));
            ((tcpipfullheader*)packet)->ipheader.ip_ttl = ourdata->thettl;
            sendto(ourdata->sockid, packet, sizerecv, 0, (struct sockaddr*)&sendtowho, ourdata->sockaddrsize);
        } else {
            t_selectpoll = selectpoll;
            s_timeout.tv_sec = 5;
            s_timeout.tv_usec = 0;
        }
    }

    return NULL;
}
