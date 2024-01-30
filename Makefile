CXX = clang++
CXXFLAGS = -std=c++17 -g
TARGET = cryolite

cryolite: diagnostic.o token.o lexer.o syntax.o main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

diagnostic.o: diagnostic.cpp diagnostic.h
	$(CXX) $(CXXFLAGS) -c $<

token.o: token.cpp token.h
	$(CXX) $(CXXFLAGS) -c $<

lexer.o: lexer.cpp lexer.h token.h diagnostic.h
	$(CXX) $(CXXFLAGS) -c $<

syntax.o: syntax.cpp syntax.h
	$(CXX) $(CXXFLAGS) -c $<

main.o: main.cpp lexer.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o cryolite