#include <iostream>

#include "diagnostic.h"

void Error(const SourceLocation &loc, const std::string &msg) {
    std::cerr << SrcLocToString(loc) << ANSI_COLOR_RED << "error: ";
    std::cerr << ANSI_COLOR_RESET << msg << std::endl;
}

void Warning(const SourceLocation &loc, const std::string &msg) {
    std::cerr << SrcLocToString(loc) << ANSI_COLOR_YELLOW << "warning: ";
    std::cerr << ANSI_COLOR_RESET << msg << std::endl;
}