# cryolite
C language compiler

## TODO

+ Replace std::string with IdentifierInfo* in many structures.

+ Replace Parser::cursor with a Token like Clang and provide methods to control cursor.

## Notice

+ Nested struct definitions like:
    ```C
    struct foo {
        int id;
        struct bar {
            float val;
        } internal;
    };
    ```
    is not supported yet. The C code above should be written as:
    ```C
    struct bar {
        float val;
    };
    
    struct foo {
        int id;
        struct bar internal;
    };
    ```
    to pass compilation and get a correct result.

## Reference

+ [llvm/llvm-project](https://github.com/llvm/llvm-project)
+ [a7vinx/axcc](https://github.com/a7vinx/axcc)