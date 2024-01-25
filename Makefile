CXX = clang++
CXXFLAGS = -std=c++17 -g
TARGET = cryolith

cryolith: feedback.o token.o lexer.o main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

feedback.o: feedback.cpp feedback.h
	$(CXX) $(CXXFLAGS) -c $<

token.o: token.cpp token.h
	$(CXX) $(CXXFLAGS) -c $<

lexer.o: lexer.cpp lexer.h token.h feedback.h
	$(CXX) $(CXXFLAGS) -c $<

main.o: main.cpp lexer.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o cryolith