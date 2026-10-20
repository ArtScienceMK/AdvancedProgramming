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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

struct Text {
    size_t bufferSize, maxLines;

    char* buffer;
    String* index;
};

struct AssembleContext{
    Text textSource;
    Text textExec;
    int* labels;
};

typedef int StackElement_t;

const char* DefaultInputFilename = "SourceFile.txt";
const char* DefaultOutputFilename = "ExecutableFile.txt";

const int MaxLabelsSize = 50;

ERROR_STATUSES Assemble(AssembleContext* asmContext);

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
ERROR_STATUSES MyIntToString(int num, char** ptrLine);
ERROR_STATUSES ReplaceAll(char* line, char from, char to);
ERROR_STATUSES TrimSpaces(char** ptrLine);


ERROR_STATUSES GetArgumentType(const char* line, int lineExpr, ARG_TYPES* ptrArgType);

ERROR_STATUSES registerNameStringToIndex(const char* line, REGISTERS_NAMES* ptrRegisterName);

int NumLen(int num);

ERROR_STATUSES InitLabels(int* labels);

ERROR_STATUSES ResolveLabels(Text* ptrTextSource, int* labels);

void AddSpace(char* buffer, char* line, int* ptrBufferInd, int* ptrIndexInd);

void MyStrcpyWithSep(char* bufferDest, char* lineDest, char* src, int* ptrBufferInd, size_t* ptrLineSize);

int main() {  
    AssembleContext asmContext = {};

    ERROR_STATUSES ReadTextErrorStatus = ReadText(DefaultInputFilename, true, &asmContext.textSource);
    
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
    
    int labels[MaxLabelsSize] = {};
    InitLabels(labels);

    asmContext.labels = labels;

    ERROR_STATUSES ResolveLabelsErrorStatus = ResolveLabels(&asmContext.textSource, asmContext.labels);
    ASSERT(ResolveLabelsErrorStatus == OK, GetErrorString(ResolveLabelsErrorStatus), ResolveLabelsErrorStatus);

    ERROR_STATUSES AssembleErrorStatus = Assemble(&asmContext);
    ON_DEBUG(ASSERT(AssembleErrorStatus == OK, GetErrorString(ASSEMBLE_ERROR), ASSEMBLE_ERROR);)
    
    ERROR_STATUSES WriteBufferToFileErrorStatus = WriteBufferToFile(DefaultOutputFilename,
         asmContext.textExec.buffer, asmContext.textExec.bufferSize);
    ON_DEBUG(ASSERT(WriteBufferToFileErrorStatus == OK, GetErrorString(WRITE_BUFFER_TO_FILE_ERROR), WRITE_BUFFER_TO_FILE_ERROR);)

    // ERROR_STATUSES WriteSepToFileErrorStatus = WriteSepToFile(DefaultOutputFilename);
    // ON_DEBUG(ASSERT(WriteSepToFileErrorStatus == OK, GetErrorString(WriteSepToFileErrorStatus), WriteSepToFileErrorStatus);)

    free(asmContext.textSource.buffer);
    free(asmContext.textSource.index);

    free(asmContext.textExec.buffer);
    free(asmContext.textExec.index);

    return 0;
}

ERROR_STATUSES ResolveLabels(Text* ptrTextSource, int* labels) {
    ASSERT(ptrTextSource           != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrTextSource->buffer   != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrTextSource->index    != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(labels                  != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    for (int curLineInd = 0; curLineInd < ptrTextSource->maxLines; curLineInd++) {
        
        ON_DEBUG(fprintf(stderr, "(ResolveLabels): curLineInd: %i\n", curLineInd);)

        char* curLine = ptrTextSource->index[curLineInd].line;
                
        ASSERT(curLine != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

        ERROR_STATUSES TrimSpacesErrorStatus = TrimSpaces(&curLine);

        ASSERT(curLine != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
        
        // DebugPrintLine(curLine);

        if (strncmp(curLine, ":", strlen(":")) == 0) {
            int labelInd = -1;
            
            if (sscanf(curLine + 1, "%i", &labelInd) == 0) {
                return READ_LABEL_ERROR;
            }
            
            ASSERT(labelInd < MaxLabelsSize, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
            ASSERT(labels[labelInd] == -1, GetErrorString(LABEL_REDEFINITION), LABEL_REDEFINITION);

            labels[labelInd] = curLineInd;
        }

    }

    return OK;
}

ERROR_STATUSES Assemble(AssembleContext* asmContext) {
    Text* ptrTextSource = &(asmContext->textSource);
    Text* ptrTextExec   = &(asmContext->textExec);
    int* labels         = asmContext->labels;

    ASSERT(ptrTextSource           != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrTextSource->buffer   != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrTextSource->index    != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrTextExec             != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrTextExec->buffer     == NULL, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrTextExec->index      == NULL, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    
    ASSERT(ptrTextExec->bufferSize == 0, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrTextExec->maxLines   == 0, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);

    ptrTextExec->buffer = (char*)calloc(ptrTextSource->bufferSize, sizeof(ptrTextExec->buffer[0]));

    ASSERT(ptrTextExec->buffer    != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    ptrTextExec->index = (String*)calloc(ptrTextSource->maxLines, sizeof(ptrTextExec->index[0]));

    ASSERT(ptrTextExec->index     != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    ptrTextExec->maxLines = ptrTextSource->maxLines;
    
    int ptrTextExecBufferInd = 0;

    for (int curLineInd = 0; curLineInd < ptrTextSource->maxLines; curLineInd++) {

        ON_DEBUG(fprintf(stderr, "(Assemble): curLineInd: %i\n", curLineInd);)

        ptrTextExec->index[curLineInd].line = (char*)calloc(ptrTextSource->index[curLineInd].size, 
            sizeof(ptrTextExec->index[curLineInd].line[0]));

        char* curLine = ptrTextSource->index[curLineInd].line;
        char* curLineBegin = curLine;
        
        ASSERT(curLine != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
        
        ERROR_STATUSES ReplaceAllErrorStatus = ReplaceAll(curLine, '\n', '\0');

        ASSERT(curLine != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

        ERROR_STATUSES TrimSpacesErrorStatus = TrimSpaces(&curLine);

        ASSERT(curLine != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

        DebugPrintLine(curLine);

        int lenExpr = 0;
        ARG_TYPES argType = NO_ARG;


        OPCODES opcode = NO_OPCODE;


        bool needJumpingIndex = false;
        
        if (strncmp(curLine, ";", strlen(";")) == 0) {
            ON_DEBUG(fprintf(stderr, "FOUND COMMENT STRING!\n");)

            MyStrcpyWithSep(ptrTextExec->buffer, ptrTextExec->index[curLineInd].line, curLine, &ptrTextExecBufferInd,
                            &(ptrTextExec->index[curLineInd].size));
            continue;
        }
        else if (strncmp(curLine, ":", strlen(":")) == 0) {
            ON_DEBUG(fprintf(stderr, "FOUND LABEL DECLARATION!\n");)

            MyStrcpyWithSep(ptrTextExec->buffer, ptrTextExec->index[curLineInd].line, curLine, &ptrTextExecBufferInd,
                &(ptrTextExec->index[curLineInd].size));
            continue;
        }
        else if (strncmp(curLine, "PUSH", strlen("PUSH")) == 0) {
            opcode = PUSH;
            lenExpr = strlen("PUSH");

            ERROR_STATUSES GetArgumentTypeErrorStatus = GetArgumentType(curLine, lenExpr, &argType);

            ASSERT(GetArgumentTypeErrorStatus == OK, GetErrorString(GetArgumentTypeErrorStatus),
                   GetArgumentTypeErrorStatus);
        }
        else if (strncmp(curLine, "POP", strlen("POP")) == 0) {
            opcode = POP;
            lenExpr = strlen("POP");

            ERROR_STATUSES GetArgumentTypeErrorStatus = GetArgumentType(curLine, lenExpr, &argType);

            ASSERT(GetArgumentTypeErrorStatus == OK, GetErrorString(GetArgumentTypeErrorStatus),
                   GetArgumentTypeErrorStatus);
        }
        else if (strncmp(curLine, "OUT", strlen("OUT")) == 0) {
            opcode = OUT;
            lenExpr = strlen("OUT");
        }
        else if (strncmp(curLine, "ADD", strlen("ADD")) == 0) {
            opcode = ADD;
            lenExpr = strlen("ADD");
        }
        else if (strncmp(curLine, "SUB", strlen("SUB")) == 0) {
            opcode = SUB;
            lenExpr = strlen("SUB");
        }
        else if (strncmp(curLine, "MUL", strlen("MUL")) == 0) {
            opcode = MUL;
            lenExpr = strlen("MUL");
        }
        else if (strncmp(curLine, "DIV", strlen("DIV")) == 0) {
            opcode = DIV;
            lenExpr = strlen("DIV");
        }
        else if (strncmp(curLine, "DUMP", strlen("DUMP")) == 0) {
            opcode = DUMP;
            lenExpr = strlen("DUMP");
        } 
        else if (strncmp(curLine, "SIN", strlen("SIN")) == 0) {
            opcode = SIN;
            lenExpr = strlen("SIN");
        } 
        else if (strncmp(curLine, "COS", strlen("COS")) == 0) {
            opcode = COS;
            lenExpr = strlen("COS");
        } 
        else if (strncmp(curLine, "SQUARE", strlen("SQUARE")) == 0) {
            opcode = SQUARE;
            lenExpr = strlen("SQUARE");
        } 
        else if (strncmp(curLine, "SQRT", strlen("SQRT")) == 0) {
            opcode = SQRT;
            lenExpr = strlen("SQRT");
        } 
        else if (strncmp(curLine, "HALT", strlen("HALT")) == 0) {
            opcode = HALT;
            lenExpr = strlen("HALT");
        } 
        else if (strncmp(curLine, "JMP", strlen("JMP")) == 0) {
            opcode = JMP;
            lenExpr = strlen("JMP");
            needJumpingIndex = true;
        } 
        else if (strncmp(curLine, "JAE", strlen("JAE")) == 0) {
            opcode = JAE;
            lenExpr = strlen("JAE");
            needJumpingIndex = true;
        }
        else if (strncmp(curLine, "JA", strlen("JA")) == 0) {
            opcode = JA;
            lenExpr = strlen("JA");
            needJumpingIndex = true;
        } 
        else if (strncmp(curLine, "JBE", strlen("JBE")) == 0) {
            opcode = JBE;
            lenExpr = strlen("JBE");
            needJumpingIndex = true;   
        } 
        else if (strncmp(curLine, "JB", strlen("JB")) == 0) {
            opcode = JB;
            lenExpr = strlen("JB");
            needJumpingIndex = true;
        } 
        else if (strncmp(curLine, "JNE", strlen("JNE")) == 0) {
            opcode = JNE;
            lenExpr = strlen("JNE");
            needJumpingIndex = true;
        }
        else if (strncmp(curLine, "JE", strlen("JE")) == 0) {
            opcode = JE;
            lenExpr = strlen("JE");
            needJumpingIndex = true;
        } 
        else if (strncmp(curLine, "CALL", strlen("CALL")) == 0) {
            opcode = CALL;
            lenExpr = strlen("CALL");
            needJumpingIndex = true;
        }
        else if (strncmp(curLine, "RET", strlen("RET")) == 0) {
            opcode = RET;
            lenExpr = strlen("RET");
        }
        else if (strncmp(curLine, "IN", strlen("IN")) == 0) {
            opcode = IN;
            lenExpr = strlen("IN");
        }
        else if (strcmp(curLine, "") == 0 && curLineInd == ptrTextSource->maxLines - 1) {

            ON_DEBUG(fprintf(stderr, "END OF FILE!\n");)

            ptrTextExec->buffer[ptrTextExecBufferInd] = '\0';
            
            ptrTextExecBufferInd++;

            ptrTextExec->index[curLineInd].line[0] = '\0';
            ptrTextExec->index[curLineInd].size = 1;
            
            ptrTextExec->bufferSize = ptrTextExecBufferInd;

            return OK;
        } 
        else {
            fprintf(stderr, GetErrorString(UNKNOWN_OPCODE));
            return UNKNOWN_OPCODE;
        }

        int ptrTextExecIndexInd = 0;
        
        char* opcodeString = NULL;
        
        ERROR_STATUSES MyIntToStringErrorStatus = MyIntToString(opcode, &opcodeString);
        ASSERT(MyIntToStringErrorStatus == OK, GetErrorString(INT_TO_STRING_ERROR), INT_TO_STRING_ERROR);
       
        
        strcpy(ptrTextExec->buffer + ptrTextExecBufferInd, opcodeString);
        strcpy(ptrTextExec->index[curLineInd].line + ptrTextExecIndexInd, opcodeString);
        
        ptrTextExecBufferInd += strlen(opcodeString);
        ptrTextExecIndexInd += strlen(opcodeString);

        if (needJumpingIndex) {
            ON_DEBUG(fprintf(stderr, "(Assemble): jump comand detected: curline + %i\n", lenExpr + 2););

            ON_DEBUG(fprintf(stderr, "(Assemble): jump comand detected: <%s>\n", curLine + lenExpr + 2););

                int labelInd = -1;
                
                if (sscanf(curLine + lenExpr + 2, "%i", &labelInd)) {
                    AddSpace(ptrTextExec->buffer, ptrTextExec->index[curLineInd].line,
                                   &ptrTextExecBufferInd, &ptrTextExecIndexInd);

                ASSERT(labelInd < MaxLabelsSize, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
                ASSERT(labels[labelInd] != -1, GetErrorString(NOT_INITIALIZED_LABEL), NOT_INITIALIZED_LABEL);
                
                char* labelAddressString = NULL;

                ERROR_STATUSES MyIntToStringErrorStatus = MyIntToString(labels[labelInd], &labelAddressString);
                ASSERT(MyIntToStringErrorStatus == OK, GetErrorString(MyIntToStringErrorStatus), MyIntToStringErrorStatus);

                ON_DEBUG(fprintf(stderr, "wanna insert label value: <%s>\n", labelAddressString););

                strcpy(ptrTextExec->buffer + ptrTextExecBufferInd, labelAddressString);
                strcpy(ptrTextExec->index[curLineInd].line + ptrTextExecIndexInd, labelAddressString);

                int labelIndLen = strlen(labelAddressString);

                ptrTextExecBufferInd += labelIndLen;
                ptrTextExecIndexInd  += labelIndLen;
                
                *(ptrTextExec->buffer + ptrTextExecBufferInd)                = '\n';
                *(ptrTextExec->index[curLineInd].line + ptrTextExecIndexInd) = '\n';
                
                ptrTextExecBufferInd++;
                ptrTextExecIndexInd++;
                ptrTextExec->index[curLineInd].size = ptrTextExecIndexInd;
                
                continue;
            }
        }

        curLine += lenExpr;

        
        
        if (argType != NO_ARG) {

            curLine += 1;

            AddSpace(ptrTextExec->buffer, ptrTextExec->index[curLineInd].line,
                                   &ptrTextExecBufferInd, &ptrTextExecIndexInd);

            
            char* argTypeString = NULL;
            
            ERROR_STATUSES MyIntToStringErrorStatus = MyIntToString((int)argType, &argTypeString);
            ASSERT(MyIntToStringErrorStatus == OK, GetErrorString(INT_TO_STRING_ERROR), INT_TO_STRING_ERROR);
            
            ON_DEBUG(fprintf(stderr, "(Assemble): found type of arguments <%s>\n", argTypeString);)

            strcpy(ptrTextExec->buffer + ptrTextExecBufferInd, argTypeString);
            strcpy(ptrTextExec->index[curLineInd].line + ptrTextExecIndexInd, argTypeString);
            
            ptrTextExecBufferInd += strlen(argTypeString);
            ptrTextExecIndexInd += strlen(argTypeString);
        }
        

        int registerLen = 0;

        if (argType == VARIABLE) {
            AddSpace(ptrTextExec->buffer, ptrTextExec->index[curLineInd].line,
                     &ptrTextExecBufferInd, &ptrTextExecIndexInd);

            char* registerNameString = curLine;

            ON_DEBUG(fprintf(stderr, "(Assemble): found register name <%s>\n", registerNameString);)

            REGISTERS_NAMES registerNameIndex = NO_NAME;
            
            ERROR_STATUSES registerNameStringToIndexErrorStatus = registerNameStringToIndex(registerNameString, 
                &registerNameIndex);
            
            ASSERT(registerNameStringToIndexErrorStatus == OK, GetErrorString(registerNameStringToIndexErrorStatus),
                registerNameStringToIndexErrorStatus);
            
            char* registerNameLine = NULL;

            ERROR_STATUSES MyIntToStringErrorStatus = MyIntToString((int)registerNameIndex, &registerNameLine);
            ASSERT(MyIntToStringErrorStatus == OK, GetErrorString(INT_TO_STRING_ERROR), INT_TO_STRING_ERROR);


            strcpy(ptrTextExec->buffer + ptrTextExecBufferInd, registerNameLine);
            strcpy(ptrTextExec->index[curLineInd].line + ptrTextExecIndexInd, registerNameLine);
            
            int registerNameLineLen = strlen(registerNameLine);

            ptrTextExecBufferInd += registerNameLineLen;
            ptrTextExecIndexInd  += registerNameLineLen;

            registerLen += strlen(registerNameString);
        }
        // exit(0);
        // if (NOT LineVerify(curLine)) {
        //     fprintf(stderr, GetErrorString(ASSEMBLE_VERIFICATION_ERROR), ASSEMBLE_VERIFICATION_ERROR);
        //     return ASSEMBLE_VERIFICATION_ERROR;
        // } TODO
        
        curLine += registerLen;

        AddSpace(ptrTextExec->buffer, ptrTextExec->index[curLineInd].line,
                &ptrTextExecBufferInd, &ptrTextExecIndexInd);

        ON_DEBUG(fprintf(stderr, "(Assemble): wanna copy <%s>\n", curLine);)
        ON_DEBUG(DebugPrintLine(curLine);)

        strcpy(ptrTextExec->buffer + ptrTextExecBufferInd,                curLine);
        // strcpy(ptrTextExec->index[curLineInd].line + ptrTextExecIndexInd, curLine);

        int addedLen = strlen(curLine);
        
        ptrTextExecBufferInd += addedLen;
        // ptrTextExecIndexInd  += addedLen;

        ptrTextExec->buffer[ptrTextExecBufferInd]                   = '\n';
        // ptrTextExec->index[curLineInd].line[ptrTextExecIndexInd] = '\n';

        ptrTextExecBufferInd++;

        // ptrTextExec->index[curLineInd].size = strlen(ptrTextExec->index[curLineInd].line) + 1;
    }

    ptrTextExec->buffer[ptrTextExecBufferInd] = '\0';
            
    ptrTextExecBufferInd++;

    ptrTextExec->bufferSize = ptrTextExecBufferInd;

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

ERROR_STATUSES MyStrlen(const char* string, int* res) {
    ASSERT(string != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int cnt = 0;
    for (; *string; string++, cnt++) {
        ;
    } 

    *res = cnt;

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

ERROR_STATUSES TrimSpaces(char** ptrLine) {
    ASSERT(*ptrLine != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    char* line = *ptrLine;

    for (; *line; line++) {
        if (*line != ' ') {
            *ptrLine = line;
            return OK;
        }
    }

    return OK;
}

ERROR_STATUSES GetArgumentType(const char* line, int lenExpr, ARG_TYPES* ptrArgType) {
    double arg = 0;

    if (!sscanf(line + lenExpr, "%lf", &arg)) {

        char* registerName = (char*)calloc(MaxRegisterNameLen, sizeof(char));

        ASSERT(registerName != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

        if (!sscanf(line + lenExpr, "%s", registerName)) {
            ASSERT(false, GetErrorString(READING_COMMAND_ASSEMBLE_ERROR), READING_COMMAND_ASSEMBLE_ERROR);
        }

        bool isValidName = false;
        
        ERROR_STATUSES IsValidRegisterNameErrorStatus = IsValidRegisterNameString(registerName, &isValidName);
        ASSERT(IsValidRegisterNameErrorStatus == OK, GetErrorString(IsValidRegisterNameErrorStatus),
               IsValidRegisterNameErrorStatus);

        if (!isValidName) {
            ASSERT(false, GetErrorString(UNKNOWN_REGISTER_NAME), UNKNOWN_REGISTER_NAME);
        }

        *ptrArgType = VARIABLE;
    } else {
        *ptrArgType = CONSTANT;
    }

    return OK;
}

ERROR_STATUSES registerNameStringToIndex(const char* line, REGISTERS_NAMES* registerName) {
    int registerNameSize = 3;

    if (strncmp(line, "RAX", registerNameSize) == 0) {
        *registerName = RAX;
    }
     else if (strncmp(line, "RBX", registerNameSize) == 0) {
        *registerName = RBX;
    }
     else if (strncmp(line, "RCX", registerNameSize) == 0) {
        *registerName = RCX;
    }
     else if (strncmp(line, "RDX", registerNameSize) == 0) {
        *registerName = RDX;
    }
     else if (strncmp(line, "REX", registerNameSize) == 0) {
        *registerName = REX;
    }
     else if (strncmp(line, "RFX", registerNameSize) == 0) {
        *registerName = RFX;
    }
     else if (strncmp(line, "RGX", registerNameSize) == 0) {
        *registerName = RGX;
    }
     else if (strncmp(line, "RJX", registerNameSize) == 0) {
        *registerName = RJX;
    }
     else {
        return UNKNOWN_REGISTER_NAME;
    }

    return OK;
}

ERROR_STATUSES IsValidRegisterNameString(const char* line, bool* res) {
    int registerNameSize = 3;

    if (strncmp(line, "RAX", registerNameSize) == 0 || strncmp(line, "RBX", registerNameSize) == 0 || 
        strncmp(line, "RCX", registerNameSize) == 0 || strncmp(line, "RDX", registerNameSize) == 0 || 
        strncmp(line, "REX", registerNameSize) == 0 || strncmp(line, "RFX", registerNameSize) == 0 || 
        strncmp(line, "RGX", registerNameSize) == 0 || strncmp(line, "RJX", registerNameSize) == 0   ) {
        *res = true;
        return OK;    
    }

    *res = false;
    return OK;
}

ERROR_STATUSES InitLabels(int* labels) {
    ASSERT(labels != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    
    for (int curInd = 0; curInd < MaxLabelsSize; curInd++) {
        labels[curInd] = -1;
    }

    return OK;
}

void AddSpace(char* buffer, char* line, int* ptrBufferInd, int* ptrLineInd) {
    const char* spaceString = " ";

    strcpy(buffer + *ptrBufferInd, spaceString);
    strcpy(line   + *ptrLineInd, spaceString);
    
    int spaceStringLen = 1;
    *ptrBufferInd += spaceStringLen;
    *ptrLineInd   += spaceStringLen;
}

void MyStrcpyWithSep(char* bufferDest, char* lineDest, char* src, int* ptrBufferInd, size_t* ptrLineSize) {
    int srcLen = strlen(src);

    strncpy(bufferDest + *ptrBufferInd, src, srcLen);
    strncpy(lineDest,                   src, srcLen);

    *ptrBufferInd += srcLen;
    *ptrLineSize   = srcLen;

    *(bufferDest + *ptrBufferInd)         = '\n';
    *(lineDest   + *ptrLineSize)          = '\n';

    (*ptrBufferInd)++;
    (*ptrLineSize)++;

    // ON_DEBUG(fprintf(stderr, "(MyStrcpyWithSep): src: <%s>\n", src);)

    // ON_DEBUG(fprintf(stderr, "(MyStrcpyWithSep): just copied buffer: <%s>\n", bufferDest);)
    // ON_DEBUG(fprintf(stderr, "(MyStrcpyWithSep): just copied line: <%s>\n", lineDest);)
}
