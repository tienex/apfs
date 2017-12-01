/*
 * Copyright (c) 2017-present Orlando Bassotto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
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

#include "nxtools/syslog_logger.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(HAVE_SYSLOG_H)
#include <syslog.h>
#ifndef LOG_PERROR
#define LOG_PERROR 0
#endif
#endif

#include <cstdlib>

using nxtools::syslog_logger;

syslog_logger::syslog_logger(char const *facility)
{
    if (facility == nullptr || *facility == '\0') {
        facility = "nxapfs";
    }

#if defined(_WIN32)
    _hEventLog = RegisterEventSourceW(nullptr, widen(facility).c_str());
#elif defined(HAVE_SYSLOG)
    openlog(facility, LOG_NDELAY | LOG_CONS | LOG_PERROR, LOG_LOCAL7);
#endif
}

syslog_logger::~syslog_logger()
{
#if defined(_WIN32)
    DeregisterEventSource(_hEventLog);
#elif defined(HAVE_SYSLOG)
    closelog();
#endif
}

void syslog_logger::
log(nx::severity::value const &severity, char const *format, va_list ap)
{
#if defined(_WIN32)
    static WORD const severity_types[] = {
        EVENTLOG_INFORMATION_TYPE,
        EVENTLOG_INFORMATION_TYPE,
        EVENTLOG_INFORMATION_TYPE,
        EVENTLOG_WARNING_TYPE,
        EVENTLOG_ERROR_TYPE,
        EVENTLOG_ERROR_TYPE
    };
    char         msgbuf[1024];
    LPWSTR       wmessage;
    std::wstring wstr;

    if (_hEventLog == nullptr || _hEventLog == INVALID_HANDLE_VALUE)
        return;

    vsnprintf(msgbuf, sizeof(msgbuf), format, ap);
    wstr = widen(msgbuf);
    wmessage = wstr.c_str();

    ReportEventW(_hEventLog, severity_types[severity], 0, nullptr,
            1, 0, &wmessage, nullptr);
#elif defined(HAVE_SYSLOG)
    static int const severity_priority[] = {
        LOG_DEBUG, LOG_NOTICE, LOG_INFO, LOG_WARNING, LOG_ERR, LOG_CRIT
    };
    char *msgbuf;

    if (!(vasprintf(&msgbuf, format, ap) < 0)) {
        syslog(severity_priority[severity] | LOG_LOCAL7, "%s", msgbuf);
        free(msgbuf);
    }
#else
    static char const * const severity_name[] = {
        "debug", "notice", "info", "warning", "error", "fatal", nullptr
    };

    fprintf(stderr, "[%s] ", severity_name[severity]);
    vfprintf(stderr, format, ap);
    fputc('\n', stderr);
#endif
}
