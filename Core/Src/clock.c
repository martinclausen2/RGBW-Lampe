/*
 * time.c
 *
 *  Created on: Sep 23, 2021
 *      Author: Martin
 */

#include "clock.h"
#include <stdbool.h>

bool Alarmflag;

// see https://stackoverflow.com/questions/26895428/how-do-i-parse-an-iso-8601-date-with-optional-milliseconds-to-a-struct-tm-in-c

void testclock()
{
    struct tm tm;
    char buf[255];

   memset(&tm, 0, sizeof(struct tm));

   const char *dateStr = "2014-11-12T19:12:14.505Z";
   int y,M,d,h,m,s;
   sscanf(dateStr, "%d-%d-%dT%d:%d:%dZ", &y, &M, &d, &h, &m, &s);
   //strptime("2001-11-12 18:31:01", "%Y-%m-%d %H:%M:%S", &tm);
   strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &tm);
}
