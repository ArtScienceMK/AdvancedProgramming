#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
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
void ReadLinesText(Text* ptrText);

void ClearFile(const char* filename);

int GetFileSize(int fileDesc);

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

int main(int argc, char** argv) {
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

    return 0;
}

Text ReadText(const char* filename) {

    int fileDesc = open(filename, O_RDONLY);
    // FILE* ptrFileCopy = ptrFile;

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    int fileSize = GetFileSize(fileDesc);

    Text text = {};
    text.buferSize = fileSize + sizeof(char);

    fprintf(stderr, "text.buferSize: %i\n", text.buferSize);

    text.bufer = (char*)calloc(text.buferSize, sizeof(text.bufer[0]));
    
    int charsRead = read(fileDesc, text.bufer, sizeof(text.bufer[0]) * text.buferSize);
    // fclose(ptrFile);

    text.bufer[text.buferSize - 1] = '\0';

    // fprintf(stderr, "(ReadText): full bufer: <");
    // for (int i = 0; i < text.buferSize; i++) {
    //     fprintf(stderr, "%c", text.bufer[i]);
    // }
    // fprintf(stderr, ">");
    // return text;

    ReadLinesText(&text);
    // fprintf(stderr, "(ReadText): text.index[maxLines - 1].size %i\n", text.index[text.maxLines - 1].size);    

    fprintf(stderr, "(ReadText): finished\n");

    close(fileDesc);  
    
    // fprintf(stderr, "(ReadText): full index: <");
    // for (int i = text.maxLines - 10; i < text.maxLines; i++) {
    //     printf("<%s>", text.index[i].line);
    // }
    // PrintLine(text.index[text.maxLines - 1].line);
    // fprintf(stderr, ">");
    return text;
}

void ReadLinesText(Text* ptrText) {
    ptrText->maxLines = CountLines(ptrText->bufer) + 1;

    fprintf(stderr, "maxLines: %i\n", ptrText->maxLines);

    ptrText->index = (String*)calloc(ptrText->maxLines, sizeof(ptrText->index[0]));


    int indexCnt = 0;

    ptrText->index[indexCnt++].line = &ptrText->bufer[0];

    for (int i = 0; i + 1 < ptrText->buferSize && indexCnt < ptrText->maxLines; i++) {
        
        // fprintf(stderr, "i: %i, indexCnt: %i\n", i, indexCnt);
        
        ASSERT(0 <= indexCnt && indexCnt < ptrText->maxLines, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

        if (ptrText->bufer[i] == '\n') {

            ptrText->index[indexCnt - 1].size = (ptrText->bufer + i) - ptrText->index[indexCnt - 1].line + 1;

            fprintf(stderr, "(ReadText): ptrText->index[Cnt - 1].size %i\n", ptrText->index[indexCnt - 1].size);
           
            ptrText->index[indexCnt].line = &(ptrText->bufer[i+1]);
            indexCnt++;
        }
    }

    // fprintf(stderr, "(ReadText): ptrText->index[0].line: %i\n", ptrText->index[0].line - ptrText->bufer);

    // ptrText->index[ptrText->maxLines - 1].size = (ptrText->bufer + ptrText->buferSize - 2)
    //                                     - ptrText->index[ptrText->maxLines - 1].line + 1 + 1;
    
    // ptrText->index[ptrText->maxLines - 1].line = &ptrText->bufer[ptrText->buferSize - 1];
    ptrText->index[ptrText->maxLines - 1].size = 1;

}

void ClearFile(const char* filename) {
    int fileDesc = open(filename, O_WRONLY);

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);
    
    close(fileDesc);
}

int GetFileSize(int fileDesc) {
    struct stat fileStats = {};

    // fprintf(stderr, "File descriptor: %i\n", fileno(ptrFile));

    int fstatStatus = fstat(fileDesc, &fileStats);

    ASSERT(fstatStatus == 0, GetErrorString(FILE_FSTAT_ERROR), FILE_FSTAT_ERROR);

    return fileStats.st_size;
}

void WriteBuferToFile(const char* filename, char* bufer, int buferSize) {
    int fileDesc = open(filename, O_APPEND);  // Дед, почему нельзя ставить просто w

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    // for (int i = 0; i < buferSize; i++) {
    //     fprintf(stderr, "<%c>", bufer[i]);
    // }
    // const char* s = "MEOW\n";
    write(fileDesc, bufer, sizeof(bufer[0]) * buferSize);  //  Почему не пишет, коли в буфере все ок
    
    close(fileDesc);
}

void WriteIndexToFile(const char* filename, String* index, int maxLines, int buferSize) {
    int fileDesc = open(filename, O_APPEND);

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    // for (int i = 0; i < buferSize; i++) {
    //     fprintf(stderr, "<%c>", bufer[i]);
    // }
    // const char* s = "MEOW\n";
    for (int i = 0; i < maxLines; i++) {
        for (int j = 0; j < index[i].size; j++) {
            write(fileDesc, &(index[i].line[j]), sizeof(index[i].line[j]));
        }
    }
    close(fileDesc);
}

void WriteSepToFile(const char* filename) {
    int fileDesc = open(filename, O_APPEND);

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    const char* sepString = "\n-------------------------------------\n";
    int sepStringSize = MyStrlen(sepString) + 1;

    write(fileDesc, sepString, sizeof(sepString[0]) * sepStringSize);

    close(fileDesc);
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
