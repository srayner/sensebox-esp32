#ifndef JSONHELPERS_H
#define JSONHELPERS_H

#include "NodeData.h"
#include <TimeLib.h> // optional for timestamp formatting

String formatUtcDatetime(time_t t) {
    struct tm timeinfo;
    gmtime_r(&t, &timeinfo);

    char buf[25];
    sprintf(buf, "%04d-%02d-%02dT%02d:%02d:%02dZ",
            timeinfo.tm_year + 1900,
            timeinfo.tm_mon + 1,
            timeinfo.tm_mday,
            timeinfo.tm_hour,
            timeinfo.tm_min,
            timeinfo.tm_sec);
    return String(buf);
}

#endif