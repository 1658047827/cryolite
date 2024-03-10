# cryolite
C language compiler

## TODO

+ Replace std::string with IdentifierInfo* in many structures.

+ Replace Parser::cursor with a Token like Clang and provide methods to control cursor.

+ Refactor Token and Lexer.

## Notice

+ It strictly follows the ISO C99 standard, like clang with "-pedantic".

## Reference

+ [llvm/llvm-project](https://github.com/llvm/llvm-project)
+ [a7vinx/axcc](https://github.com/a7vinx/axcc)