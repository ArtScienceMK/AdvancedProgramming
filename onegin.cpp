#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>

#include "MyError.h"
#include "MyAssert.h"
#include "MyString.h"

struct Text {
    size_t buferSize, maxLines;

    char* bufer;
    String* index;
};

const char* InputFilename = "OneginFile.txt";
const char* OutputFilename = "OneginOut.txt";

Text ReadText(const char* filename);

void ClearFile(const char* filename);

void WriteBuferToFile(const char* filename, char* bufer, int buferSize);
void WriteIndexToFile(const char* filename, String* index, int maxLines, int buferSize);
void WriteSepToFile(const char* filename);

void DebugPrintLine(char* line);
void PrintLine(char* line);

void PrintString(char* string);

bool EndOfString(char c);

int CountLines(char* buf);

void BubbleSort(void** arr, size_t arrSize, int (*MyCmp)(void*, void*), void (*StringSwap)(void*, void*));

// Comparators returns -1 = less, 0 = equal, 1 = more, other result = error
int CmpLettersAZ(void* ptrString1, void* ptrString2);
int CmpLettersZA(void* ptrString1, void* ptrString2);

int CmpLettersInvAZ(void* ptrString1, void* ptrString2);
int CmpLettersInvZA(void* ptrString1, void* ptrString2);

int StrCmpAZ(void* ptrString1, void* ptrString2);
int StrCmpZA(void* ptrString1, void* ptrString2);

int StrCmpInvAZ(void* ptrString1, void* ptrString2);
int StrCmpInvZA(void* ptrString1, void* ptrString2);


void StringSwap(void* op1, void* op2);  //  Почему нельзя сделать универсальный swap для void**, void**, чтобы менять местами два void указателя

int main() {    
    // Text textOnegin = {};
    // Text textTrash = ReadText(InputFilename);
    Text textOnegin = ReadText(InputFilename);
    
    int (*CmpFuncAZ)(void*, void*) = CmpLettersAZ;
    int (*CmpFuncZA)(void*, void*) = CmpLettersZA;
    int (*CmpFuncInvAZ)(void*, void*) = CmpLettersInvAZ;
    int (*CmpFuncInvZA)(void*, void*) = CmpLettersInvZA;

    void (*SwapFunc)(void*, void*) = StringSwap;

    // printf("Comaprison result of first two lines is %i\n",
    //      CmpFunc(textOnegin.index[1].line, textOnegin.index[2].line));
    
    // StringSwap((void*)&(textOnegin.index[0]), (void*)&(textOnegin.index[1]));

    // printf("Index 0 line printf<%s>\n", textOnegin.index[0].line);
    
    // printf("Index 0 line printString\n");
    // PrintString(textOnegin.index[0].line);

    // fprintf(stderr, "index[0].size: %i\n", textOnegin.index[0].size);
    // fprintf(stderr, "index[1].size: %i\n", textOnegin.index[1].size);
    // fprintf(stderr, "index[2].size: %i\n", textOnegin.index[2].size);
    
    // DebugPrintLine(textOnegin.index[0].line);
    // DebugPrintLine(textOnegin.index[1].line);    
    // DebugPrintLine(textOnegin.index[2].line);    

    ClearFile(OutputFilename);

    // BubbleSort((void**)textOnegin.index, textOnegin.maxLines, CmpFuncAZ, SwapFunc);

    // WriteIndexToFile(OutputFilename, textOnegin.index, textOnegin.maxLines, textOnegin.buferSize);    
    // WriteSepToFile(OutputFilename);

    qsort((void**)textOnegin.index, textOnegin.maxLines, sizeof(textOnegin.index[0]), (comparison_fn_t)CmpFuncInvAZ);

    WriteIndexToFile(OutputFilename, textOnegin.index, textOnegin.maxLines, textOnegin.buferSize);    
    WriteSepToFile(OutputFilename);

    WriteBuferToFile(OutputFilename, textOnegin.bufer, textOnegin.buferSize);

    free(textOnegin.bufer);
    free(textOnegin.index);

    // Main Sample
    // FILE* ptrInFile = fopen(InputFilename, "r");
    // fclose(ptrInFile);

    // FILE* ptrFile = fopen(OutputFilename, "w");

    // ASSERT(ptrFile != NULL, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    // // for (int i = 0; i < buferSize; i++) {
    // //     fprintf(stderr, "<%c>", bufer[i]);
    // // }
    // const char* s = "MEOW\n";
    // int sz = 6;
    // fprintf(stderr, "actual size: %i\n", sz);

    // fwrite(s, sizeof(s[0]), sz, ptrFile);  //  Почему не пишет, коли в буфере все ок
    // fclose(ptrFile);

    return 0;
}

Text ReadText(const char* filename) {

    FILE* ptrFile = fopen(filename, "r");
    // FILE* ptrFileCopy = ptrFile;

    ASSERT(ptrFile != NULL, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    struct stat fileStats = {};

    // fprintf(stderr, "File descriptor: %i\n", fileno(ptrFile));

    int fstatStatus = fstat(fileno(ptrFile), &fileStats);

    ASSERT(fstatStatus == 0, GetErrorString(FILE_FSTAT_ERROR), FILE_FSTAT_ERROR);

    Text text = {};
    text.buferSize = fileStats.st_size + sizeof(char);

    fprintf(stderr, "text.buferSize: %i\n", text.buferSize);

    text.bufer = (char*)calloc(text.buferSize, sizeof(text.bufer[0]));
    
    int charsRead = fread(text.bufer, sizeof(text.bufer[0]), text.buferSize, ptrFile);
    // fclose(ptrFile);

    text.bufer[text.buferSize - 1] = '\0';

    // fprintf(stderr, "(ReadText): full bufer: <");
    // for (int i = 0; i < text.buferSize; i++) {
    //     fprintf(stderr, "%c", text.bufer[i]);
    // }
    // fprintf(stderr, ">");
    // return text;

    text.maxLines = CountLines(text.bufer) + 1;

    fprintf(stderr, "maxLines: %i\n", text.maxLines);

    text.index = (String*)calloc(text.maxLines, sizeof(text.index[0]));


    int indexCnt = 0;

    text.index[indexCnt++].line = &text.bufer[0];

    for (int i = 0; i + 1 < text.buferSize && indexCnt < text.maxLines; i++) {
        
        // fprintf(stderr, "i: %i, indexCnt: %i\n", i, indexCnt);
        
        ASSERT(0 <= indexCnt && indexCnt < text.maxLines, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

        if (text.bufer[i] == '\n') {

            text.index[indexCnt - 1].size = (text.bufer + i) - text.index[indexCnt - 1].line + 1;

            fprintf(stderr, "(ReadText): text.index[Cnt - 1].size %i\n", text.index[indexCnt - 1].size);
           
            text.index[indexCnt].line = &(text.bufer[i+1]);
            indexCnt++;
        }
    }

    // fprintf(stderr, "(ReadText): text.index[0].line: %i\n", text.index[0].line - text.bufer);

    // text.index[text.maxLines - 1].size = (text.bufer + text.buferSize - 2)
    //                                     - text.index[text.maxLines - 1].line + 1 + 1;
    
    // text.index[text.maxLines - 1].line = &text.bufer[text.buferSize - 1];
    text.index[text.maxLines - 1].size = 1;

    // fprintf(stderr, "(ReadText): text.index[maxLines - 1].size %i\n", text.index[text.maxLines - 1].size);    

    fprintf(stderr, "(ReadText): finished\n");

    fclose(ptrFile);  
    
    // fprintf(stderr, "(ReadText): full index: <");
    // for (int i = text.maxLines - 10; i < text.maxLines; i++) {
    //     printf("<%s>", text.index[i].line);
    // }
    // PrintLine(text.index[text.maxLines - 1].line);
    // fprintf(stderr, ">");
    return text;
}

void ClearFile(const char* filename) {
    FILE* ptrFile = fopen(filename, "w");

    ASSERT(ptrFile != NULL, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);
    
    fclose(ptrFile);
}

void WriteBuferToFile(const char* filename, char* bufer, int buferSize) {
    FILE* ptrFile = fopen(filename, "a");  // Дед, почему нельзя ставить просто w

    ASSERT(ptrFile != NULL, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    // for (int i = 0; i < buferSize; i++) {
    //     fprintf(stderr, "<%c>", bufer[i]);
    // }
    // const char* s = "MEOW\n";
    fwrite(bufer, sizeof(bufer[0]), buferSize, ptrFile);  //  Почему не пишет, коли в буфере все ок
    
    fclose(ptrFile);
}

void WriteIndexToFile(const char* filename, String* index, int maxLines, int buferSize) {
    FILE* ptrFile = fopen(filename, "a");

    ASSERT(ptrFile != NULL, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    // for (int i = 0; i < buferSize; i++) {
    //     fprintf(stderr, "<%c>", bufer[i]);
    // }
    // const char* s = "MEOW\n";
    for (int i = 0; i < maxLines; i++) {
        for (int j = 0; j < index[i].size; j++) {
            fputc(index[i].line[j], ptrFile);
        }
    }
    fclose(ptrFile);
}

void WriteSepToFile(const char* filename) {
    FILE* ptrFile = fopen(filename, "a");

    ASSERT(ptrFile != NULL, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    const char* sepString = "\n-------------------------------------\n";

    fputs(sepString, ptrFile);

    fclose(ptrFile);
}

void DebugPrintLine(char* line) {
    for (; *line != '\0' && *line != '\n'; line++) {
        printf("char: <%c> with number: %i\n", *line, *line);
    }
    printf("char: <%c> with number: %i\n", *line, *line);    
}

bool EndOfString(char c) {
    return (c == '\n') || (c == '\0');
}

int CountLines(char* bufer) {
    //  Note, that file should end with a newline character
    int lines = 0;
    for (; *bufer; bufer++) {

        if (*bufer == '\n') {
            lines++;
        }

    }

    return lines;
}

void BubbleSort(void** arr, size_t arrSize, int (*MyCmp)(void*, void*), void (*StringSwap)(void*, void*)) {

    String* arrString = (String*)arr;

    fprintf(stderr, "(Bubble sort): started\n");

    for (size_t i = 0; i < arrSize - 1; i++) {
        for (size_t j = i + 1; j < arrSize; j++) {

            int cmpResult = MyCmp((void*)&(arrString[i]), (void*)&(arrString[j]));
            
            ASSERT(cmpResult != 2, GetErrorString(CMP_ERROR), CMP_ERROR);

            if (cmpResult == 1) {
                StringSwap((void*)&(arrString[i]), (void*)&(arrString[j]));

                fprintf(stderr, "(BubbleSort): want to swap i: %i, j: %i\n", i, j);
            }
        }
    }
}

int CmpLettersAZ(void* ptrString1, void* ptrString2) {
    char* s1 = ((String*)ptrString1)->line;
    char* s2 = ((String*)ptrString2)->line;

    // fprintf(stderr, "*s1:<%c>, *s2:<%c>\n", *s1, *s2);

    if (EndOfString(*s1)) {
        if (EndOfString(*s2)) {
            return 0;
        }
        return 1;
    }

    if (EndOfString(*s2)) {
        return -1;
    }

    while (!EndOfString(*s1) && !EndOfString(*s2)) {
        // fprintf(stderr, "(CmpLetterAZ): *s1: <%c>, *s2: <%c>\n", *s1, *s2);

        while (!isalpha(*s1)) {
            s1++;
        }
        while (!isalpha(*s2)) {
            s2++;
        }
        if (*s1 != *s2) {
            // fprintf(stderr, "(CmpLetterAZ): *s1: <%c>, *s2: <%c>\n", *s1, *s2);

            return (*s1 > *s2) ? 1 : -1;
        }
        s1++, s2++;
    }
    if (EndOfString(*s1)) {
        if (EndOfString(*s2)) {
            return 0;
        }
        return -1;
    }
    if (EndOfString(*s2)) {
        return 1;
    }

    return 2;
}

int CmpLettersZA(void* ptrString1, void* ptrString2) {
    int cmpLettersAZResult = CmpLettersAZ(ptrString1, ptrString2);

    if (cmpLettersAZResult == 2) {
        return 2;
    }

    return -cmpLettersAZResult;
}

int CmpLettersInvAZ(void* ptrString1, void* ptrString2) {
    char* s1 = ((String*)ptrString1)->line;
    char* s2 = ((String*)ptrString2)->line;

    char* s1Begin = s1, *s2Begin = s2;
    // fprintf(stderr, "*s1:<%c>, *s2:<%c>\n", *s1, *s2);

    if (EndOfString(*s1)) {
        if (EndOfString(*s2)) {
            return 0;
        }
        return 1;
    }

    if (EndOfString(*s2)) {
        return -1;
    }

    while (!EndOfString(*s1)) {
        s1++;
    }

    s1--;

    while (!EndOfString(*s2)) {
        s2++;
    }

    s2--;

    while (s1 != s1Begin && s2 != s2Begin) {
        // fprintf(stderr, "(CmpLetterAZ): *s1: <%c>, *s2: <%c>\n", *s1, *s2);

        while (!isalpha(*s1)) {
            s1--;
        }
        while (!isalpha(*s2)) {
            s2--;
        }
        if (*s1 != *s2) {
            // fprintf(stderr, "(CmpLetterAZ): *s1: <%c>, *s2: <%c>\n", *s1, *s2);

            return (*s1 > *s2) ? 1 : -1;
        }
        s1--, s2--;
    }
    if (s1 == s1Begin) {
        if (s2 == s2Begin) {
            return 0;
        }
        return -1;
    }
    if (s2 == s2Begin) {
        return 1;
    }

    return 2;    
}

int CmpLettersInvZA(void* ptrString1, void* ptrString2) {
    int cmpLettersInvAZResult = CmpLettersInvAZ(ptrString1, ptrString2);

    if (cmpLettersInvAZResult == 2) {
        return 2;
    }

    return -cmpLettersInvAZResult;
}

int StrCmpAZ(void* ptrString1, void* ptrString2) {
    char* s1 = ((String*)ptrString1)->line;
    char* s2 = ((String*)ptrString2)->line;

    // fprintf(stderr, "*s1:<%c>, *s2:<%c>\n", *s1, *s2);

    if (EndOfString(*s1)) {
        if (EndOfString(*s2)) {
            return 0;
        }
        return 1;
    }

    if (EndOfString(*s2)) {
        return -1;
    }

    while (!EndOfString(*s1) && !EndOfString(*s2)) {
        // fprintf(stderr, "(CmpLetterAZ): *s1: <%c>, *s2: <%c>\n", *s1, *s2);
        if (*s1 != *s2) {
            // fprintf(stderr, "(CmpLetterAZ): *s1: <%c>, *s2: <%c>\n", *s1, *s2);

            return (*s1 > *s2) ? 1 : -1;
        }
        s1++, s2++;
    }
    if (EndOfString(*s1)) {
        if (EndOfString(*s2)) {
            return 0;
        }
        return -1;
    }
    if (EndOfString(*s2)) {
        return 1;
    }

    return 2;
}

int StrCmpZA(void* ptrString1, void* ptrString2) {
    int strCmpAZResult = StrCmpAZ(ptrString1, ptrString2);

    if (strCmpAZResult == 2) {
        return 2;
    }

    return -strCmpAZResult;

}

int StrCmpInvAZ(void* ptrString1, void* ptrString2) {
    char* s1 = ((String*)ptrString1)->line;
    char* s2 = ((String*)ptrString2)->line;

    char* s1Begin = s1, *s2Begin = s2;
    // fprintf(stderr, "*s1:<%c>, *s2:<%c>\n", *s1, *s2);

    if (EndOfString(*s1)) {
        if (EndOfString(*s2)) {
            return 0;
        }
        return 1;
    }

    if (EndOfString(*s2)) {
        return -1;
    }

    while (!EndOfString(*s1)) {
        s1++;
    }

    s1--;

    while (!EndOfString(*s2)) {
        s2++;
    }

    s2--;

    while (s1 != s1Begin && s2 != s2Begin) {
        // fprintf(stderr, "(CmpLetterAZ): *s1: <%c>, *s2: <%c>\n", *s1, *s2);

        if (*s1 != *s2) {
            // fprintf(stderr, "(CmpLetterAZ): *s1: <%c>, *s2: <%c>\n", *s1, *s2);

            return (*s1 > *s2) ? 1 : -1;
        }
        s1--, s2--;
    }
    if (s1 == s1Begin) {
        if (s2 == s2Begin) {
            return 0;
        }
        return -1;
    }
    if (s2 == s2Begin) {
        return 1;
    }

    return 2;
}

int StrCmpInvZA(void* ptrString1, void* ptrString2) {
    int strCmpInvAZResult = StrCmpInvAZ(ptrString1, ptrString2);

    if (strCmpInvAZResult == 2) {
        return 2;
    }

    return -strCmpInvAZResult;

}

void StringSwap(void* op1, void* op2) {
    String temp = *(String*)op1;
    *(String*)op1 = *(String*)op2;
    *(String*)op2 = temp;
}

void PrintLine(char* line) {
    for (; *line != '\0' && *line != '\n'; line++) {
        putchar(*line);
    }
    putchar(*line);
}

void PrintString(char* string) {
    for (; *string != '\0'; string++) {
        putchar(*string);
    }
}

// void BubbleSort(const void** arr, const size_t arrSize, int (*MyCmp)(void*, void*), void (*StringSwap)(void*, void*)) {
//     // sorts in ascending order

//     for (int i = 0; i < arrSize; i++) {
//         for (int j = 0; j < arrSize - 1; j++) {
            
//             // fprintf(stderr, "Comparing strings[index[%i]]: %s, strings[index[%i]]: %s\n", 
//             // j, strings[index[j]].line, j + 1, strings[index[j + 1]].line);

//             // printf("Comparison result is: %i\n", MyStrcmp(strings[index[j]].line, strings[index[j + 1]].line));

//             if (MyCmp(arr[i], arr[j]) > 0) {
//                 StringSwap(&arr[i], &arr[j]);
//                 // printf("Want to SwapSize_t: index[%i] and index[%i]\n", j, j + 1);
//             }

//         }
//     }
// }
