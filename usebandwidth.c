/*
 *  @(!--#) @(#) userbandwidth.c, version 005, 24-november-2024
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
#include <time.h>
#include <errno.h>

/*****************************************************************************/

/*
 *  defines
 */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define PORTBASE 55555

#define MIN_PACKET_SIZE 10

#define MAX_IPv4_ADDRESS_LENGTH 15

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
	char			*ipv4_address;
	int			dest_port_number;
	int			packet_size;
	int			count;
	int			arg;
	char			*packet;
	time_t			pausesecond;
	long			pausenanosecond;
	struct timespec		pause;
	int			pauseflag;
	int			sock;
	struct sockaddr_in	destaddress;
	socklen_t		addresssize;
	int			i;
	int			bytes_sent;
	int			nanosleep_retcode;

	progname = basename(argv[0]);

	ipv4_address = NULL;
	dest_port_number = 55555;
	packet_size = 1000;
	count = 10;
	pausesecond = (time_t)0;
	pausenanosecond = (long)1000000;

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
		} else if (strcmp(argv[arg], "-s") == 0) {
			arg++;

			if (arg >= argc) {
				fprintf(stderr, "%s: expected packet size after -s command line option\n", progname);
				exit(2);
			}

			packet_size = atoi(argv[arg]);
		} else if (strcmp(argv[arg], "-c") == 0) {
			arg++;

			if (arg >= argc) {
				fprintf(stderr, "%s: expected packet count after -c command line option\n", progname);
				exit(2);
			}

			count = atoi(argv[arg]);
		} else if (strcmp(argv[arg], "-n") == 0) {
			arg++;

			if (arg >= argc) {
				fprintf(stderr, "%s: expected nanosecond pause value after -n command line option\n", progname);
				exit(2);
			}

			pausenanosecond = atol(argv[arg]);
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

	if (pausenanosecond > 999999999L) {
		fprintf(stderr, "%s: pause nanosecond value %ld exceeds 999,999,999\n", progname, pausenanosecond);
		exit(2);
	}

	pause.tv_sec = pausesecond;
	pause.tv_nsec = pausenanosecond;

	printf("IPv4 address: %s\n", ipv4_address);
	printf("Destination UDP port: %d\n", dest_port_number);
	printf("Packet size: %d\n", packet_size);
	printf("Count: %d\n", count);
	printf("Nanosecond: %ld\n", pausenanosecond);

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

	sock = socket(PF_INET, SOCK_DGRAM, 0);

	if (sock == -1) {
		fprintf(stderr, "%s: error trying to create socket\n", progname);
		exit(2);
	}

	if (pausenanosecond != 0L) {
		pauseflag = TRUE;
	} else {
		pauseflag = FALSE;
	}

	for (i = 0; i < count; i++) {
		printf("i=%d\n", i);

		destaddress.sin_family      = AF_INET;
		destaddress.sin_port        = htons(dest_port_number);
		destaddress.sin_addr.s_addr = inet_addr(ipv4_address);

		memset(destaddress.sin_zero, '\0', sizeof(destaddress.sin_zero));

		addresssize = sizeof(destaddress);

		bytes_sent = sendto(sock, packet, packet_size, 0, (struct sockaddr *)&destaddress, addresssize);

		if (bytes_sent == -1) {
			perror("sendto has failed");
		} else {
			if (bytes_sent != packet_size) {
				fprintf(stderr, "%s: mismatch between bytes sent and size of package (%d != %d)\n", progname, bytes_sent, packet_size);
			}
		}

		if (pauseflag) {
			printf("Pausing...\n");
			printf("Sec: %ld\n", pause.tv_sec);
			printf("Nan: %ld\n", pause.tv_nsec);
			nanosleep_retcode = nanosleep(&pause, NULL);
			printf("Retcode: %d\n", nanosleep_retcode);
			if (nanosleep_retcode == -1) {
				perror("nanosleep had an error");
			}
		}
	}

	close(sock);

	exit(0);
}

/**********************************************************************/
