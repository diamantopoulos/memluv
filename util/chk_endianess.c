#include <stdio.h>

#define LITTLE_ENDIAN 0
#define BIG_ENDIAN    1

void main() {
    int i = 1;
    char *p = (char *)&i;

    if (p[0] == 1)
        printf("LITTLE_ENDIAN\n");
    else
        printf("BIG_ENDIAN\n");
}
