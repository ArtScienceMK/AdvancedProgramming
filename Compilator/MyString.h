#ifndef _MY_STRING_H
#define _MY_STRING_H

#define SWAP_BLOCK_CYCLE \
    for (; bytesLeft > sizeof(*s1);s1++, ptrS1++, s2++, bytesLeft -= sizeof(*s1)) { \
            buf = *s1; \
            *ptrS1 = *s2; \
            *s2 = buf; \
        } \

struct  String {
    size_t size;
    char* line;
};

struct String StringInit();

struct String StringInit() {
    struct String string;
    
    string.size = 0;
    string.line = NULL;
    
    return string;
}

ERROR_STATUSES MyStrcmp(const char* s1, const char* s2, int* res);
int MyStrNcmp(const char* s1, const char* s2, int n);
char* MyStrdup(const char* src);

int MyStrlen(const char* string);
ERROR_STATUSES MyStrcpy(char* dest, const char* src, char** res);
void MyPrintf(char* string);

void SwapStringFullBuf(String* ptrString1, String* ptrString2);
void SwapStringOneChar(String* ptrString1, String* ptrString2);
void SwapStringBlockChars(String* ptrString1, String* ptrString2);
void SwapPtrStrings(String** op1, String** op2);


#endif /*_MY_STRING_H*/