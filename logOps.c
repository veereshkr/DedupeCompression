#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "misc.h"

int gLogLevel;

void setGLogLevel(int logLevel){ gLogLevel = logLevel; }

void debug(int level, int numArgs, char* format, ...){
//if(level > NO_LOG && level <= LOG_LEVEL){ //LOW or MEDIUM or HIGH
if(level > NO_LOG && level <= gLogLevel){ //LOW or MEDIUM or HIGH
        va_list ap;
        va_start(ap, numArgs); vprintf(format, ap);
}
}

