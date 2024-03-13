CXX = clang++
CXXFLAGS = -std=c++17
ifeq ($(OS), Windows_NT)
	TARGET = cryolite.exe
	RM = del /q
else
	TARGET = cryolite
	RM = rm -f
endif

# $(TARGET): Diagnostic.o Token.o Lexer.o Type.o AST.o Semantic.o Parser.o Compile.o
$(TARGET): Diagnostic.o Token.o Lexer.o Compile.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

Diagnostic.o: Diagnostic.cpp Diagnostic.h
	$(CXX) $(CXXFLAGS) -c $<

Token.o: Token.cpp Token.h TokenKind.def
	$(CXX) $(CXXFLAGS) -c $<

Lexer.o: Lexer.cpp Lexer.h Token.h Diagnostic.h TokenKind.def
	$(CXX) $(CXXFLAGS) -c $<

# Parser.o: Parser.cpp Parser.h Token.h AST.h
# 	$(CXX) $(CXXFLAGS) -c $<

# Type.o: Type.cpp Type.h ArithType.def
# 	$(CXX) $(CXXFLAGS) -c $<

# AST.o: AST.cpp AST.h Token.h Type.h Visitor.h ASTNode.def ArithType.def
# 	$(CXX) $(CXXFLAGS) -c $<

# Semantic.o: Semantic.cpp Semantic.h
# 	$(CXX) $(CXXFLAGS) -c $<

Compile.o: Compile.cpp Lexer.h Parser.h Diagnostic.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	$(RM) *.o $(TARGET)