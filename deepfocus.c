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

/* DISCLAIMER: This was created for personal use. Contributions and criticisms
 * are welcome.
 */

/* deepfocus.c
 * This program is essentially a time tracker. Run it before beginning a
 * session of deeply focussed work. Data is stored in a database inside the
 * directory ~/.deepfocus. Each field in the database is 64B in size, and there
 * is a 1B seperator between fields.
 *
 * The format of the file written out is:
 * session-name|status|starttime|enddtime
 * Where 'status' is either: RUNNING or STOPPED.
 *
 * Start a new session:
 * deepfocus -n session-name
 *
 * Stop a session (only one session can run at a time):
 * deepfocus -s
 *
 * TODO: Add reporting.
 */

#include "deepfocus.h"

int main(int argc, char *argv[])
{
	sprintf(FOCUSFILE, "%s/.deepfocus/focusfile", getenv("HOME"));

	errmsgs[E_INPUT] = "Error receiving CLI input\n";
	errmsgs[E_STOP] = "No task to stop\n";
	errmsgs[E_RUNNING] = "Task is already running\n";
	errmsgs[E_USAGE] = "CLI Usage Error\n";
	errmsgs[E_ZEROBYTE] = "Tried to write zero byte file\n";

	int opt;
	char sessionName[64] = "";
	const char *nstr;
	bool op;

	while ((opt = getopt(argc, argv, "n:s")) != -1) {
		switch (opt) {
		case 'n':
			nstr = strdup(optarg);
			op = START;
			break;
		case 's':
			op = STOP;
			break;
		default:
			appError(E_USAGE);
		}
	}

	if (op) {
		if (strncpy(sessionName, nstr, 64) < 0)
			appError(E_INPUT);	
		if (startTask(sessionName))
			appError(E_RUNNING);
	} else {
		if (stopTask())
			appError(E_STOP);
	}
	exit(EXIT_SUCCESS);
}

int sessionName(int fd, char namebuffer[])
{
	struct stat st;
	if (fstat(fd, &st))
		sysError("fstat");

	if (!st.st_size)
		appError(E_ZEROBYTE);

	// TODO: Switch to atomic seek and read.
	if (lseek(fd, -260, SEEK_END) == -1)
		sysError("lseek");
	if (read(fd, namebuffer, 64) < 0)
		sysError("read");

	return 0;
}

int sessionStatus(int fd, char statusbuffer[])
{
	struct stat st;
	fstat(fd, &st);

	// If file is zero byte, status is ready.
	if (!st.st_size) {
		statusbuffer = "READY";
		return 0;
	}

	// TODO: Switch to atomic seek and read.
	if (lseek(fd, -195, SEEK_END) == -1)
		sysError("lseek");

	if (read(fd, statusbuffer, 64) < 0)
		sysError("read");

	if (lseek(fd, -64, SEEK_CUR) == -1)
		sysError("lseek");

	return 0;
}

static int isRunning(int fd)
{
	struct fsession csession;

	sessionStatus(fd, csession.status);

	if (strcmp(csession.status, "RUNNING"))
		return false;
	return true;
}

static int startTask(char *session)
{
	const int fd = open(FOCUSFILE, O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	
	if (fd == -1)
		sysError("open");

	time_t *timer = 0;
	const char startstatus[64] = "RUNNING";
	char starttime[64] = { 0 };
	const char enddtime[64] = { 0 };
	time_t epoch;

	if (isRunning(fd))
		return EXIT_FAILURE;

	if (write(fd, session, 64) != 64)
		sysError("write() failed");

	epoch = time(timer);

	if (sprintf(starttime, "%li", epoch) < sizeof(epoch))
		sysError("sprintf() failed");

	if (write(fd, "|", 1) != 1)
		sysError("write() failed");

	if (write(fd, startstatus, 64) != 64)
		sysError("write() failed");

	if (write(fd, "|", 1) != 1)
		sysError("write() failed");

	if (write(fd, starttime, 64) != 64)
		sysError("write() failed");

	if (write(fd, "|", 1) != 1)
		sysError("write() failed");

	if (write(fd, enddtime, 64) != 64)
		sysError("write() failed");

	if (write(fd, "\n", 1) != 1)
		sysError("write() failed");

	return 0;
}


static int stopTask(void)
{
	const int fd = open(FOCUSFILE, O_RDWR, S_IRUSR | S_IWUSR);

	if (fd == -1)
		sysError("open");

	time_t *timer = 0;
	const char stopstatus[64] = "STOPPED";
	char endtime[64] = { 0 };
	const time_t epoch = time(timer);

	// This will leave the offset at the status
	if (!isRunning(fd))
		appError(E_STOP);

	if (write(fd, stopstatus, 64) != 64)
		sysError("write() failed");

	if (sprintf(endtime, "%li", epoch) < sizeof(epoch))
		sysError("sprintf() failed");

	if (write(fd, "|", 1) != 1)
		sysError("write() failed");

	if (lseek(fd, +64, SEEK_CUR) == -1)
		sysError("lseek");

	if (write(fd, "|", 1) != 1)
		sysError("write() failed");

	if (write(fd, endtime, 64) != 64)
		sysError("write() failed");

	return 0;
}

void sysError(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

void appError(errstate dferror)
{
	fprintf(stdout, errmsgs[dferror]);
	exit(EXIT_FAILURE);
}

