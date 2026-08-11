#include <stdio.h>

int main() {
#ifdef __linux
    printf("__linux defined\n");
#endif
#ifdef __hpux
    printf("__hpux defined\n");
#endif
#ifdef _AIX
    printf("_AIX defined\n");
#endif
#ifdef sun
    printf("sun defined\n");
#endif
    return 0;
}
