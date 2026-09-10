#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "MyError.h"
#include "MyAssert.h"
#include "MyString.h"

const int MaxStrings = 10;
const int MaxStringLen = 100;
const int StringsCnt = 3;

char SWAP_STRING_BUF[MaxStringLen] = "";

String StringsList[MaxStrings] = {};

const char* ConstStringsForStringsList[MaxStringLen] = {
    "aba", 
    "cd", 
    "abc"
};

int IndexStringsList[MaxStrings] = {
    0,
    1,
    2
};

void StringsListInit(int size);

void BubbleSort(String strings[], int index[], size_t stringsSize);

void SwapSize_t(size_t* op1, size_t* op2);
void SwapPtrChar(char** op1, char** op2);

size_t MaxSize_t(size_t op1, size_t op2);

bool IsGoodChar(const char* ptrChar);

int main() {
    StringsListInit(StringsCnt);

    // SwapStringBlockChars(&StringsList[0], &StringsList[1]);
    BubbleSort(StringsList, IndexStringsList, StringsCnt);

    // MyStrcpy(SWAP_STRING_BUF, (const char*)StringsList[1].line);
    // MyStrcpy(StringsList[0].line, (const char*)StringsList[1].line);

    for (int i = 0; i < StringsCnt; i++) {
        printf("StringsList[index[%i]]: %s\n", i, StringsList[IndexStringsList[i]].line);
    }


    return 0;
}

void StringsListInit(int StringListSize) {
    for (int i = 0; i < StringListSize; i++) {
        StringsList[i].size = strlen(ConstStringsForStringsList[i]) + 1;
        StringsList[i].line = (char*)calloc(sizeof(ConstStringsForStringsList[i]), StringsList[i].size);
        
        ASSERT(StringsList[i].line != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
        
        MyStrcpy(StringsList[i].line, ConstStringsForStringsList[i]);
    }
}

void BubbleSort(String strings[], int index[], size_t stringsSize) {
    // sorts in ascending order

    for (int i = 0; i < stringsSize; i++) {
        for (int j = 0; j < stringsSize - 1; j++) {
            
            fprintf(stderr, "Comparing strings[index[%i]]: %s, strings[index[%i]]: %s\n", 
            j, strings[index[j]].line, j + 1, strings[index[j + 1]].line);

            printf("Comparison result is: %i\n", MyStrcmp(strings[index[j]].line, strings[index[j + 1]].line));

            if (MyStrcmp(strings[index[j]].line, strings[index[j + 1]].line) == 1) {
                // SwapSize_t(&index[j], &index[j + 1]);
                SwapStringBlockChars(&strings[index[j]], &strings[index[j + 1]]);
                // printf("Want to SwapSize_t: index[%i] and index[%i]\n", j, j + 1);
            }

        }
    }
}

void SwapSize_t(size_t* op1, size_t* op2) {
    size_t temp = *op1;
    *op1 = *op2;
    *op2 = temp;
}

void SwapPtrChar(char** op1, char** op2) {
    char* temp = *op1;
    *op1 = *op2;
    *op2 = temp;
}

void SwapStringFullBuf(String* ptrString1, String* ptrString2) {

    if (ptrString1->size > ptrString2->size) {
        fprintf(stderr, "(SwapStringFullBuf): swapping strings\n");

        SwapPtrStrings(&ptrString1, &ptrString2);
    }

    size_t s1Size = ptrString1->size, s2Size = ptrString2->size;
    char* s1 = ptrString1->line;  // Почему не работает, если указать две иниц char* в одной строке?
    char* s2 = ptrString2->line;

    // fprintf(stderr, "(SwapStringFullBuf): before first copying\n");

    MyStrcpy(SWAP_STRING_BUF, (const char*)s1);

    // fprintf(stderr, "(SwapStringFullBuf): after first copying\n");

    char* ptrS1 = (char*)calloc(sizeof(s1), s2Size);

    ASSERT(ptrS1 != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    MyStrcpy(ptrS1, (const char*)s2);

    // fprintf(stderr, "(SwapStringFullBuf): after second copying\n");

    ptrString1->line = ptrS1;

    fprintf(stderr, "(SwapStringFullBuf): before third copying\n");

    fprintf(stderr, "(SwapStringFullBuf): s2:<%s>, buf:<%s>\n", s2, SWAP_STRING_BUF);

    //  зачем аллоцировать память, если у нас s2Size > s1Size?? Ну типо она уже занята?
    MyStrcpy(s2, (const char*)SWAP_STRING_BUF);

    fprintf(stderr, "(SwapStringFullBuf): after third copying\n"); 

    ptrString2->line = s2;
    fprintf(stderr, "(SwapStringFullBuf): string1->line:<%s>, string2->line:<%s>\n",
                    ptrString1->line, ptrString2->line);
}

void SwapPtrStrings(String** op1, String** op2) {
    String* temp = *op1;

    *op1 = *op2;
    *op2 = temp;
}

void SwapStringOneChar(String* ptrString1, String* ptrString2) {
    // А при перемещении в меньшую строку большей мы не вылезем за рамки допустимой памяти

    if (ptrString1->size > ptrString2->size) {
        fprintf(stderr, "(SwapStringOneChar): swapping strings\n");

        SwapPtrStrings(&ptrString1, &ptrString2);
    }

    size_t s1Size = ptrString1->size, s2Size = ptrString2->size;

    char* s1 = ptrString1->line;
    char* s2 = ptrString2->line;

    char buf = '\0';


    fprintf(stderr, "(SwapStringOneChar): s1:<%s>, s2:<%s>\n", s1, s2);

    ptrString1->line = (char*)calloc(sizeof(s1), s2Size);

    ASSERT((string1->line) != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    char* ptrS1 = ptrString1->line; 


    for (int i = 0; i < s2Size; i++, s1++, ptrS1++, s2++) {
        buf = *s1;
        *ptrS1 = *s2;
        *s2 = buf;

        fprintf(stderr, "buf:<%c>, *ptrS1:<%c>, *s2:<%c>\n", buf, *ptrS1, *s2);
    }

    fprintf(stderr, "s1:<%s>, s2:<%s>\n", ptrString1->line, ptrString2->line);
}

void SwapStringBlockChars(String* ptrString1, String* ptrString2) {
    if (ptrString1->size > ptrString2->size) {
        fprintf(stderr, "(SwapStringOneChar): swapping strings\n");

        SwapPtrStrings(&ptrString1, &ptrString2);
    }

    size_t s1Size = ptrString1->size, s2Size = ptrString2->size;

    char* s1 = ptrString1->line;
    char* s2 = ptrString2->line;
    char* beginS2 = s2;

    u_int64_t buf = 0;

    fprintf(stderr, "(SwapStringOneChar): s1:<%s>, s2:<%s>\n", s1, s2);

    ptrString1->line = (char*)calloc(sizeof(s1), s2Size);

    ASSERT((string1->line) != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    char* ptrS1 = ptrString1->line; 

    (u_int64_t*)s1, ptrS1, s2;
    (u_int64_t)buf;
    
    size_t bytesLeft = s2Size;

    SWAP_BLOCK_CYCLE

    (u_int32_t*)s1, ptrS1, s2;
    (u_int32_t)buf;
    
    SWAP_BLOCK_CYCLE

    (short*)s1, ptrS1, s2;
    (short)buf;
    
    SWAP_BLOCK_CYCLE

    (char*)s1, ptrS1, s2;
    (char)buf;
    
    SWAP_BLOCK_CYCLE


    fprintf(stderr, "s1:<%s>, s2:<%s>\n", ptrString1->line, ptrString2->line);

}


char* MyStrcpy(char* dest, const char* src) {

    fprintf(stderr, "(MyStrcpy) dest:<%s>, src<%s>\n", dest, src);

    char* beginDest = dest;
    for (; *src; dest++, src++) {
        fprintf(stderr, "(MyStrcpy) dest char:<%c>, src char:<%c>\n", *dest, *src);

        *dest = *src;

        fprintf(stderr, "(MyStrcpy) set char successfully\n");
    }
    *dest = *src;

    fprintf(stderr, "(MyStrcpy) dest:<%s>\n", beginDest);
    
    return beginDest; 
}

int MyStrcmp(const char* s1, const char *s2) {
    for (; IsGoodChar(s1) && IsGoodChar(s2) && ((*s1) == (*s2)); s1++, s2++) {
        ;
    }

    // printf("(MyStrcmp): Cycle ended\n");

    if (!IsGoodChar(s1)) {
        if (!IsGoodChar(s2)) {
            return 0;
        }
        return -1;
    }

    // printf("(MyStrcmp): first comparison ended\n");

    if (!IsGoodChar(s2)) {
        return 1;
    }
    if (*s1 < *s2) {
        return -1;
    }
    return 1;
}

void SwapStringBlockChars(String string1, String string2) {

}

size_t MaxSize_t(size_t op1, size_t op2) {
    return (op1 < op2) ? op1 : op2;
}

bool IsGoodChar(const char* ptrChar) {
    return ptrChar != NULL && *ptrChar != '\0';
}
