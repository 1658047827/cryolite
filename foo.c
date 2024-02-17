// 3.14L + 114.514F ? sizeof(44L) - 'a' : 'b' + 47  
struct Node {
    int id;
    double val;
    int next;
    struct Extra {
        char *msg;
    } extra;
}