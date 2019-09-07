/* A lightweight Memory usage tracking script
 *
 * This script essentially reads /proc/<PID>/status file periodically
 * to track memory usage by a tool.
 *
 */

#include <stdint.h>
#include <stdarg.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <signal.h>
#include <time.h>

#include "Stringlib.h"

#ifndef WAIT_TIME_IN_SEC
#define WAIT_TIME_IN_SEC 10
#endif

#define ASSERT __Script_Assert
#define WARN   __Script_Warn

/* We use this function to emulate assert() in the C library. We have to handle
 * exceptions based on conditions ourselves since all the other functions use
 * heap and functions assert().
 */
void __Script_Assert(bool condition, char *errorMessage) {
/* If the condition is false, we handle the error, or else we're done */
	if(!condition) {
	/* Print the error message */
		__Debug_Printf(errorMessage);

	/* Now, we raise SIGABRT signal to the application */
		raise(SIGABRT);
	}
}

int8_t __Script_Warn(bool condition, char *errorMessage) {
/* If the condition is false, we handle the error, or else we're done */
	if(!condition) {
	/* Print the error message */
		__Debug_Printf(errorMessage);
		return 1;
	}
	return 0;
}

uint64_t Read_StatusProcFile(int64_t pid) {
	char filepath[1024] = {0};
	__Safe_Sprintf(filepath, 1024, "/proc/%d/status", pid);

/* Proc files should only be read */
	int fd = open(filepath, O_RDONLY);
	ASSERT(fd != -1, "Error: Could not open /proc/PID/status file\n");

/* Read the file */
	char buffer[1024] = {0};
	char readChar;
	uint64_t lineNum = 0;
	char *bufPtr = buffer;
	while(read(fd, (void *)&readChar, 1) > 0) {
		if(readChar == '\n') {
			*bufPtr = '\0';

		/* We look for VmHWM field */
			if(__Safe_Strstr(buffer, "VmHWM")) {
			/* Iterate through the string till we hit a number */
				char *charPtr = buffer;
				while(!(*charPtr >= 48 && *charPtr <= 57))
					charPtr++;

			/* Now we try to it the first non-numeric character following the number */
				char *ptr = charPtr;
				while(*ptr >= 48 && *ptr <= 57)
					ptr++;

				*ptr = '\0';
				uint64_t highWaterMark = __Safe_Atoi(charPtr);
				ASSERT(highWaterMark != (int64_t)SAFE_ATOI_ERROR,
								"ERROR IN READING /PROC/PID/STATUS FILE.\n");
				return highWaterMark;
			}

			bufPtr = buffer;
			continue;
		}
		*bufPtr = readChar;
		bufPtr++;
	}

	ASSERT(!close(fd), "Error: Failed to close proc file,\n");
	return 0;  /* Just keep the compiler happy */
}

int main(int argc, char **argv) {
/* The process id of the program to be tracked has to be specified in the
 * command line before running this script.
 */
	ASSERT(argc >= 2, "Error: No PID specified\n");

/* Get the PID */
	int64_t pid = (int64_t)__Safe_Atoi(argv[1]);
	ASSERT(pid != (int64_t)SAFE_ATOI_ERROR, "Error: Invalid PID\n");

	int64_t waitTimeSec = (int64_t)WAIT_TIME_IN_SEC;
	if(argc == 3) {
		waitTimeSec = (int64_t)__Safe_Atoi(argv[2]);
		if(WARN(waitTimeSec != (int64_t)SAFE_ATOI_ERROR,
				"Second command-line argument not recognized\n")) {
		/* Set the timer as specified */
			waitTimeSec =  (int64_t)WAIT_TIME_IN_SEC;
		}
	}

/* Wait time has to be set in milliseconds */
	waitTimeSec *= 100;

/* Now we've got to start a timer. We use a signal alarm for this */
	uint64_t maxMemInUse = 0; /* In kilobytes */
	__Debug_Printf("The amount of memory consumed will be printed as it changes with time.\n");
	while(1) {
	/* Read proc file */
		uint64_t readMemInUse = Read_StatusProcFile(pid);
		if(readMemInUse > maxMemInUse) {
			maxMemInUse = readMemInUse;
			__Debug_Printf("Memory in use: %d kB\n", maxMemInUse);
		}

	/* Wait a while */
		int msec = 0;
		clock_t before = clock();
		while(waitTimeSec > msec) {
			clock_t difference = clock() - before;
			msec = difference * 1000 / CLOCKS_PER_SEC;
		}
	}

	return 0;
}
