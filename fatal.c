#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef _WIN32
#include <windows.h>
#endif

void
fatal(const char *fmt, ...)
{
	va_list args;
	char buf[128];
	char nfmt[128];

	snprintf(nfmt, 128, "rzrwar: %s\n", fmt);

	va_start(args, fmt);
	vsprintf(buf, nfmt, args);
	va_end(args);

#ifdef _WIN32
	MessageBox(0, buf, "rzrwar error", MB_ICONERROR|MB_TASKMODAL|MB_OK);
#else
	fprintf(stderr, buf);
#endif

	exit(-1);
}
