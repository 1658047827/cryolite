// #include <stdio.h>

typedef unsigned long long ull;
;

struct Node {
    int id;
    char *name;
};

enum Qualifier {
    CONST,
    RESTRICT,
    VOLATILE
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
        sizeof(1 + 3.0),
        sizeof(int),
        sizeof(void),
        sizeof(F),
        sizeof(helper),
        sizeof(long),
    };
    for (int i = 0; i < 8; ++i)
        ;
    // printf("a[%d]: %d\n", i, a[i]);
    return 0;
}

int function(int k) {
    return k++;
}

int main() {
    enum Qualifier q = CONST;
    int n = 10;
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
    helper(0, 0);
    function(10);
    return 0;
}