#include "Diagnostic.h"
#include "Lexer.h"
// #include "Parser.h"
#include <iostream>

int main(int argc, char *argv[]) {
    // std::string filename(argv[1]);
    std::string filename("foo.c");
    std::vector<char> buffer = readFile(filename);

    Lexer lexer(&filename, buffer);
    Token tok;
    do {
        lexer.lex(tok);
        std::cout << tok.repr() << ' ';
    } while (tok.getKind() != TokenKind::TK_EOF);
    std::cout << std::endl;

    // ASTContext context;
    // Semantic sema(context);
    // Parser parser(lexer, sema);

    // Expr *expr = parser.parseExpression();
    // ASTDumper *astDumper = new ASTDumper(std::cout);
    // expr->accept(astDumper);

    diagCount();
    return 0;
}