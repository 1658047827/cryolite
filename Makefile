CXX = clang++
CXXFLAGS = -std=c++17
ifeq ($(OS), Windows_NT)
	TARGET = cryolite.exe
	RM = del /q
else
	TARGET = cryolite
	RM = rm -f
endif

$(TARGET): diagnostic.o token.o lexer.o syntax.o parser.o main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

diagnostic.o: diagnostic.cpp diagnostic.h
	$(CXX) $(CXXFLAGS) -c $<

token.o: token.cpp token.h tokenkind.def
	$(CXX) $(CXXFLAGS) -c $<

lexer.o: lexer.cpp lexer.h token.h diagnostic.h tokenkind.def
	$(CXX) $(CXXFLAGS) -c $<

syntax.o: syntax.cpp syntax.h
	$(CXX) $(CXXFLAGS) -c $<

parser.o: parser.cpp parser.h token.h syntax.h
	$(CXX) $(CXXFLAGS) -c $<

main.o: main.cpp lexer.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	$(RM) *.o $(TARGET)