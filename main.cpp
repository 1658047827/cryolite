#include "lexer.h"
#include "parser.h"
#include <iostream>

int main(int argc, char *argv[]) {
    std::string filename(argv[1]);
    std::vector<char> buffer;
    readFile(filename, buffer);
    Lexer lexer(&filename, buffer);
    TokenSequence ts;
    lexer.lex(ts);
    ts.print(std::cout);
    // Parser parser(ts);
    // TranslationUnit *root = parser.parseTranslationUnit();
    return 0;
}