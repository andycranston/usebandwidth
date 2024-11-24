/*
 *  @(!--#) @(#) userbandwidth.c, version 001, 24-november-2024
 *
 *  use out outgoing bandwidth by sending dummy UDP packages to a specified UDP IPv4 address and port number
 *
 */

/*****************************************************************************/

/*
 *  includes
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/*****************************************************************************/

/*
 *  defines
 */

#define PORTBASE 55555

#define MAX_IPv4_ADDRESS_LENGTH 15

#define FAILSAFE_MAX 256

#define MESSAGE_TEXT "This UDP packet sent by program fillarp by Andy Cranston (andy@cranstonhub.com) - https://github.com/andycranston/fillarp"

/*****************************************************************************/

/*
 *  globals
 */

char *progname;

/*****************************************************************************/

void usage()
{
  fprintf(stderr, "%s: not enough arguments\n", progname);
  fprintf(stderr, "\tusage:\n");
  fprintf(stderr, "\t\t%s basenetwork startsubnet endsubnet\n", progname);
  fprintf(stderr, "\texample:\n");
  fprintf(stderr, "\t\t%s 192.168.1 1 254\n", progname);

  return;
}

/*****************************************************************************/

char *basename(filespec)
  char *filespec;
{
  char *bname;

  bname = filespec;

  while (*filespec != '\0') {
    if (*filespec == '/') {
      bname = filespec + 1;
    }

    filespec++;
  }

  return (bname);
}

/*****************************************************************************/

int main(argc, argv)
  int   argc;
  char *argv[];
{
  char     *ipv4_address;
  int       dest_port_number;
  int       count;
  int       sock;
  struct    sockaddr_in   destaddress;
  socklen_t addresssize;
  char     *basenetwork;
  int       startsubnet;
  int       endsubnet;
  char      ipaddress[MAX_IPv4_ADDRESS_LENGTH+sizeof(char)];
  int       i;
  int       failsafecounter;
  int       messagetextlength;
  int       bytes_sent;

  progname = basename(argv[0]);

  if (argc < 3) {
    usage();
    exit(1);
  }

  ipv4_address = argv[1];
  dest_port_number = atoi(argv[2]);

  count = 10;

  if (argc > 3) {
    count = atoi(argv[3]);
  }

  printf("IPv4=%s   Port=%d   Count=%d\n", ipv4_address, dest_port_number, count);

  messagetextlength = strlen(MESSAGE_TEXT);

  sock = socket(PF_INET, SOCK_DGRAM, 0);

  for (i = 0; i < count; i++) {
    printf("i=%d\n", i);

    destaddress.sin_family      = AF_INET;
    destaddress.sin_port        = htons(dest_port_number);
    destaddress.sin_addr.s_addr = inet_addr(ipv4_address);

    memset(destaddress.sin_zero, '\0', sizeof(destaddress.sin_zero));

    addresssize = sizeof(destaddress);

    bytes_sent = sendto(sock, MESSAGE_TEXT, messagetextlength, 0, (struct sockaddr *)&destaddress, addresssize);

    if (bytes_sent == -1) {
      perror("sendto has failed");
    } else {
      if (bytes_sent != messagetextlength) {
        fprintf(stderr, "%s: mismatch between bytes sent and size of package (%d != %d)\n", progname, bytes_sent, messagetextlength);
      }
    }
  }

  close(sock);

  exit(0);
}

/**********************************************************************/
