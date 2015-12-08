#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/*
 *++
 * NAME
 *    v4d 8
 * SUMMARY
 *    Wait for device
 * SYNOPSIS
 *    w4d *[-d* *dev]* *[-i* *ms]* *[-n* *tries]* _[-v]_ _[-V]_ _[-h]_
 * DESCRIPTION
 *    *w4d* is a trivial tool which checks and waits for a device to 
 *    become available. This can be used, for example, when waiting for
 *    an USB webcam to become available after the USB kernel modules have
 *    been loaded.
 * OPTIONS
 *    * *-d* _dev_
 *      Device to monitor
 *    * *-i* _ms_
 *      Check intervla time in ms.
 *    * *-n* _tries_
 *      Number of times to test (0 for wait forever)
 *    * *-v*
 *      Be verbose
 *    * *-V*
 *      Show version and exit
 *    * *-h*
 *      Usage
 * EXAMPLE
 *
 *    : $ w4d -d /dev/video0 -i 250 -n 10
 *
 *    Check 4 times a second for 2.5 seconds if device =/dev/video0=
 *    comes available.
 * HOME PAGE
 *    http://www.vanheusden.com/w4d/
 *--
 */

void usage(void)
{
	fprintf(stderr, "-d dev	device to check\n");
	fprintf(stderr, "-i x	check interval (in ms)\n");
	fprintf(stderr, "-n x	number of tests (or 0 for forever)\n");
	fprintf(stderr, "-v	be verbose\n");
	fprintf(stderr, "-V	show version and exit\n");
	fprintf(stderr, "-h	this help\n");
}

void version(void)
{
	fprintf(stderr, "waitfordevice " VERSION ", (C) 2005 by folkert@vanheusden.com\n\n");
}

int main(int argc, char *argv[])
{
	char *dev = NULL;
	int check_interval = 100;
	int number_of_checks = 0;
	int verbose = 0;
	int c;

	while((c = getopt(argc, argv, "d:i:n:vVh")) != -1)
        {
                switch(c)
                {
			case 'd':
				dev = optarg;
				break;

			case 'i':
				check_interval = atoi(optarg);
				if (check_interval < 0)
				{
					fprintf(stderr, "Error: check interval (-i) must be >= 0\n");
					return 1;
				}
				break;

			case 'n':
				number_of_checks = atoi(optarg);
				if (number_of_checks < 0)
				{
					fprintf(stderr, "Error: number of checks (-n) must be >= 0\n");
					return 1;
				}
				break;

			case 'v':
				verbose++;
				break;

			case 'V':
				version();
				return 0;

			case 'h':
				usage();
				return 0;

			default:
				usage();
				return 1;
		}
	}

	if (!dev)
	{
		fprintf(stderr, "Error: no device selected\n");
		usage();
		return 1;
	}

	for(;;)
	{
		int fd = open(dev, O_RDONLY);
		if (fd != -1)
		{
			if (verbose) printf("Opening device %s success\n", dev);

			return 0;
		}

		if (errno != ENODEV && errno != ENXIO)
		{
			fprintf(stderr, "Unexpected error while opening device %s: %s (%d)\n", dev, strerror(errno), errno);
			return 1;
		}

		if (verbose > 1) printf("Device not available (yet).\n");

		if (number_of_checks > 0)
		{
			if (--number_of_checks == 0)
			{
				if (verbose) printf("Failed opening device %s\n", dev);

				return 1;
			}
		}

		usleep(check_interval * 1000);
	}

	return 0;
}
