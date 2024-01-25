#include <string>
#include <iostream>
#include <vector>

#include "lexer.h"

int main(int argc, char *argv[]) {
    std::string filename(argv[1]);
    std::vector<char> buffer;
    ReadFile(filename, buffer);
    Lexer lexer(&filename, buffer);
    TokenSequence ts;
    lexer.Lex(ts);
    ts.Print(std::cout);
    return 0;
}