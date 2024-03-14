CXX = clang++

CXXFLAGS = -std=c++17

ifeq ($(OS), Windows_NT)
	TARGET = cryolite.exe
	RM = del /q
else
	TARGET = cryolite
	RM = rm -f
endif

SRCS = Compile.cpp Diagnostic.cpp TokenKind.cpp Token.cpp Identifier.cpp Lexer.cpp

OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)