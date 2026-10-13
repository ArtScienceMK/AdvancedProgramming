#ifdef DEBUG

#define ON_DEBUG(...) __VA_ARGS__

#else

#define ON_DEBUG(...)

#endif

#define STRINGIFY( x ) (# x)
#define GET_SOURCE_LOCATION(sourceLocation, varName)\
    sourceLocation.file = strdup(__FILE__);\
    sourceLocation.function = strdup(__FUNCTION__);\
    sourceLocation.line = __LINE__;\
    sourceLocation.name = strdup(STRINGIFY(varName));

#include <math.h>
#include <string.h>

#include "MyAssert.h"
#include "MyError.h"
#include "MyString.h"
#include "MyVirtualMachine.h"

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

struct Text {
    size_t bufferSize, maxLines;

    char* buffer;
    String* index;
};

typedef double StackElement_t;

const char* DefaultInputFilename = "ExecutableFile.txt";
const char* DefaultOutputFilename = "Source.txt";

ERROR_STATUSES Disassemble(Text* ptrTextExec, Text* ptrTextSource);

ERROR_STATUSES ReadText(const char* filename, bool fillIndex, Text* ptrText);
ERROR_STATUSES ReadLinesText(Text* ptrText);
ERROR_STATUSES ReadFile(const char* filename, char** ptrBuffer);

ERROR_STATUSES WriteBufferToFile(const char* filename, char* buffer, size_t bufferSize);
ERROR_STATUSES WriteIndexToFile(const char* filename, String* index, size_t maxLines, size_t bufferSize);
ERROR_STATUSES WriteSepToFile(const char* filename);

ERROR_STATUSES DebugPrintLine(char* line);

ERROR_STATUSES UnlinkFile(const char* filename);

size_t GetFileSize(int fileDesc);

size_t CountChar(char* buf, char toFind);

ERROR_STATUSES MyStrlen(const char* string, int* ptrRes);
ERROR_STATUSES MyIntToString(int num, char** ptrLine);
ERROR_STATUSES ReplaceAll(char* line, char from, char to);


int NumLen(int num);

int main() {   
    Text textExec = {};
    ERROR_STATUSES ReadTextErrorStatus = ReadText(DefaultInputFilename, true, &textExec);
    
    // LOG(fprintf(stderr, "Info about first 3 lines!\n"));
    // LOG(fprintf(stderr, "index[0].size: %zu\n", textSource.index[0].size);)
    // LOG(fprintf(stderr, "index[1].size: %zu\n", textSource.index[1].size);)
    // LOG(fprintf(stderr, "index[2].size: %zu\n", textSource.index[2].size);)
    
    // LOG(DebugPrintLine(textSource.index[0].line);)
    // LOG(DebugPrintLine(textSource.index[1].line);)
    // LOG(DebugPrintLine(textSource.index[2].line);)
    
    // return 0;
        
    ERROR_STATUSES UnlinkFileErrorStatuses = UnlinkFile(DefaultOutputFilename);
    ON_DEBUG(ASSERT(UnlinkFileErrorStatuses == OK, GetErrorString(UNLINK_FILE_ERROR), UNLINK_FILE_ERROR);)

    Text textSource = {};
    
    ERROR_STATUSES DisassembleErrorStatus  = Disassemble(&textExec, &textSource);
    ON_DEBUG(ASSERT(DisassembleErrorStatus == OK, GetErrorString(DISASSEMBLE_ERROR), DISASSEMBLE_ERROR);)
    
    ERROR_STATUSES WriteBufferToFileErrorStatus = WriteBufferToFile(DefaultOutputFilename, textSource.buffer, textSource.bufferSize);
    ON_DEBUG(ASSERT(WriteBufferToFileErrorStatus == OK, GetErrorString(WRITE_BUFFER_TO_FILE_ERROR), WRITE_BUFFER_TO_FILE_ERROR);)

    // ERROR_STATUSES WriteIndexToFileErrorStatus = WriteIndexToFile(DefaultOutputFilename, textSource.index,
    //                textSource.maxLines, textSource.bufferSize);
    // ON_DEBUG(ASSERT(WriteIndexToFileErrorStatus == OK, GetErrorString(WRITE_INDEX_TO_FILE_ERROR), WRITE_INDEX_TO_FILE_ERROR);)


    // ERROR_STATUSES WriteSepToFileErrorStatus = WriteSepToFile(DefaultOutputFilename);
    // ON_DEBUG(ASSERT(WriteSepToFileErrorStatus == OK, GetErrorString(WriteSepToFileErrorStatus), WriteSepToFileErrorStatus);)

    free(textSource.buffer);
    free(textSource.index);

    free(textExec.buffer);
    free(textExec.index);

    return 0;
}

ERROR_STATUSES Disassemble(Text* ptrTextExec, Text* ptrTextSource) {
    ASSERT(ptrTextExec           != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrTextExec->buffer   != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrTextExec->index    != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrTextSource             != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrTextSource->buffer     == NULL, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrTextSource->index      == NULL, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    
    ASSERT(ptrTextSource->bufferSize == 0, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrTextSource->maxLines   == 0, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);

    ptrTextSource->buffer = (char*)calloc(ptrTextExec->bufferSize, sizeof(ptrTextSource->buffer[0]));

    ASSERT(ptrTextSource->buffer    != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    ptrTextSource->index = (String*)calloc(ptrTextExec->maxLines, sizeof(ptrTextSource->index[0]));

    ASSERT(ptrTextSource->index     != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    ptrTextSource->maxLines = ptrTextExec->maxLines;
    
    int ptrTextSourceBufferInd = 0;

    for (int curLineInd = 0; curLineInd < ptrTextExec->maxLines; curLineInd++) {

        ON_DEBUG(fprintf(stderr, "(Disassemble): curLineInd: %i\n", curLineInd);)

        ptrTextSource->index[curLineInd].line = (char*)calloc(ptrTextExec->index[curLineInd].size, 
            sizeof(ptrTextSource->index[curLineInd].line[0]));

        char* curLine = ptrTextExec->index[curLineInd].line;
        ERROR_STATUSES ReplaceAllErrorStatus = ReplaceAll(curLine, '\n', '\0');

        DebugPrintLine(curLine);

        ASSERT(curLine != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

        if (strcmp(curLine, "") == 0) {
    
            ON_DEBUG(fprintf(stderr, "END OF FILE!\n");)

            ptrTextSource->buffer[ptrTextSourceBufferInd] = '\0';
            
            ptrTextSourceBufferInd++;

            ptrTextSource->index[curLineInd].line[0] = '\0';
            ptrTextSource->index[curLineInd].size = 1;
            
            ptrTextSource->bufferSize = ptrTextSourceBufferInd;

            return OK;
        } 

        int opcode_int = 0;

        if (!sscanf(curLine, "%i", &opcode_int)) {
            ASSERT(false, GetErrorString(READING_OPCODE_DISASSEMBLE_ERROR), READING_OPCODE_DISASSEMBLE_ERROR);
        }

        char* opcodeString = NULL;

        ARG_TYPES argType = NO_ARG;
        
        int argTypeLen = 0;

        switch ((OPCODES)opcode_int) {
            case PUSH:
                opcodeString = strdup("PUSH");

                if (sscanf(curLine + NumLen(opcode_int) + 1, "%i", &argType) == 0) {
                    ASSERT(false, GetErrorString(NO_ARGUMENT_TYPE_ERROR), NO_ARGUMENT_TYPE_ERROR);
                }
                
                argTypeLen = NumLen(argType);

                break;

            case POP:
                opcodeString = strdup("POP");
                break;

            case OUT:
                opcodeString = strdup("OUT");
                break;
            
            case ADD:
                opcodeString = strdup("ADD");
                break;

            case SUB:
                opcodeString = strdup("SUB");
                break;
            
            case MUL:
                opcodeString = strdup("MUL");
                break;
            
            case DIV:
                opcodeString = strdup("DIV");
                break;
            
            case DUMP:
                opcodeString = strdup("DUMP");
                break;
            
            default:
                fprintf(stderr, GetErrorString(UNKNOWN_OPCODE));
                return UNKNOWN_OPCODE;
        }

        int ptrTextSourceIndexInd = 0;
        
        int opcodeStringLen = strlen(opcodeString);
                
        strcpy(ptrTextSource->buffer + ptrTextSourceBufferInd, opcodeString);
        strcpy(ptrTextSource->index[curLineInd].line + ptrTextSourceIndexInd, opcodeString);

        ptrTextSourceBufferInd += opcodeStringLen;
        ptrTextSourceIndexInd  += opcodeStringLen;
        
        // exit(0);
        // if (NOT LineVerify(curLine + opcodeStringLen + 1)) {
        //     fprintf(stderr, GetErrorString(Disassemble_VERIFICATION_ERROR), Disassemble_VERIFICATION_ERROR);
        //     return Disassemble_VERIFICATION_ERROR;
        // } TODO
        
        ON_DEBUG(fprintf(stderr, "(Disassemble): wanna copy <%s>\n", curLine + opcodeStringLen);)
        ON_DEBUG(DebugPrintLine(curLine + opcodeStringLen);)

        strcpy(ptrTextSource->buffer + ptrTextSourceBufferInd,                curLine + opcodeStringLen + argTypeLen);
        strcpy(ptrTextSource->index[curLineInd].line + ptrTextSourceIndexInd, curLine + opcodeStringLen + argTypeLen);

        int addedLen = strlen(curLine + opcodeStringLen + argTypeLen);
        
        ptrTextSourceBufferInd += addedLen;
        ptrTextSourceIndexInd  += addedLen;

        ptrTextSource->buffer[ptrTextSourceBufferInd]                   = '\n';
        ptrTextSource->index[curLineInd].line[ptrTextSourceIndexInd]    = '\n';

        ptrTextSourceBufferInd++;

        ptrTextSource->index[curLineInd].size = strlen(ptrTextSource->index[curLineInd].line) + 1;
    }

    ptrTextSource->buffer[ptrTextSourceBufferInd] = '\0';
            
    ptrTextSourceBufferInd++;

    ptrTextSource->bufferSize = ptrTextSourceBufferInd;

    return OK;
}

ERROR_STATUSES ReadText(const char* filename, bool fillIndex, Text* ptrText) {
    ASSERT(filename != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrText  != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrText->buffer == NULL, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrText->index == NULL, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    
    ASSERT(ptrText->bufferSize == 0, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrText->maxLines == 0, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);

    int fileDesc = open(filename, O_RDONLY);

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    size_t fileSize = GetFileSize(fileDesc);

    ptrText->bufferSize = fileSize + sizeof(char);

    LOG(fprintf(stderr, "ptrText->bufferSize: %zu\n", ptrText->bufferSize);)

    ERROR_STATUSES ReadFileErrorStatus = ReadFile(filename, &(ptrText->buffer));

    ASSERT(ReadFileErrorStatus == OK, GetErrorString(READ_FILE_ERROR), READ_FILE_ERROR);

    // fprintf(stderr, "(ReadText): full buffer: <");
    // for (int i = 0; i < text.bufferSize; i++) {
    //     fprintf(stderr, "%c", text.buffer[i]);
    // }
    // fprintf(stderr, ">");
    // return text;

    if (fillIndex) {
        ReadLinesText(ptrText);
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
    return OK;
}

ERROR_STATUSES ReadLinesText(Text* ptrText) {
    ASSERT(ptrText != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ptrText->maxLines = CountChar(ptrText->buffer, '\n') + 1;

    LOG(fprintf(stderr, "maxLines: %zu\n", ptrText->maxLines);)

    ptrText->index = (String*)calloc(ptrText->maxLines, sizeof(ptrText->index[0]));


    size_t indexCnt = 0;

    ptrText->index[indexCnt++].line = &ptrText->buffer[0];

    for (size_t i = 0; i + 1 < ptrText->bufferSize && indexCnt < ptrText->maxLines; i++) {
        
        // fprintf(stderr, "i: %i, indexCnt: %i\n", i, indexCnt);
        
        ASSERT(0 <= indexCnt && indexCnt < ptrText->maxLines, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

        if (ptrText->buffer[i] == '\n') {

            ptrText->index[indexCnt - 1].size = (size_t)((ptrText->buffer + i) - ptrText->index[indexCnt - 1].line) + 1;

            LOG(fprintf(stderr, "(ReadText): ptrText->index[Cnt - 1].size %zu\n", ptrText->index[indexCnt - 1].size);)
           
            ptrText->index[indexCnt].line = &(ptrText->buffer[i+1]);
            indexCnt++;
        }
    }

    // fprintf(stderr, "(ReadText): ptrText->index[0].line: %i\n", ptrText->index[0].line - ptrText->buffer);

    // ptrText->index[ptrText->maxLines - 1].size = (ptrText->buffer + ptrText->bufferSize - 2)
    //                                     - ptrText->index[ptrText->maxLines - 1].line + 1 + 1;
    
    // ptrText->index[ptrText->maxLines - 1].line = &ptrText->buffer[ptrText->bufferSize - 1];
    ptrText->index[ptrText->maxLines - 1].size = 1;

    return OK;
}

ERROR_STATUSES ReadFile(const char* filename, char** ptrBuffer) {
    ASSERT(filename != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrBuffer != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int fileDesc = open(filename, O_RDONLY);

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    size_t fileSize = GetFileSize(fileDesc);
    size_t bufferSize = fileSize + sizeof(char);

    LOG(fprintf(stderr, "bufferSize: %zu\n", bufferSize);)

    char* buffer = (char*)calloc(bufferSize, sizeof(buffer[0]));
    
    int charsRead = read(fileDesc, buffer, sizeof(buffer[0]) * bufferSize);
    // fclose(ptrFile);

    buffer[bufferSize - 1] = '\0';

    *ptrBuffer = buffer;

    return OK;
}

ERROR_STATUSES WriteBufferToFile(const char* filename, char* buffer, size_t bufferSize) {
    ASSERT(filename != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    
    ASSERT(buffer   != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int fileDesc = open(filename, O_WRONLY | O_APPEND);  // Дед, почему нельзя ставить просто w

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    ON_DEBUG(fprintf(stderr, "(WriteBufferToFile): bufferSize: %i\n", bufferSize);)
    ON_DEBUG(fprintf(stderr, "(WriteBufferToFile): buffer: ");)

    for (int i = 0; i < bufferSize; i++) {
        ON_DEBUG(fprintf(stderr, "<%c> has number %i\n", buffer[i], buffer[i]);)
    }
    // const char* s = "MEOW\n";
    write(fileDesc, buffer, sizeof(buffer[0]) * bufferSize);  //  Почему не пишет, коли в буфере все ок
    
    close(fileDesc);

    return OK;
}

ERROR_STATUSES WriteIndexToFile(const char* filename, String* index, size_t maxLines, size_t bufferSize) {
    ASSERT(filename != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int fileDesc = open(filename, O_WRONLY | O_APPEND);
    
    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);
    
    LOG(fprintf(stderr, "Successfully opened file with name <%s>!\n", filename));

    // for (int i = 0; i < bufferSize; i++) {
    //     fprintf(stderr, "<%c>", buffer[i]);
    // }
    // const char* s = "MEOW\n";
    for (int i = 0; i < maxLines; i++) {
        for (int j = 0; j < index[i].size; j++) {
            write(fileDesc, &(index[i].line[j]), sizeof(index[i].line[j]));
        }
    }
    close(fileDesc);

    return OK;
}

ERROR_STATUSES WriteSepToFile(const char* filename) {
    ASSERT(filename != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    
    int fileDesc = open(filename, O_WRONLY | O_APPEND);

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    const char* sepString = "\n-------------------------------------\n";
    int sepStringSize = 0;
    
    ERROR_STATUSES MyStrlenErrorStatus = MyStrlen(sepString, &sepStringSize);

    sepStringSize++;

    write(fileDesc, sepString, sizeof(sepString[0]) * sepStringSize);

    close(fileDesc);

    return OK;
}

ERROR_STATUSES DebugPrintLine(char* line) {
    ASSERT(line != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    for (; *line != '\0' && *line != '\n'; line++) {
        printf("char: <%c> with number: %i\n", *line, *line);
    }
    printf("char: <%c> with number: %i\n", *line, *line);   
    
    return OK;
}

ERROR_STATUSES UnlinkFile(const char* filename) {
    ASSERT(filename != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int fileDesc = open(filename, O_WRONLY | O_CREAT, S_IRWXU); //  unlink

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);
    
    ftruncate(fileDesc, 0);

    close(fileDesc);

    return OK;
}

size_t GetFileSize(int fileDesc) {
    struct stat fileStats = {};

    // fprintf(stderr, "File descriptor: %i\n", fileno(ptrFile));

    int fstatStatus = fstat(fileDesc, &fileStats);

    ASSERT(fstatStatus == 0, GetErrorString(FILE_FSTAT_ERROR), FILE_FSTAT_ERROR);

    return fileStats.st_size;
}

size_t CountChar(char* buffer, char toFind) {
    ASSERT(buffer != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    //  Note, that file should end with a newline character
    int lines = 0;
    for (; *buffer; buffer++) {

        if (*buffer == toFind) {
            lines++;
        }

    }

    return lines;
}

ERROR_STATUSES MyStrlen(const char* string, int* ptrRes) {
    ASSERT(string != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int cnt = 0;
    for (; *string; string++, cnt++) {
        ;
    } 

    *ptrRes = cnt;

    return OK;
}


ERROR_STATUSES MyStrcmp(const char* s1, const char *s2, int* res) {
    ASSERT(s1 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(s2 != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    char c1 = '\0', c2 = '\0';
    for (; (c1 = *s1) && (c2 = *s2) && (c1 == c2); s1++, s2++) {
        ;
    }
    if (!(*s1)) {
        if (!(*s2)) {
            *res = 0;
            return OK;
        }
        *res = -1;
        return OK;
    }
    if (!(*s2)) {
        *res = 1;
        return OK;
    }
    if (*s1 < *s2) {
        *res = -1;
        return OK;
    }
    *res = 1;
    return OK;
}

ERROR_STATUSES MyIntToString(int num, char** ptrLine) {
    int lineLen = NumLen(num);
    
    char* line = (char*)calloc(lineLen + 1, sizeof(line[0]));

    ASSERT(line != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    sprintf(line, "%i", num);

    *ptrLine = line;

    return OK;
}

int NumLen(int num) {
    if (num == 0) {
        return 1;
    }

    int res = 0;
    if (num < 0) {
        res++;
    }

    num += log10(num) + 1;
    
    return num;
}

ERROR_STATUSES ReplaceAll(char* line, char from, char to) {
    ASSERT(line != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    for (; *line; line++) {
        if (*line == from) {
            *line = to;
        }
    }

    return OK;
}
