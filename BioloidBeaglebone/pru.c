/** @file pru.c
 *  @brief Functions for initializing, loading, starting, and stopping a PRU.
 *
 *  These are the functions that: initialize the PRU subsystem, load a program
 *  compiled with the PRU C compiler into the program memory of a given PRU,
 *  initiate execution of a program in a given PRU, and terminate execution of
 *  a program in a given PRU.
 *
 *  @author Bill Merryman
 *  @bug No known bugs.
 *
 *  Created on: Sep 8, 2015
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <err.h>
#include <errno.h>

#include "pru.h"
#include "fileParse.h"

void *pruExternalMemoryVirtual;

PRU_INTEROP_0_DATA *PRUInterop0DataVirtual;
PRU_INTEROP_1_DATA *PRUInterop1DataVirtual;

PRU_INTEROP_0_DATA *getPRUInterop0Data()
{
	return PRUInterop0DataVirtual;
}

PRU_INTEROP_1_DATA *getPRUInterop1Data()
{
	return PRUInterop1DataVirtual;
}

void initializePRU(const char *PRU_0_Firmware, const char *PRU_1_Firmware)
{
	configurePRU_0(PRU_0_Firmware);
	configurePRU_1(PRU_1_Firmware);
	startPRU_0();
	startPRU_1();

	PRUInterop0DataVirtual = parseFile("/sys/kernel/debug/remoteproc/remoteproc1/resource_table");
	PRUInterop1DataVirtual = parseFile("/sys/kernel/debug/remoteproc/remoteproc2/resource_table");

}

void configurePRU_0(const char *pruFirmware)
{
	int fd;

	fd = open("/sys/class/remoteproc/remoteproc1/firmware", O_RDWR);
	write(fd, pruFirmware, strlen(pruFirmware));
	close(fd);
}

void startPRU_0()
{
	char *start = "start";
	int fd;

	fd = open("/sys/class/remoteproc/remoteproc1/state", O_RDWR);
	write(fd, start, strlen(start));
	close(fd);
}

void stopPRU_0()
{
	char *stop = "stop";
	int fd;

	fd = open("/sys/class/remoteproc/remoteproc1/state", O_RDWR);
	write(fd, stop, strlen(stop));
	close(fd);
}

void configurePRU_1(const char *pruFirmware)
{
	int fd;

	fd = open("/sys/class/remoteproc/remoteproc2/firmware", O_RDWR);
	write(fd, pruFirmware, strlen(pruFirmware));
	close(fd);
}

void startPRU_1()
{
	char *start = "start";
	int fd;

	fd = open("/sys/class/remoteproc/remoteproc2/state", O_RDWR);
	write(fd, start, strlen(start));
	close(fd);
}

void stopPRU_1()
{
	char *stop = "stop";
	int fd;

	fd = open("/sys/class/remoteproc/remoteproc2/state", O_RDWR);
	write(fd, stop, strlen(stop));
	close(fd);
}
