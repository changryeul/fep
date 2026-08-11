#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    int         c1 = '"';
    int         c2 = ' ';
    int         c3 = '\t';
    char        *sp;

    char        input[100];

    memset(input, sizeof(input));
    memcpy(input, "abc\"\"def", 8);

    printf("input [%s]\n", input);
    sp = strchr(buf, c1);
}
