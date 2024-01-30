#include "lexer.h"
#include "parser.h"
#include <iostream>

int main(int argc, char *argv[]) {
    std::string filename(argv[1]);
    std::vector<char> buffer;
    ReadFile(filename, buffer);
    Lexer lexer(&filename, buffer);
    TokenSequence ts;
    lexer.Lex(ts);
    ts.Print(std::cout);
    Parser parser(ts);
    TranslationUnit *root = parser.ParseTranslationUnit();
    return 0;
}