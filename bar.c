// clang -Xclang -ast-dump -fsyntax-only -std=c99 -pedantic bar.c
// clang -Xclang -ast-dump -fsyntax-only -std=c99 -pedantic-errors bar.c
// gcc -std=c99 -pedantic bar.c
// gcc -std=c99 -pedantic-errors bar.c

// #include <stdio.h>

typedef unsigned long long ull;
typedef const ull cull;
typedef cull f0(int);
typedef f0 *f1(float);
typedef f0 *(*f2)(f1 *, ull);
typedef cull (*(*crazy)(cull (*(*)(float))(int), ull))(int);
// std::is_same<f2, crazy>::value == true

typedef int Foo;
typedef Foo *Bar;
typedef const Bar xyz;

const int w = 10;

struct Node {
    int id : ' ';
    char *name;
    struct Internal {
        int length;
        int width;
    } internal;
    // enum Qualifier qual;
    float;
};

enum Qualifier {
    CONST,
    RESTRICT,
    VOLATILE,
    WTF = CONST // ???
};

typedef float (*F)(long **);

F helper(double *arg, int arr[static const volatile restrict 4]);

struct Edge {
    int k;
};

struct InComplete;

const int step = 1;

F helper(double *arg, int arr[static const volatile restrict 4]) {
    ull a[10] = {
        sizeof(struct Edge), // If struct is not complete, then error.
        sizeof(int *()),
        sizeof(1.0F + 3.0),
        sizeof(1ULL + 3),
        sizeof(void),
        sizeof(F),
        sizeof(helper),
        sizeof(long),
    };
    for (int i = 0; i < 8; ++i)
        break;
    // printf("a[%d]: %d\n", i, a[i]);
    // struct KFC becomes incomplete outside this function.
    struct KFC {
        int Thursday;
        int crazy;
    };
    struct KFC kfc;
    return 0;
}

inline long int long function(int k) {
    return k++;
}

struct BitField {
    unsigned char a : 1;
    unsigned char b : 7;
};

int lol, ac = 0;

void bar(int a[*]);

void bar(int a[]) {
    return;
}

int main() {
    enum Qualifier q = CONST;
    int n = 10, m = 100;
    int arr[n]; // VLA
    (const int *){0, 1}[0];
    float floatArr[4 + 6 * 11][sizeof(struct Node)];
    char str[] = "hello"
                 " world";
    float (*(*const func)(double *, int[]))(long **) = helper;
    struct Node node = (struct Node){0, "Michael"};
    struct Node *nodePtr = &node;
    node.name;
    nodePtr->id;
    (*nodePtr).id;
    // func(0, 0);
    helper(0, '\0');
    // function(10);
    (3.14L + 114.514F) - (sizeof(44L) - 'a');
    unsigned kkk = (unsigned)(0x12 & 1 == 2);
    return 0;
}