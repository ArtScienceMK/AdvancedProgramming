#include <stdio.h>
#include "MyError.h"
#include "MyAssert.h"

int MyStrlen(const char* string) {
    ASSERT(string != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int cnt = 0;
    for (; *string; string++, cnt++) {
        ;
    } 
    return cnt;
}

int MyPuts(const char* string) {
    ASSERT(string != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    for (; *string; string++) {
        putchar(*string);
    }
    putchar('\0');
    return 0;
}

char* MyStrcpy(char* dest, const char* src) {
    ASSERT(dest != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(src != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    char* beginDest = dest;
    for (; *src; dest++, src++) {
        *dest = *src;
    }
    *dest = *src;
    return beginDest; 
}

char* MyStrcat(char* dest, const char* src) {
    ASSERT(dest != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(src != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

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
    ASSERT(s1 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(s2 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

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

int MyStrNcmp(const char* s1, const char *s2, int n) {
    ASSERT(s1 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(s2 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    char c1 = '\0', c2 = '\0';
    
    int charsDone = 0;

    for (;  charsDone < n && (c1 = *s1) && (c2 = *s2) && (c1 == c2); s1++, s2++, charsDone++) {
        ;
    }

    if (charsDone == n) {
        return 0;
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
    if (*s1 == *s2) {
        return 0;
    }
    return 1;
}

char* MyStrdup(const char* src) {
    ASSERT(src != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int srcSz = MyStrlen(src) + 1;
    char* dest = (char*)calloc(srcSz, sizeof(*src));
    char* beginDest = dest;

    for (int i = 0; i < srcSz; i++) {
        *dest = *src;
        dest++, src++;
    }

    return beginDest;
}

void MyPrintf(const char* string) {
    ASSERT(string != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    for (; *string != '\0' && *string != '\n'; string++) {
        putchar(*string);
    }
    putchar(*string);
}
