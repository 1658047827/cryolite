#include <iostream>

#include "diagnostic.h"

unsigned diagErrorCount = 0;
unsigned diagWarningCount = 0;

void error(const SourceLocation &loc, const std::string &msg) {
    ++diagErrorCount;
    std::cerr << srcLocToStr(loc) << ANSI_COLOR_RED << "error: ";
    std::cerr << ANSI_COLOR_RESET << msg << std::endl;
}

void warning(const SourceLocation &loc, const std::string &msg) {
    ++diagWarningCount;
    std::cerr << srcLocToStr(loc) << ANSI_COLOR_MAGENTA << "warning: ";
    std::cerr << ANSI_COLOR_RESET << msg << std::endl;
}

unsigned getErrorCount() { return diagErrorCount; }

unsigned getWarningCount() { return diagWarningCount; }

void diagCount() {
    if (diagWarningCount != 0) {
        std::cout << diagWarningCount << " warning";
        if (diagWarningCount != 1)
            std::cout << 's';
    }
    if (diagErrorCount != 0 && diagWarningCount != 0)
        std::cout << " and ";
    if (diagErrorCount != 0) {
        std::cout << diagErrorCount << " error";
        if (diagErrorCount != 1)
            std::cout << 's';
    }
    if (diagErrorCount != 0 || diagWarningCount != 0)
        std::cout << " generated.";
}