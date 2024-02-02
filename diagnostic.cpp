#include <iostream>

#include "diagnostic.h"

void error(const SourceLocation &loc, const std::string &msg) {
    std::cerr << srcLocToString(loc) << ANSI_COLOR_RED << "error: ";
    std::cerr << ANSI_COLOR_RESET << msg << std::endl;
}

void warning(const SourceLocation &loc, const std::string &msg) {
    std::cerr << srcLocToString(loc) << ANSI_COLOR_YELLOW << "warning: ";
    std::cerr << ANSI_COLOR_RESET << msg << std::endl;
}