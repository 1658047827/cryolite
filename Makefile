CXX = clang++
CXXFLAGS = -std=c++17
ifeq ($(OS), Windows_NT)
	TARGET = cryolite.exe
	RM = del /q
else
	TARGET = cryolite
	RM = rm -f
endif

$(TARGET): diagnostic.o token.o lexer.o type.o ast.o semantic.o parser.o main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

diagnostic.o: diagnostic.cpp Diagnostic.h
	$(CXX) $(CXXFLAGS) -c $<

token.o: token.cpp Token.h TokenKind.def
	$(CXX) $(CXXFLAGS) -c $<

lexer.o: lexer.cpp Lexer.h Token.h Diagnostic.h TokenKind.def
	$(CXX) $(CXXFLAGS) -c $<

parser.o: parser.cpp Parser.h Token.h AST.h
	$(CXX) $(CXXFLAGS) -c $<

type.o: type.cpp Type.h ArithType.def
	$(CXX) $(CXXFLAGS) -c $<

ast.o: ast.cpp AST.h Token.h Type.h Visitor.h ExprNode.def
	$(CXX) $(CXXFLAGS) -c $<

semantic.o: semantic.cpp Semantic.h
	$(CXX) $(CXXFLAGS) -c $<

main.o: Compile.cpp Lexer.h Parser.h Diagnostic.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	$(RM) *.o $(TARGET)