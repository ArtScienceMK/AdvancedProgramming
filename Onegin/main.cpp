#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include "MyError.h"
#include "MyAssert.h"

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

struct Text {
    size_t buferSize, maxLines;

    char* bufer;
    String* index;
};

struct Arguments {
    bool needHelp;
    char* inputFilename, *outputFilename;
};


const char* DefaultInputFilename = "OneginFile.txt";
const char* DefaultOutputFilename = "OneginOut.txt";

Arguments InitArguments(void);
void GetArguments(int argc, char** argv, Arguments* ptrArguments);
void ProvideInfo(void);

Text ReadText(const char* filename, bool fillIndex);
void ReadLinesText(Text* ptrText);

void UnlinkFile(const char* filename);

int GetFileSize(int fileDesc);

void WriteBuferToFile(const char* filename, char* bufer, int buferSize);
void WriteIndexToFile(const char* filename, String* index, int maxLines, int buferSize);
void WriteSepToFile(const char* filename);

void DebugPrintLine(char* line);
void PrintLine(char* line);

void PrintString(char* string);

bool EndOfString(char c);

int CountChar(char* buf, char toFind);

void BubbleSort(void** arr, size_t arrSize, int (*MyCmp)(void*, void*), void (*StringSwap)(void*, void*));

// Comparators returns -1 = less, 0 = equal, 1 = more, other result = error

int CmpLettersAZ(const void* ptrString1, const void* ptrString2);
int CmpLettersZA(const void* ptrString1, const void* ptrString2);

int CmpLettersInvAZ(const void* ptrString1, const void* ptrString2);
int CmpLettersInvZA(const void* ptrString1, const void* ptrString2);

int StrCmpAZ(const void* ptrString1, const void* ptrString2);
int StrCmpZA(const void* ptrString1, const void* ptrString2);

int StrCmpInvAZ(const void* ptrString1, const void* ptrString2);
int StrCmpInvZA(const void* ptrString1, const void* ptrString2);

int MyStrcmp(const char* s1, const char* s2);
int MyStrNcmp(const char* s1, const char* s2, int n);
char* MyStrdup(const char* src);

int MyStrlen(const char* string);
char* MyStrcpy(char* dest, const char* src);
void MyPrintf(char* string);

void SwapStringFullBuf(String* ptrString1, String* ptrString2);
void SwapStringOneChar(String* ptrString1, String* ptrString2);
void SwapStringBlockChars(String* ptrString1, String* ptrString2);
void SwapPtrStrings(String** op1, String** op2);


void StringSwap(void* op1, void* op2);  //  Почему нельзя сделать универсальный swap для void**, void**, чтобы менять местами два void указателя

int main(int argc, char** argv) {

    Arguments myArgv = InitArguments();
    GetArguments(argc, argv, &myArgv);

    LOG(fprintf(stderr, "MyArgv: help: %i, input file: <%s>, output file: <%s>\n", 
                    myArgv.needHelp,   myArgv.inputFilename, myArgv.outputFilename);)

    if (myArgv.needHelp) {
        ProvideInfo();
        
        return 0;
    }
    
    Text textOnegin = ReadText(myArgv.inputFilename, true);
    
    int (*CmpFuncAZ)(const void*, const void*) = CmpLettersAZ;
    int (*CmpFuncZA)(const void*, const void*) = CmpLettersZA;
    int (*CmpFuncInvAZ)(const void*, const void*) = CmpLettersInvAZ;
    int (*CmpFuncInvZA)(const void*, const void*) = CmpLettersInvZA;

    void (*SwapFunc)(void*, void*) = StringSwap;

    // printf("Comaprison result of first two lines is %i\n",
    //      CmpFunc(textOnegin.index[1].line, textOnegin.index[2].line));
    
    // StringSwap((void*)&(textOnegin.index[0]), (void*)&(textOnegin.index[1]));

    // printf("Index 0 line printf<%s>\n", textOnegin.index[0].line);
    
    // printf("Index 0 line printString\n");
    // PrintString(textOnegin.index[0].line);

    LOG(fprintf(stderr, "Info about first 3 lines!\n"));
    LOG(fprintf(stderr, "index[0].size: %i\n", textOnegin.index[0].size);)
    LOG(fprintf(stderr, "index[1].size: %i\n", textOnegin.index[1].size);)
    LOG(fprintf(stderr, "index[2].size: %i\n", textOnegin.index[2].size);)
    
    LOG(DebugPrintLine(textOnegin.index[0].line);)
    LOG(DebugPrintLine(textOnegin.index[1].line);)
    LOG(DebugPrintLine(textOnegin.index[2].line);)

    UnlinkFile(myArgv.outputFilename);

    // BubbleSort((void**)textOnegin.index, textOnegin.maxLines, CmpFuncAZ, SwapFunc);

    // WriteIndexToFile(DefaultOutputFilename, textOnegin.index, textOnegin.maxLines, textOnegin.buferSize);    
    // WriteSepToFile(DefaultOutputFilename);

    qsort((void**)textOnegin.index, textOnegin.maxLines, sizeof(textOnegin.index[0]), CmpFuncInvAZ);

    WriteIndexToFile(myArgv.outputFilename, textOnegin.index, textOnegin.maxLines, textOnegin.buferSize);    
    WriteSepToFile(myArgv.outputFilename);

    WriteBuferToFile(myArgv.outputFilename, textOnegin.bufer, textOnegin.buferSize);

    free(textOnegin.bufer);
    free(textOnegin.index);

    return 0;
}

Arguments InitArguments(void) {
    Arguments myArgv = {};
    myArgv.inputFilename = MyStrdup(DefaultInputFilename);
    myArgv.outputFilename = MyStrdup(DefaultOutputFilename);
    
    return myArgv; 
}

void GetArguments(int argc, char** argv, Arguments* ptrArguments) {
    for (int i = 1; i < argc; i++) {

        LOG(fprintf(stderr, "(GetArguments): argv[%i]: <%s>\n", i, argv[i]));

        if (MyStrcmp(argv[i], "-h") == 0) {
            ptrArguments->needHelp = true;
            LOG(fprintf(stderr, "(GetArguments): help mode activated!\n");)
        }
        if (MyStrNcmp(argv[i], "-i=", 3) == 0) {
            ptrArguments->inputFilename = MyStrdup(argv[i] + 3);
            LOG(fprintf(stderr, "(GetArguments): input filename: <%s>!\n", 
                        ptrArguments->inputFilename);)    
        } 
        if (MyStrNcmp(argv[i], "-o=", 3) == 0) {
            ptrArguments->outputFilename = MyStrdup(argv[i] + 3);
            LOG(fprintf(stderr, "(GetArguments): output filename: <%s>!\n", 
                        ptrArguments->outputFilename);)
        }
    }
}

void ProvideInfo() {
    fprintf(stderr, "./main -i=[Path to input file], -o=[Path to output file], -h (for help)\n");
}

Text ReadText(const char* filename, bool fillIndex) {

    int fileDesc = open(filename, O_RDONLY);

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    int fileSize = GetFileSize(fileDesc);

    Text text = {};
    text.buferSize = fileSize + sizeof(char);

    LOG(fprintf(stderr, "text.buferSize: %i\n", text.buferSize);)

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

    if (fillIndex) {
        ReadLinesText(&text);
    // fprintf(stderr, "(ReadText): text.index[maxLines - 1].size %i\n", text.index[text.maxLines - 1].size);    
    }
    
    LOG(fprintf(stderr, "(ReadText): finished\n");)

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
    ptrText->maxLines = CountChar(ptrText->bufer, '\n') + 1;

    LOG(fprintf(stderr, "maxLines: %i\n", ptrText->maxLines);)

    ptrText->index = (String*)calloc(ptrText->maxLines, sizeof(ptrText->index[0]));


    int indexCnt = 0;

    ptrText->index[indexCnt++].line = &ptrText->bufer[0];

    for (int i = 0; i + 1 < ptrText->buferSize && indexCnt < ptrText->maxLines; i++) {
        
        // fprintf(stderr, "i: %i, indexCnt: %i\n", i, indexCnt);
        
        ASSERT(0 <= indexCnt && indexCnt < ptrText->maxLines, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

        if (ptrText->bufer[i] == '\n') {

            ptrText->index[indexCnt - 1].size = (ptrText->bufer + i) - ptrText->index[indexCnt - 1].line + 1;

            LOG(fprintf(stderr, "(ReadText): ptrText->index[Cnt - 1].size %i\n", ptrText->index[indexCnt - 1].size);)
           
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

void UnlinkFile(const char* filename) {
    int fileDesc = open(filename, O_WRONLY); //  unlink

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
    int fileDesc = open(filename, O_WRONLY | O_APPEND);
    
    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);
    
    LOG(fprintf(stderr, "Successfully opened file with name <%s>!\n", filename));

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
    ASSERT(line != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    for (; *line != '\0' && *line != '\n'; line++) {
        printf("char: <%c> with number: %i\n", *line, *line);
    }
    printf("char: <%c> with number: %i\n", *line, *line);    
}

bool EndOfString(char c) {
    return (c == '\n') || (c == '\0');
}

int CountChar(char* bufer, char toFind) {
    ASSERT(bufer != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    //  Note, that file should end with a newline character
    int lines = 0;
    for (; *bufer; bufer++) {

        if (*bufer == toFind) {
            lines++;
        }

    }

    return lines;
}

void BubbleSort(void** arr, size_t arrSize, int (*MyCmp)(void*, void*), void (*StringSwap)(void*, void*)) {
    ASSERT(arr != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(MyCmp != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(StringSwap != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    String* arrString = (String*)arr;

    LOG(fprintf(stderr, "(Bubble sort): started\n");)

    for (size_t i = 0; i < arrSize - 1; i++) {
        for (size_t j = i + 1; j < arrSize; j++) {

            int cmpResult = MyCmp((void*)&(arrString[i]), (void*)&(arrString[j]));
            
            ASSERT(cmpResult != 2, GetErrorString(CMP_ERROR), CMP_ERROR);

            if (cmpResult == 1) {
                StringSwap((void*)&(arrString[i]), (void*)&(arrString[j]));

                LOG(fprintf(stderr, "(BubbleSort): want to swap i: %i, j: %i\n", i, j);)
            }
        }
    }
}

int CmpLettersAZ(const void* ptrString1, const void* ptrString2) {
    ASSERT(ptrString1 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrString2 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

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

        char c1 = tolower(*s1), c2 = tolower(*s2);

        if (c1 != c2) {
            // fprintf(stderr, "(CmpLetterAZ): *s1: <%c>, *s2: <%c>\n", *s1, *s2);

            return (c1 > c2) ? 1 : -1;
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

int CmpLettersZA(const void* ptrString1, const void* ptrString2) {
    ASSERT(ptrString1 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrString2 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int cmpLettersAZResult = CmpLettersAZ(ptrString1, ptrString2);

    if (cmpLettersAZResult == 2) {
        return 2;
    }

    return -cmpLettersAZResult;
}

int CmpLettersInvAZ(const void* ptrString1, const void* ptrString2) {
    ASSERT(ptrString1 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrString2 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

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

        char c1 = tolower(*s1), c2 = tolower(*s2); 

        if (c1 != c2) {
            // fprintf(stderr, "(CmpLetterAZ): *s1: <%c>, *s2: <%c>\n", *s1, *s2);

            return (c1 > c2) ? 1 : -1;
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

int CmpLettersInvZA(const void* ptrString1, const void* ptrString2) {
    ASSERT(ptrString1 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrString2 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int cmpLettersInvAZResult = CmpLettersInvAZ(ptrString1, ptrString2);

    if (cmpLettersInvAZResult == 2) {
        return 2;
    }

    return -cmpLettersInvAZResult;
}

int StrCmpAZ(const void* ptrString1, const void* ptrString2) {
    ASSERT(ptrString1 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrString2 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

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

int StrCmpZA(const void* ptrString1, const void* ptrString2) {
    ASSERT(ptrString1 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrString2 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int strCmpAZResult = StrCmpAZ(ptrString1, ptrString2);

    if (strCmpAZResult == 2) {
        return 2;
    }

    return -strCmpAZResult;

}

int StrCmpInvAZ(const void* ptrString1, const void* ptrString2) {
    ASSERT(ptrString1 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrString2 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

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

int StrCmpInvZA(const void* ptrString1, const void* ptrString2) {
    ASSERT(ptrString1 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrString2 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int strCmpInvAZResult = StrCmpInvAZ(ptrString1, ptrString2);

    if (strCmpInvAZResult == 2) {
        return 2;
    }

    return -strCmpInvAZResult;

}

void StringSwap(void* op1, void* op2) {
    ASSERT(op1 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(op2 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    String temp = *(String*)op1;
    *(String*)op1 = *(String*)op2;
    *(String*)op2 = temp;
}

void PrintLine(char* line) {
    ASSERT(line != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    for (; *line != '\0' && *line != '\n'; line++) {
        putchar(*line);
    }
    putchar(*line);
}

void PrintString(char* string) {
    ASSERT(string != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    for (; *string != '\0'; string++) {
        putchar(*string);
    }
}

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
