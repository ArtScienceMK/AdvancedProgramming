#ifdef DEBUG

#define ON_DEBUG(...)  __VA_ARGS__

#else

#define ON_DEBUG(...)

#endif

#define STRINGIFY( x ) (# x)
#define GET_SOURCE_LOCATION(sourceLocation, varName)\
    sourceLocation.file = strdup(__FILE__);\
    sourceLocation.function = strdup(__FUNCTION__);\
    sourceLocation.line = __LINE__;\
    sourceLocation.name = strdup(STRINGIFY(varName));

#include <fcntl.h>
#include <stdio.h>
#include "MyError.h"
#include "MyAssert.h"
#include <unistd.h>
#include <string.h>
#include <math.h>

int DUMP_CNT = 1;

const int MaxFilenameLen = 30;
const int MaxNumberLineSize = 3;

const char* VizDumpFilename =  "Dump";
const char* VizDumpExtension = ".png";

const char* DotFilename  = "dot.txt";
const int   DotFileSize  = 10000;
const int MaxDotLineSize = 100;

const char* HtmlFilename    = "Dump.html";
const int   HtmlFileSize    = 10000;
const int   MaxHtmlLineSize = 100;


const char* TxtFilename    = "Dump.txt";
const int   TxtFileSize    = 10000;
const int   MaxTxtLineSize = 100;

const char* DotFileTemplate = "digraph G{\n\trankdir=LR;\n\tnode[color=\"#000000\", style=filled, fillcolor=\"#00ff88\", fontsize=14, shape=\"Mrecord\"];\n";

const char* DotSystemCallTemplate = "dot dot.txt -Tpng -o ";

struct ListElem_t {
    int info;
    int next;
    int prev;
    int index;
};

struct List_t {
    size_t size;
    size_t capacity;
    ListElem_t* elems;
    int head;
    int tail;
};

ERROR_STATUSES ClearLine(char* line);
ERROR_STATUSES AddLine(     char* buffer, size_t* ptrBufferSize, const char* toAdd);
ERROR_STATUSES AddLineClean(char* buffer, size_t* ptrBufferSize, const char* toAdd);

ERROR_STATUSES GenerateDotFile(List_t* ptrList, char** ptrBuffer, size_t* ptrBufferSize);

ERROR_STATUSES GenerateDumpFilename(char** ptrFilename);

ERROR_STATUSES GenerateHtmlFile(List_t* ptrList, char** ptrBuffer, size_t* ptrBufferSize,
    int DUMP_CNT, SourceLocation sourceLocation);

ERROR_STATUSES GenerateTxtFileDump(List_t* ptrList, char** ptrBuffer, size_t* ptrBufferSize, SourceLocation sourceLocation);

ERROR_STATUSES UnlinkFile(const char* filename);

ERROR_STATUSES WriteBufferToFile(const char* filename, char* buffer, size_t bufferSize);

ERROR_STATUSES ListInit(List_t* ptrList, size_t _capacity);

void ListDestroy(List_t* ptrList);

ERROR_STATUSES ListVerify(List_t* ptrList);
ERROR_STATUSES NodeVerify(ListElem_t* ptrElem);

ERROR_STATUSES ListTextDump(List_t list, SourceLocation sourceLocation);
ERROR_STATUSES ListVizDump( List_t list, SourceLocation sourceLocation);
ERROR_STATUSES ListHtmlDump(List_t list, SourceLocation sourceLocation);

int NumLen(int num);

ERROR_STATUSES ListInsert(List_t* ptrList, int insertIndex, int value);
ERROR_STATUSES ListDelete(List_t* ptrList, int insertIndex);

bool isValidListIndex(int ind, int left, int right);


int main() {
    List_t list = {};
    size_t listCapacity = 2;

    ListInit(&list, listCapacity);

    ListInsert(&list, 0, 10);
    ListInsert(&list, 0, 20);
    ListInsert(&list, 0, 30);

    ListInsert(&list, 1, 40);
    ListInsert(&list, 1, 50);
    ListInsert(&list, 1, 60);

    ListInsert(&list, 1, 70);
    ListInsert(&list, 1, 80);
    ListInsert(&list, 1, 90);

    ListInsert(&list, 1, 100);
    ListInsert(&list, 1, 110);
    ListInsert(&list, 1, 120);

    // ListInsert(&list, 1, 60);

    list.elems[2].prev = -1;

    ListDelete(&list, 1);

    SourceLocation sourceLocation1 = {};
    GET_SOURCE_LOCATION(sourceLocation1, list)

    ListHtmlDump(list, sourceLocation1);

    // ListDelete(&list, 2);

    // SourceLocation sourceLocation2 = {};
    // GET_SOURCE_LOCATION(sourceLocation2, list)

    // ListVizDump(list, sourceLocation2);


    return 0;
}

ERROR_STATUSES ClearLine(char* line) {
    ASSERT(line != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    for (; *line != '\0'; line++) {
        *line = '\0';
    }

    return OK;
}

ERROR_STATUSES AddLine(char* buffer, size_t* ptrBufferSize, char* toAdd) {
    ASSERT(buffer        != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrBufferSize != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(toAdd         != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    strcpy(buffer + *ptrBufferSize, toAdd);
    
    int toAddLen = strlen(toAdd);
    *ptrBufferSize += toAddLen;

    return OK;
}

ERROR_STATUSES AddLineClean(char* buffer, size_t* ptrBufferSize, char* toAdd) {
    ERROR_STATUSES AddLineErrorStatus = AddLine(buffer, ptrBufferSize, toAdd);

    ASSERT(AddLineErrorStatus == OK, GetErrorString(AddLineErrorStatus),
           AddLineErrorStatus);

    ERROR_STATUSES ClearLineErrorStatus = ClearLine(toAdd);

    ASSERT(ClearLineErrorStatus == OK, GetErrorString(ClearLineErrorStatus),
           ClearLineErrorStatus);

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

ERROR_STATUSES WriteBufferToFile(const char* filename, char* buffer, size_t bufferSize) {
    ASSERT(filename != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    
    ASSERT(buffer   != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int fileDesc = open(filename, O_CREAT | O_WRONLY);  // Дед, почему нельзя ставить просто w

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    ON_DEBUG(fprintf(stderr, "(WriteBufferToFile): bufferSize: %i\n", bufferSize);)
    ON_DEBUG(fprintf(stderr, "(WriteBufferToFile): buffer:\n");)

    for (int i = 0; i < bufferSize; i++) {
        ON_DEBUG(fprintf(stderr, "<%c> has number %i\n", buffer[i], buffer[i]);)
    }
    // const char* s = "MEOW\n";
    write(fileDesc, buffer, sizeof(buffer[0]) * bufferSize);  //  Почему не пишет, коли в буфере все ок
    
    close(fileDesc);

    return OK;
}

ERROR_STATUSES ListInit(List_t* ptrList, size_t _capacity) {
    ASSERT(ptrList != NULL,         GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrList->size     == 0,      GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrList->capacity == 0,      GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);

    ASSERT(ptrList->head == 0,      GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrList->tail == 0,      GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);

    ptrList->capacity  = _capacity;
    ptrList->elems = (ListElem_t*)calloc(ptrList->capacity, sizeof(ptrList->elems[0])); 

    ASSERT(ptrList->elems != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    ptrList->elems[0] = {};

    for (int i = 1; i < ptrList->capacity; i++) {
        ptrList->elems[i].info  = 0;
        ptrList->elems[i].index = i;
        ptrList->elems[i].prev  = -1;
        ptrList->elems[i].next  = -1;
    }

    ptrList->size = 1;

    ptrList->head = 0;
    ptrList->tail = 0;

    return OK;
}

void ListDestroy(List_t* ptrList) {
    free(ptrList->elems);
    ptrList->head = -1;
    ptrList->tail = -1;
    ptrList->size = -1;
}

ERROR_STATUSES ListVerify(List_t* ptrList) {
    ASSERT(ptrList != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrList->size > 0, GetErrorString(BAD_LIST_SIZE), BAD_LIST_SIZE);
    ASSERT(ptrList->capacity > 0, GetErrorString(BAD_LIST_SIZE), BAD_LIST_SIZE);

    ASSERT(ptrList->size <= ptrList->capacity, GetErrorString(LIST_CAP_SIZE), LIST_CAP_SIZE);

    ASSERT(isValidListIndex(ptrList->head, 0, ptrList->size), GetErrorString(BAD_LIST_HEAD), BAD_LIST_HEAD);
    ASSERT(isValidListIndex(ptrList->tail, 0, ptrList->size) > 0, GetErrorString(BAD_LIST_TAIL), BAD_LIST_TAIL);

    ASSERT(ptrList->elems != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrList->elems[0].info == 0, GetErrorString(BAD_LIST_ZERO_ELEM), BAD_LIST_ZERO_ELEM);
    ASSERT(ptrList->elems[0].prev == 0, GetErrorString(BAD_LIST_ZERO_ELEM), BAD_LIST_ZERO_ELEM);
    ASSERT(ptrList->elems[0].next == 0, GetErrorString(BAD_LIST_ZERO_ELEM), BAD_LIST_ZERO_ELEM);

    
    bool* visited = (bool*)calloc(ptrList->size, sizeof(visited[0]));

    ASSERT(visited != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    int curInd = ptrList->head;

    while (curInd != ptrList->tail) {
        ASSERT(isValidListIndex(curInd, 0, ptrList->size), GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

        if (visited[curInd]) {
            ASSERT(false, GetErrorString(LIST_CYCLE), LIST_CYCLE);
        }

        visited[curInd] = true;

        curInd = ptrList->elems[curInd].next;
    }

    for (int i = 0; i < ptrList->size; i++) {
        visited[i] = false;
    }

    // while (curInd != ptrList-> head) {
    //     ASSERT(isValidListIndex(curInd, 0, ptrList->size), GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    //     if (visited[curInd]) {
    //         ASSERT(false, GetErrorString(LIST_CYCLE), LIST_CYCLE);
    //     }

    //     visited[curInd] = true;

    //     curInd = ptrList->elems[curInd].prev;
    // }

    return OK;
}

ERROR_STATUSES ElemVerify(ListElem_t* ptrElem, size_t capacity) {
    ASSERT(ptrElem != NULL,  GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrElem->index >= 0 && ptrElem->index < capacity; GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrElem->prev >= 0; GetErrorString(NO_EDGE_ERROR), NO_EDGE_ERROR);
    ASSERT(ptrElem->next >= 0; GetErrorString(NO_EDGE_ERROR), NO_EDGE_ERROR);

    return OK;
}

ERROR_STATUSES ListTextDump(List_t list, SourceLocation sourceLocation) {
    ListVerify(&list);

    char* buffer = NULL;
    size_t bufferSize = 0;

    ERROR_STATUSES GenerateTxtFileDumpErrorStatus = GenerateTxtFileDump(&list, &buffer, &bufferSize, sourceLocation);

    ASSERT(GenerateTxtFileDumpErrorStatus == OK, GetErrorString(GenerateTxtFileDumpErrorStatus),
           GenerateTxtFileDumpErrorStatus);

    ERROR_STATUSES UnlinkFileErrorStatus = UnlinkFile(TxtFilename);

    ASSERT(UnlinkFileErrorStatus == OK, GetErrorString(UnlinkFileErrorStatus), UnlinkFileErrorStatus);

    ERROR_STATUSES WriteBufferToFileErrorStatus = WriteBufferToFile(TxtFilename, buffer, bufferSize);

    ASSERT(WriteBufferToFileErrorStatus == OK, GetErrorString(WriteBufferToFileErrorStatus),
           WriteBufferToFileErrorStatus);
    
    ON_DEBUG(ListVerify(&list);)

    return OK;
}

ERROR_STATUSES GenerateDotFile(List_t* ptrList, char** ptrBuffer, int* ptrBufferSize) {

    char* buffer = (char*)calloc(DotFileSize, sizeof(buffer[0]));
    
    ASSERT(buffer != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
    
    size_t bufferInd = 0;

    strcpy(buffer + bufferInd, DotFileTemplate);
    bufferInd += strlen(DotFileTemplate);

    char* curLine = (char*)calloc(MaxDotLineSize, sizeof(curLine[0]));
   
    ASSERT(curLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    ERROR_STATUSES AddLineCleanErrorStatus = OK;

    sprintf(curLine, "\t 0 [color=\"#000000\", fillcolor=\"#ffea5f\", label=\"{ prev = %i | { index = %i | info = %i } | next = %i }\"];\n", 0,
            ptrList->elems[0].prev,ptrList->elems[0].index, ptrList->elems[0].info, ptrList->elems[0].next);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);

    for (size_t i = 1; i < ptrList->size; i++) {
        
        
        sprintf(curLine, "\t %i [label=\"{ prev = %i | { index = %i | info = %i } | next = %i }\"];\n", i,
                ptrList->elems[i].prev,ptrList->elems[i].index, ptrList->elems[i].info, ptrList->elems[i].next);
        
        AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

        ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);
    }

    for (size_t i = ptrList->size; i < ptrList->capacity; i++) {
        sprintf(curLine, "\t %zu [color=\"#000000\", fillcolor=\"#fc6666\", label=\"{prev = %i | { index = %i | info = %i } | next = %i }\"];\n", 
                i, ptrList->elems[i].prev, ptrList->elems[i].index,  ptrList->elems[i].info, ptrList->elems[i].next);
        
        AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

        ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);
    }

    sprintf(curLine, "\t HEAD [color=\"#000000\", fillcolor=\"#fb62ee\", label=\"HEAD\"];\n");

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);

    sprintf(curLine, "\t TAIL [color=\"#000000\", fillcolor=\"#fbff00\", label=\"TAIL\"];\n");

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);

    sprintf(curLine, "\t FREE [color=\"#000000\", fillcolor=\"#ff1e1e\", label=\"FREE\"];\n");

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);

    int curLineInd = 0;

    for (size_t i = 0; i < ptrList->capacity - 1; i++) {
        sprintf(curLine + curLineInd, "%zu->", i);
        curLineInd += NumLen(i) + strlen("->");
    }

    sprintf(curLine + curLineInd, "%zu[color=\"#ffffff\", weight=1000];\n", ptrList->capacity - 1);
    curLineInd += NumLen(ptrList->capacity - 1) + strlen("[color=\"#ffffff\", weight=1000];\n");

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);

    curLineInd = 0;
    
    for (size_t i = 0; i < ptrList->size; i++) {
        
        if (ptrList->elems[i].next >= 0 && ptrList->elems[i].prev >= 0) {
            sprintf(curLine, "%zu->%i[color=\"#9028ff\", dir=both];\n", i, ptrList->elems[i].next);
        } else {
            sprintf(curLine, "%zu->%i[color=\"#ff2222\", penwidth=5];\n", i, ptrList->elems[i].next);
        }

        AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

        ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);
    }

    ON_DEBUG(fprintf(stderr, "curLine: <%s>\n", curLine);)

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);

    
    sprintf(curLine, "HEAD->%i[color=\"#ff0077\", dir=both];\n", ptrList->head);

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);


    sprintf(curLine, "TAIL->%i[color=\"#eaff00\", dir=both];\n", ptrList->tail);

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);


    sprintf(curLine, "FREE->%i[color=\"#ff0000\", dir=both];\n", ptrList->size);

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);
    

    sprintf(curLine, "}");

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);

    *ptrBuffer     = buffer;
    *ptrBufferSize = bufferInd;

    return OK;
}

ERROR_STATUSES GenerateDumpFilename(char** ptrFilename) {
    // static int DUMP_CNT = 1;

    ASSERT(ptrFilename != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    char* curDumpFilename = (char*) calloc(MaxFilenameLen, sizeof(curDumpFilename[0]));

    int curDumpFilenameInd = 0;

    ASSERT(curDumpFilename != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    strcpy(curDumpFilename + curDumpFilenameInd, VizDumpFilename);
    curDumpFilenameInd += strlen(VizDumpFilename);

    char* numberLine = (char*) calloc(MaxNumberLineSize, sizeof(numberLine[0]));

    ASSERT(numberLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    sprintf(numberLine, "%i", DUMP_CNT);

    strcpy(curDumpFilename + curDumpFilenameInd, numberLine);
    curDumpFilenameInd += strlen(numberLine);

    strcpy(curDumpFilename + curDumpFilenameInd, VizDumpExtension);
    curDumpFilenameInd += strlen(VizDumpExtension);

    *ptrFilename = curDumpFilename;

    DUMP_CNT++;

    return OK;
}

ERROR_STATUSES GenerateHtmlFile(List_t* ptrList, char** ptrBuffer, size_t* ptrBufferSize, int DUMP_CNT, SourceLocation sourceLocation) {
    // No verification, because it's always call after calling functions, requiring verification

    char* buffer = (char*) calloc(HtmlFileSize, sizeof(buffer[0]));

    ASSERT(buffer != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    size_t bufferInd = 0;

    char* curLine = (char*) calloc(MaxHtmlLineSize, sizeof(curLine[0]));

    ASSERT(curLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    sprintf(curLine, "<pre>\n");

    ERROR_STATUSES AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);



    sprintf(curLine, "Dump %i\n", DUMP_CNT);

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);
           

    sprintf(curLine, "Was called from %s, %s\n", sourceLocation.function, sourceLocation.file);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);


    sprintf(curLine, "List_t <%s> at [%p] \n\n",
                sourceLocation.name, ptrList);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

    
    sprintf(curLine, "Head = %i\n", ptrList->head);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

    
    sprintf(curLine, "Tail = %i\n", ptrList->tail);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

    
    sprintf(curLine, "Size = %i\n", ptrList->size);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

        
    sprintf(curLine, "Capacity = %i\n\n", ptrList->capacity);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);
    

    sprintf(curLine, "Visual dump\n <img src=%s%i%s>", VizDumpFilename, DUMP_CNT - 1, VizDumpExtension);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

    sprintf(curLine, "Text dump\n <embed src=%s>", TxtFilename);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

    *ptrBuffer     = buffer;
    *ptrBufferSize = bufferInd;

    return OK;
}

ERROR_STATUSES GenerateTxtFileDump(List_t* ptrList, char** ptrBuffer, size_t* ptrBufferSize, SourceLocation sourceLocation) {
    ListVerify(ptrList);

    char* buffer = (char*) calloc(TxtFileSize, sizeof(buffer[0]));

    ASSERT(buffer != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    size_t bufferInd = 0;

    char* curLine = (char*) calloc(MaxTxtLineSize, sizeof(curLine[0]));

    ASSERT(curLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    sprintf(curLine,  "\n-----------\n(ListTextDump): Starting...\n");

    ERROR_STATUSES AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);


    sprintf(curLine,  "List_t <%s> [%p] %s at %s {\n",
                sourceLocation.name, ptrList, sourceLocation.function, sourceLocation.file);

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

    sprintf(curLine, "Head = %i\n", ptrList->head);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

    
    sprintf(curLine, "Tail = %i\n", ptrList->tail);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

    
    sprintf(curLine, "Size = %i\n", ptrList->size);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

        
    sprintf(curLine, "Capacity = %i\n\n", ptrList->capacity);
    
    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);
    
    // if (isatty(fileno(stderr))) {
    //     sprintf(curLine, "%s", GREEN);
    
    //     AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    //     ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
    //         AddLineCleanErrorStatus);
    // }

    for (size_t i = 0; i < ptrList->size; i++) {

        sprintf(curLine, "*[%zu] = {info: %i, next: %i, prev: %i}\n",
                i, ptrList->elems[i].info, ptrList->elems[i].next, 
                ptrList->elems[i].prev);
    
        AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

        ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
            AddLineCleanErrorStatus);
    }

    // if (isatty(fileno(stderr))) {
    //     printf(curLine, "%s", RED);
    
    //     AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    //     ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
    //         AddLineCleanErrorStatus);
    // }

    for (size_t i = ptrList->size; i + 1 < ptrList->capacity; i++) {
        sprintf(curLine, "*[%zu] = {info: %i, next: %i, prev: %i}\n",
                i, ptrList->elems[i].info, ptrList->elems[i].next, 
                ptrList->elems[i].prev);
    
        AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

        ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
            AddLineCleanErrorStatus);
    }

    // if (isatty(fileno(stderr))) {
    //     sprintf(curLine, "%s", RESET);
    
    //     AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    //     ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
    //         AddLineCleanErrorStatus);
    // }

    sprintf(curLine, "}\n");

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
        AddLineCleanErrorStatus);


    sprintf(curLine, "(ListTextDump): Ending...\n");

    AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
        AddLineCleanErrorStatus);

    *ptrBuffer     = buffer;
    *ptrBufferSize = bufferInd;

    return OK;
}

ERROR_STATUSES ListVizDump(List_t list, SourceLocation sourceLocation) {
    ListVerify(&list);

    char* buffer = NULL;

    int bufferSize = 0;

    ERROR_STATUSES GenerateDotFileErrorStatus = GenerateDotFile(&list, &buffer, &bufferSize);

    ASSERT(GenerateDotFileErrorStatus == OK, GetErrorString(GenerateDotFileErrorStatus),
           GenerateDotFileErrorStatus);

    ERROR_STATUSES UnlinkDotFileErrorStatus = UnlinkFile(DotFilename);

    ASSERT(UnlinkDotFileErrorStatus == OK, GetErrorString(UnlinkDotFileErrorStatus), UnlinkDotFileErrorStatus);

    ERROR_STATUSES WriteBufferToDotFileErrorStatus = WriteBufferToFile(DotFilename, buffer, bufferSize);

    ASSERT(WriteBufferToDotFileErrorStatus == OK, GetErrorString(WriteBufferToDotFileErrorStatus),
           WriteBufferToDotFileErrorStatus);

    char* dotCallLine = (char*) calloc(strlen(DotSystemCallTemplate) + MaxFilenameLen, 
                                       sizeof(dotCallLine[0]));
    
    ASSERT(dotCallLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
    
    int dotCallLineInd = 0;

    strcpy(dotCallLine + dotCallLineInd, DotSystemCallTemplate);
    dotCallLineInd += strlen(DotSystemCallTemplate);

    char* curDumpFilename = NULL;

    ERROR_STATUSES GenerateDumpFilenameErrorStatus = GenerateDumpFilename(&curDumpFilename);
    
    ASSERT(GenerateDumpFilenameErrorStatus == OK, GetErrorString(GenerateDumpFilenameErrorStatus), 
           GenerateDumpFilenameErrorStatus);
    
    ON_DEBUG(fprintf(stderr, "dump filename: <%s>\n", curDumpFilename);)

    strcpy(dotCallLine + dotCallLineInd, curDumpFilename);
    dotCallLineInd += strlen(curDumpFilename);

    ON_DEBUG(fprintf(stderr, "dotCallLineInd: <%s>\n", dotCallLine);)

    int SystemCallErrorStatus = system(dotCallLine);

    ASSERT(SystemCallErrorStatus == 0, GetErrorString(SYSTEM_CALL_ERROR), SYSTEM_CALL_ERROR);

    // LOG(fprintf(stderr, "\n-----------\n");)
    // LOG(fprintf(stderr, "(ListTextDump): Starting...\n");)

    // LOG(fprintf(stderr, "List_t <%s> [%p] %s at %s {\n",
    //             sourceLocation.name, &list, sourceLocation.function, sourceLocation.file);)  //  TODO make STRINGIFY

    // LOG(fprintf(stderr, "size     = %zu\n", list.size);)
    // LOG(fprintf(stderr, "capacity = %zu\n", list.capacity);)

    // LOG(fprintf(stderr, "head = %i\n",  list.head);)
    // LOG(fprintf(stderr, "tail = %i\n",  list.tail);)
    

    return OK;
}

ERROR_STATUSES ListHtmlDump(List_t list, SourceLocation sourceLocation) {
    ListVizDump( list, sourceLocation);
    ListTextDump(list, sourceLocation);

    char* buffer = NULL;

    size_t bufferSize = 0;

    ERROR_STATUSES GenerateHtmlFileErrorStatus = GenerateHtmlFile(&list, &buffer, &bufferSize, DUMP_CNT, sourceLocation);

    ASSERT(GenerateHtmlFileErrorStatus == OK, GetErrorString(GenerateHtmlFileErrorStatus),
           GenerateHtmlFileErrorStatus);
    
    ERROR_STATUSES WriteBufferToFileErrorStatus = WriteBufferToFile(HtmlFilename, buffer, bufferSize);

    ASSERT(WriteBufferToFileErrorStatus == OK, GetErrorString(WriteBufferToFileErrorStatus), 
           WriteBufferToFileErrorStatus);

    return OK;
}

int NumLen(int num) {
    if (num == 0) {
        return 1;
    }

    int res = 0;

    if (num < 0) {
        res++;
        num *= -1;
    }

    res += log10(num) + 1;
    
    ON_DEBUG(fprintf(stderr, "(NumLen) %i has len %i\n", num, res );)
    return res;
}

ERROR_STATUSES ListInsert(List_t* ptrList, int insIndex, int value) {
    ASSERT(ptrList != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(isValidListIndex(insIndex, 0, ptrList->size), GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    if (ptrList->size == ptrList->capacity) {
        ptrList->elems = (ListElem_t*) realloc(ptrList->elems, ptrList->capacity * 2 * sizeof(ptrList->elems[0]));

        ASSERT(ptrList->elems != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

        for (size_t i = ptrList->size; i < ptrList->capacity * 2; i++) {
            ptrList->elems[i].index = i;
        }

        ptrList->capacity *= 2;
    }

    int curFree = ptrList->size;

    // for (size_t i = 0; i < ptrList->size; i++) {
    //     if (ptrList->elems[i].next == -1 && ptrList->elems[i].prev == -1) {
    //         curFree = i;
    //         break;
    //     }
    // }

    // if (curFree == -1) {
    //     curFree = ptrList->size;
    // }

    ptrList->elems[curFree].info = value;

    ptrList->size++;

    int nextInd = ptrList->elems[insIndex].next;

    ptrList->elems[curFree].next = nextInd;
    ptrList->elems[nextInd].prev = curFree;

    ptrList->elems[curFree].prev = insIndex;
    ptrList->elems[insIndex].next = curFree;

    ptrList->head = ptrList->elems[0].next;
    ptrList->tail = ptrList->elems[0].prev;

    return OK;
}

ERROR_STATUSES ListDelete(List_t* ptrList, int delIndex) {
    ASSERT(ptrList != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(isValidListIndex(delIndex, 0, ptrList->size), GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    
    int prevInd = ptrList->elems[delIndex].prev;
    int nextInd = ptrList->elems[delIndex].next;
    
    if (prevInd != -1) {
        ptrList->elems[prevInd].next = nextInd;
    }

    if (nextInd != -1) {
        ptrList->elems[nextInd].prev = prevInd;
    }

    // ptrList->elems[delIndex].info = POISON;
    ptrList->elems[delIndex].prev = -1;
    ptrList->elems[delIndex].next = -1;

    ptrList->head = ptrList->elems[0].next;
    ptrList->tail = ptrList->elems[0].prev;


    return OK;
}

bool isValidListIndex(int ind, int left, int right) {
    return left <= ind && ind < right;
}

