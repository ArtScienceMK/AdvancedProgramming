#ifdef STACK_DEBUG

#define ON_DEBUG(...)  __VA_ARGS__

#else

#define ON_DEBUG(...)

#endif

#ifdef CANARY_DEBUG

#define ON_CANARY(...) __VA_ARGS__

#else

#define ON_CANARY(...)

#endif

#ifdef HASH_DEBUG

#define ON_HASH(...)   __VA_ARGS__

#else

#define ON_HASH(...)

#endif

#define STRINGIFY( x ) (# x)
#define GET_SOURCE_LOCATION(sourceLocation, varName)\
    sourceLocation.file = strdup(__FILE__);\
    sourceLocation.function = strdup(__FUNCTION__);\
    sourceLocation.line = __LINE__;\
    sourceLocation.name = strdup(STRINGIFY(varName));


typedef double StackElement_t;

#include <math.h>
#include <string.h>

#define POISON NAN
#define CANARY ((StackElement_t) 0xEDA)

const double Eps = 1e-3;

#include "MyAssert.h"
#include "MyError.h"
#include "MyStack.h"
#include "MyString.h"
#include "MyVirtualMachine.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

struct Text {
    size_t bufferSize, maxLines;

    char* buffer;
    String* index;
};

const char* DefaultInputFilename = "ExecutableFile.txt";
const char* DefaultOutputFilename = "VirtualMachineOut.txt";

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

ERROR_STATUSES MyStrlen(const char* string, int* res);

ERROR_STATUSES Execute(Text* ptrExec);

int NumLen(int num);

int main() {   
    Text textExec = {};
    ERROR_STATUSES ReadTextErrorStatus = ReadText(DefaultInputFilename, true, &textExec);
    
    // LOG(fprintf(stderr, "Info about first 3 lines!\n"));
    // LOG(fprintf(stderr, "index[0].size: %zu\n", textExec.index[0].size);)
    // LOG(fprintf(stderr, "index[1].size: %zu\n", textExec.index[1].size);)
    // LOG(fprintf(stderr, "index[2].size: %zu\n", textExec.index[2].size);)
    
    // LOG(DebugPrintLine(textExec.index[0].line);)
    // LOG(DebugPrintLine(textExec.index[1].line);)
    // LOG(DebugPrintLine(textExec.index[2].line);)
    
    // return 0;

    ERROR_STATUSES ExecuteErrorStatus = Execute(&textExec);
    ON_DEBUG(ASSERT(ExecuteErrorStatus == OK, GetErrorString(EXECUTION_ERROR), EXECUTION_ERROR))
        
    ERROR_STATUSES UnlinkFileErrorStatuses = UnlinkFile(DefaultOutputFilename);
    ON_DEBUG(ASSERT(UnlinkFileErrorStatuses == OK, GetErrorString(UNLINK_FILE_ERROR), UNLINK_FILE_ERROR))

    ERROR_STATUSES WriteBufferToFileErrorStatus = WriteBufferToFile(DefaultOutputFilename, textExec.buffer, textExec.bufferSize);
    ON_DEBUG(ASSERT(WriteBufferToFileErrorStatus == OK, GetErrorString(WRITE_BUFFER_TO_FILE_ERROR), WRITE_BUFFER_TO_FILE_ERROR))

    ERROR_STATUSES WriteSepToFileErrorStatus = WriteSepToFile(DefaultOutputFilename);
    ON_DEBUG(ASSERT(WriteSepToFileErrorStatus == OK, GetErrorString(WriteSepToFileErrorStatus), WriteSepToFileErrorStatus))

    free(textExec.buffer);
    free(textExec.index);

    return 0;
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

    int fileDesc = open(filename, O_WRONLY | O_APPEND);  // Дед, почему нельзя ставить просто w

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    // for (int i = 0; i < bufferSize; i++) {
    //     fprintf(stderr, "<%c>", buffer[i]);
    // }
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

ERROR_STATUSES MyStrlen(const char* string, int* res) {
    ASSERT(string != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int cnt = 0;
    for (; *string; string++, cnt++) {
        ;
    } 

    *res = cnt;

    return OK;
}

ERROR_STATUSES StackInit(Stack_t* ptrStack, size_t _capacity ON_DEBUG(, SourceLocation sourceLocation)) {

    ON_DEBUG(LOG(fprintf(stderr, "(StackInit): want to init %s\n Called from %s:%i %s()\n", 
        sourceLocation.name, sourceLocation.file, sourceLocation.line, sourceLocation.function);))    //  Дед, что писать в name??
    //   Как сделать line ctr+click able??                                                                        
    
    ASSERT(ptrStack != NULL,         GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->data == NULL,   GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrStack->size == 0,      GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    
    ASSERT(ptrStack->capacity == 0,  GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrStack->status == NULL, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrStack->hash == 0,      GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);                                                             
    
    
    ptrStack->capacity = _capacity + 2; // stack initializes with size = 0
    ptrStack-> data = (StackElement_t*)calloc(ptrStack->capacity, sizeof(StackElement_t)); 
    
    ASSERT(ptrStack->data != NULL,   GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
    
    ptrStack->data[0] = CANARY;
    
    for (size_t i = 1; i < ptrStack->capacity - 1; i++) {
        ptrStack->data[i] = POISON;
    }
    
    ptrStack->data[ptrStack->capacity - 1] = CANARY;
    ptrStack->status = "Initialized\n";  //  Здесь strdup не нужен
    
    ON_HASH(ptrStack->hash = GetHash(ptrStack));
    
    ON_HASH(LOG(fprintf(stderr, "HASH IN INIT FUNC: %llu\n", ptrStack->hash);))

    ON_DEBUG(StackVerify(ptrStack));    

    return OK;
}

ERROR_STATUSES StackDestroy(Stack_t* ptrStack) {

    ON_DEBUG(StackVerify(ptrStack));

    ASSERT(ptrStack != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    free(ptrStack->data);  // TODO make a wrapper to put a poison to freed memory
    ptrStack->capacity = 0;
    ptrStack->size = 0;
    ptrStack->status = "Destroyed\n";
    ptrStack->hash = 0;

    ptrStack = NULL;
    
    // ON_DEBUG(StackVerify(ptrStack));

    return OK;
}

ERROR_STATUSES StackPush(Stack_t* ptrStack, StackElement_t pushValue) {
    
    ON_DEBUG(StackVerify(ptrStack));

    ASSERT(ptrStack != NULL,       GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->data != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    if (ptrStack->size + 1 == ptrStack->capacity - 1) {
        ptrStack->data = (StackElement_t*)realloc(ptrStack->data, sizeof(StackElement_t) * ptrStack->capacity * 2);
        
        ASSERT(ptrStack->data != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
        
        ptrStack->capacity *= 2;
    }

    ptrStack->data[ptrStack->size + 1] = pushValue;
    ptrStack->size++; 

    ON_HASH(ptrStack->hash = GetHash(ptrStack);)

    ON_DEBUG(StackVerify(ptrStack));

    return OK;
}

ERROR_STATUSES StackPop(Stack_t* ptrStack, StackElement_t* ptrValue) {

    ON_DEBUG(StackVerify(ptrStack));
    
    ASSERT(ptrStack != NULL,       GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->data != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->size > 0,     GetErrorString(EMPTY_STACK), EMPTY_STACK);

    *ptrValue = ptrStack->data[ptrStack->size];

    ptrStack->data[ptrStack->size] = POISON;
    ptrStack->size--;
    
    ON_HASH(ptrStack->hash = GetHash(ptrStack);)

    ON_DEBUG(StackVerify(ptrStack));

    return OK;
}

ERROR_STATUSES StackVerify(Stack_t* ptrStack) {

    // LOG(fprintf(stderr, "(StackVerify) starting...\n");)
    
    StackVerifyWithoutHash(ptrStack);

    ON_HASH(ASSERT(ptrStack->hash == GetHash(ptrStack), GetErrorString(HASHES_DIFFER), HASHES_DIFFER);)
    
    return OK;
}

ERROR_STATUSES StackVerifyWithoutHash(Stack_t* ptrStack) {

    // LOG(fprintf(stderr, "(StackVerify) starting...\n");)

    ASSERT(ptrStack != NULL,         GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->data   != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrStack->status != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->capacity >= ptrStack->size, GetErrorString(STACK_CAP_SIZE), STACK_CAP_SIZE);
    
    ON_CANARY(ASSERT(ptrStack->data[0] == CANARY, GetErrorString(BAD_CANARY), BAD_CANARY);)
    ON_CANARY(ASSERT(ptrStack->data[ptrStack->capacity - 1] == CANARY, GetErrorString(BAD_CANARY), BAD_CANARY);)
    
    return OK;
}

ERROR_STATUSES StackDump(Stack_t stack, SourceLocation sourceLocation) {
    ON_DEBUG(StackVerify(&stack));

    LOG(fprintf(stderr, "\n-----------\n");)
    LOG(fprintf(stderr, "(StackDump): Starting...\n");)

    LOG(fprintf(stderr, "Stack_t <%s> [%p] %s at %s {\n",
                sourceLocation.name, &stack, sourceLocation.function, sourceLocation.file);)  //  TODO make STRINGIFY

    LOG(fprintf(stderr, "capacity = %zu\n", stack.capacity);)
    LOG(fprintf(stderr, "    size = %zu\n", stack.size);)
    LOG(fprintf(stderr, "    hash = %llu\n", stack.hash);)

    if (isatty(fileno(stderr))) {
        LOG(fprintf(stderr, "%s", ORANGE);)
    }

    if (stack.data[0] != CANARY) {
        LOG(fprintf(stderr, "*[0] = %lf (BAD CANARY) should be %lf\n", stack.data[0], CANARY);)               
    } else {
        LOG(fprintf(stderr, "*[0] = %lf (CANARY)\n", stack.data[0]);)
    }

    if (isatty(fileno(stderr))) {
        LOG(fprintf(stderr, "%s", GREEN);)
    }

    for (size_t i = 1; i <= stack.size; i++) {
        LOG(fprintf(stderr, "*[%zu] = %lf\n", i, stack.data[i]);)
    }

    if (isatty(fileno(stderr))) {
        LOG(fprintf(stderr, "%s", RED);)
    }

    for (size_t i = stack.size + 1; i + 1 < stack.capacity; i++) {
        LOG(fprintf(stderr, "[%zu] = %lf (POISON)\n", i, stack.data[i]);)
    }

    if (isatty(fileno(stderr))) {
        LOG(fprintf(stderr, "%s", ORANGE);)
    }

    if (stack.data[stack.capacity - 1] != CANARY) {
        LOG(fprintf(stderr, "*[%zu] = %lf (BAD CANARY) should be %lf\n",
                    stack.capacity - 1, stack.data[stack.capacity - 1], CANARY);)               
    } else {
        LOG(fprintf(stderr, "*[%zu] = %lf (CANARY)\n",
            stack.capacity - 1, stack.data[stack.capacity - 1]);)
    }

    if (isatty(fileno(stderr))) {
        LOG(fprintf(stderr, "%s", RESET);)
    }

    LOG(fprintf(stderr, "}\n");)

    LOG(fprintf(stderr, "(StackDump): Ending...\n");)

    ON_DEBUG(StackVerify(&stack));

    return OK;
}

unsigned long long GetHash(Stack_t* ptrStack) {

    StackVerifyWithoutHash(ptrStack);

    unsigned long long hashMod = 1e9 + 7, hashPow = 239;
    unsigned long long hashValue = 0;
    hashValue += ptrStack->capacity;
    hashValue *= hashPow;
    hashValue %= hashMod;

    hashValue += ptrStack->size;
    hashValue *= hashPow;
    hashValue %= hashMod;

    for (size_t i = 0; i < ptrStack->capacity; i++) {
        hashValue += (unsigned long long)ptrStack->data[i];
        hashValue *= hashPow;
        hashValue %= hashMod;
    }

    char* statusString = (char*)ptrStack->status;
    for (; *statusString != '\0'; statusString++) {
        hashValue += (unsigned long long)*statusString;
        hashValue *= hashPow;
        hashValue %= hashMod;
    }

    StackVerifyWithoutHash(ptrStack);

    return hashValue;

}

ERROR_STATUSES Execute(Text* ptrExec) {
    ON_DEBUG(ASSERT(ptrExec         != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);)
    ON_DEBUG(ASSERT(ptrExec->buffer != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);)
    ON_DEBUG(ASSERT(ptrExec->index  != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);)

    Stack_t stk1 = {};
    
    SourceLocation sourceLocation1 = {};
    GET_SOURCE_LOCATION(sourceLocation1, stk1)

    ERROR_STATUSES StackInitErrorStatus = StackInit(&stk1, 5 ON_DEBUG(, sourceLocation1));
    
    ON_DEBUG(ASSERT(StackInitErrorStatus == OK, GetErrorString(StackInitErrorStatus), StackInitErrorStatus);)
    
    ON_DEBUG(fprintf(stderr, "(Execute): going in cycle\n");)

    for (int curLineId = 0; curLineId < ptrExec->maxLines; curLineId++) {
        int opcode = 0;
        StackElement_t args = 0;

        char* curLine = ptrExec->index[curLineId].line;

        DebugPrintLine(curLine);

        if (sscanf(curLine, "%i ", &opcode) == 0) {
            ASSERT(0, GetErrorString(OPCODE_INPUT_ERROR), OPCODE_INPUT_ERROR);
        }
        //  BUG: считывай строку во второй раз, учитывая длину opcode
        
        if ((OPCODES)opcode != POP && (OPCODES)opcode != OUT && (OPCODES)opcode != DUMP) {
            if (sscanf(curLine + NumLen(opcode), "%lf", &args) == 0) {
                ASSERT(0, GetErrorString(ARGS_INPUT_ERROR), ARGS_INPUT_ERROR);
            }

            ON_DEBUG(fprintf(stderr, "Args input enabled and correct!!!\n");)
        }
        
        ON_DEBUG(fprintf(stderr, "(Execute): opcode: %i, args: %lf\n", 
                                 opcode, args);)

        switch((OPCODES)opcode) {
            case PUSH: {
                StackPush(&stk1, args);
                break;
            }
            
            case POP: {
                StackElement_t popValue = 0;
                StackPop(&stk1, &popValue);
                break;
            }
            
            case OUT: {
                StackElement_t popValue = 0;
                StackPop(&stk1, &popValue);

                fprintf(stderr, "(out): %lf\n", popValue);
            
                break;
            }
            
            case DUMP: {
                SourceLocation sourceLocation1 = {};
                
                GET_SOURCE_LOCATION(sourceLocation1, stk1)

                StackDump(stk1, sourceLocation1);
            
                break;
            }
            

            case ADD: {
                StackElement_t op1 = 0, op2 = 0;
                StackPop(&stk1, &op1);
                StackPop(&stk1, &op2);
                
                StackPush(&stk1, op1 + op2);
                break;
            }
            
            case SUB: {
                StackElement_t op1 = 0, op2 = 0;
                StackPop(&stk1, &op1);
                StackPop(&stk1, &op2);

                StackPush(&stk1, op1 - op2);
                break;
            }
            
            case MUL: {
                StackElement_t op1 = 0, op2 = 0;
                StackPop(&stk1, &op1);
                StackPop(&stk1, &op2);

                StackPush(&stk1, op1 * op2);
                break;
            }
            
            case DIV: {
                StackElement_t op1 = 0, op2 = 0;
                StackPop(&stk1, &op1);
                StackPop(&stk1, &op2);

                ASSERT(op2 != 0, GetErrorString(ZERO_DIVISION_ERROR), ZERO_DIVISION_ERROR);

                StackPush(&stk1, op1 / op2);
                break;
            }
            
            default: {
                ASSERT(0, GetErrorString(UNKNOWN_OPCODE), UNKNOWN_OPCODE);
                break;
            }
        }
    }

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
