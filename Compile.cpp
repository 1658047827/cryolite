#include "Diagnostic.h"
#include "Lexer.h"
#include "Parser.h"
#include <iostream>

int main(int argc, char *argv[]) {
    // std::string filename(argv[1]);
    std::string filename("foo.c");
    std::vector<char> buffer;
    readFile(filename, buffer);

    Lexer lexer(&filename, buffer);
    TokenSequence ts;
    lexer.lex(ts);
    ts.print(std::cout);
    std::cout << std::endl;

    ASTContext context;
    Semantic sema(context);
    Parser parser(ts, sema);
    
    Expr *expr = parser.parseExpression();
    ASTDumper *astDumper = new ASTDumper(std::cout);
    expr->accept(astDumper);

    diagCount();
    return 0;
}