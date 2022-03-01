#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(void) {
    char *line = "hello";
    char name[6] = "hello";
    printf("%d", strcmp(name,line));
    return 0;
}
