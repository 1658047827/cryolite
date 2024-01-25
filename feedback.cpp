#include <iostream>

#include "feedback.h"

void Error(SourceLocation &loc, const std::string &msg) {
    std::cerr << SrcLocToString(loc) << ANSI_COLOR_RED << "error: ";
    std::cerr << ANSI_COLOR_RESET << msg << std::endl;
}

void Warning(SourceLocation &loc, const std::string &msg) {
    std::cerr << SrcLocToString(loc) << ANSI_COLOR_YELLOW << "warning: ";
    std::cerr << ANSI_COLOR_RESET << msg << std::endl;
}