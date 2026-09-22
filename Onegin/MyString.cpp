#include <stdio.h>

int MyStrlen(const char* string) {
    int cnt = 0;
    for (; *string; string++, cnt++) {
        ;
    } 
    return cnt;
}

int MyPuts(const char* string) {
    for (; *string; string++) {
        putchar(*string);
    }
    putchar('\0');
    return 0;
}

char* MyStrcpy(char* dest, const char* src) {
    char* beginDest = dest;
    for (; *src; dest++, src++) {
        *dest = *src;
    }
    *dest = *src;
    return beginDest; 
}

char* MyStrcat(char* dest, const char* src) {
    char* beginDest = dest;
    for (; *dest; dest++) {
        ;
    }
    for (; *src; dest++, src++) {
        *dest = *src;
    }
    *dest = *src;
    return beginDest;
}

int MyStrcmp(const char* s1, const char *s2) {
    char c1 = '\0', c2 = '\0';
    for (; (c1 = *s1) && (c2 = *s2) && (c1 == c2); s1++, s2++) {
        ;
    }
    if (!(*s1)) {
        if (!(*s2)) {
            return 0;
        }
        return -1;
    }
    if (!(*s2)) {
        return 1;
    }
    if (*s1 < *s2) {
        return -1;
    }
    return 1;
}

void MyPrintf(const char* string) {
    for (; *string != '\0' && *string != '\n'; string++) {
        putchar(*string);
    }
    putchar(*string);
}
