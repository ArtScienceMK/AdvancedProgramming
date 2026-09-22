#ifndef MY_STRING_H
#define MY_STRING_H

#define SWAP_BLOCK_CYCLE \
    for (; bytesLeft > sizeof(*s1);s1++, ptrS1++, s2++, bytesLeft -= sizeof(*s1)) { \
            buf = *s1; \
            *ptrS1 = *s2; \
            *s2 = buf; \
        } \

struct String {
    size_t size;
    char* line;
};

struct String StringInit() {
    struct String string;
    
    string.size = 0;
    string.line = NULL;
    
    return string;
}

int MyStrcmp(const char* s1, const char* s2);
int MyStrlen(const char* string);
char* MyStrcpy(char* dest, const char* src);
void MyPrintf(char* string);

void SwapStringFullBuf(String* ptrString1, String* ptrString2);
void SwapStringOneChar(String* ptrString1, String* ptrString2);
void SwapStringBlockChars(String* ptrString1, String* ptrString2);
void SwapPtrStrings(String** op1, String** op2);


#endif /*MY_STRING_H*/