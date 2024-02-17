#ifndef _CRYOLITE_DIAGNOSTIC_H_
#define _CRYOLITE_DIAGNOSTIC_H_

#include "token.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_MAGENTA "\x1b[95m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_RESET "\x1b[0m"

void error(const SourceLocation &loc, const std::string &msg);
void warning(const SourceLocation &loc, const std::string &msg);
unsigned getErrorCount();
unsigned getWarningCount();
void diagCount();

#endif