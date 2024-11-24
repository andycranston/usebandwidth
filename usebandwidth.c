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

#define MIN_PACKET_SIZE 10

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
	char		*ipv4_address;
	int		dest_port_number;
	int		count;
	int		packet_size;
	int		arg;
	unsigned char	*packet;
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

	ipv4_address = NULL;
	dest_port_number = 55555;
	count = 10;
	packet_size = 1000;

	arg = 1;

	while (arg < argc) {
		if (strcmp(argv[arg], "-i") == 0) {
			arg++;

			if (arg >= argc) {
				fprintf(stderr, "%s: expected IPv4 destination address after -i command line option\n", progname);
				exit(2);
			}

			ipv4_address = argv[arg];
		} else if (strcmp(argv[arg], "-p") == 0) {
			arg++;

			if (arg >= argc) {
				fprintf(stderr, "%s: expected UDP port number after -p command line option\n", progname);
				exit(2);
			}

			dest_port_number = atoi(argv[arg]);
		} else if (strcmp(argv[arg], "-c") == 0) {
			arg++;

			if (arg >= argc) {
				fprintf(stderr, "%s: expected packet count after -c command line option\n", progname);
				exit(2);
			}

			count = atoi(argv[arg]);
		} else if (strcmp(argv[arg], "-s") == 0) {
			arg++;

			if (arg >= argc) {
				fprintf(stderr, "%s: expected packet size after -s command line option\n", progname);
				exit(2);
			}

			packet_size = atoi(argv[arg]);
		} else {
			fprintf(stderr, "%s: unrecognised command line argument \"%s\"\n", progname, argv[arg]);
			exit(2);
		}

		arg++;
	}

	if (ipv4_address == NULL) {
		fprintf(stderr, "%s: must specify a destination IPv4 address with the -i command line option\n", progname);
		exit(2);
	}

	if (packet_size < MIN_PACKET_SIZE) {
		fprintf(stderr, "%s: packet size of %d specified with command line option is less than minimum allowed value of %d\n", progname, packet_size, MIN_PACKET_SIZE);
		exit(2);
	}

	if ((packet = malloc((size_t)packet_size)) == NULL) {
		fprintf(stderr, "%s: unable to allocate a packet buffer of %d bytes\n", progname, packet_size);
		exit(2);
	}

	for (i = 0; i < packet_size; i++) {
		if (i < MIN_PACKET_SIZE) {
			packet[i] = '0';
		} else {
			packet[i] = (char)('A' + (i % 26));
		}
	}

	for (i = 0; i < packet_size; i++) {
		putchar(packet[i]);
	}
	putchar('\n');

	exit(0);





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
