/* Copyright (c) 2016 Matt Barlow
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

/* deepfocus.h
*/

#ifndef DEEPFOCUS_H
#define DEEPFOCUS_H

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>		// Provides open() function.
#include <time.h>		// Provides time() function.
#include <sys/stat.h>	// Provides fstat() function.
#include <limits.h>		// PATH_MAX

const bool START = true;
const bool STOP = false;

typedef enum {
	E_INPUT,
	E_STOP,
	E_RUNNING,
	E_USAGE,
	E_ZEROBYTE
} errstate;

const char *errmsgs[4];
errstate dferror;

struct fsession {
	char name[64];
	char status[64];
	time_t starttime;
	time_t endtime;
};

void sysError(const char *msg);
void appError(errstate dferror);
int sessionName(int fd, char namebuffer[]);
int sessionStatus(int fd, char statusbuffer[]);
static int isRunning(int fd);
static int startTask(char *session);
static int stopTask(void);
static char FOCUSFILE[PATH_MAX];

#endif
