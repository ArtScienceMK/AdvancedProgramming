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
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include "MyError.h"
#include "MyAssert.h"

int DUMP_CNT = 1;

const int MaxFilenameLen     = 30;
const int MaxNumberLineSize  = 3;
const int MaxUnicodeLineSize = 100;

const char* VizDumpFilename  =  "Dump";
const char* VizDumpExtension = ".png";

const char* DotFilename      = "dot.txt";
const int   DotFileSize      = 20000;
const int MaxDotLineSize     = 200;

const char* TreeFilename     = "data.trq";
const char* TreeSaveFilename = "dataSave.trq";

const int TreeFileSize       = 20000;
const int MaxTreeLineSize    = 200;
const int MaxObjectLineSize  = 30;

const char* DotFileTemplate = "digraph G{\n\trankdir=TB;\n\tnode[color=\"#000000\", style=filled, fillcolor=\"#00ff88\", fontsize=14, shape=\"Mrecord\"];\n";

const char* DotSystemCallTemplate = "dot dot.txt -Tpng -o ";

struct NodeElem_t {
    int value;
    char* line;
};


struct Node {
    NodeElem_t data;
    Node* left;
    Node* right;
    Node* parent;
};

struct Tree {
    int size;
    Node* root;
};

ERROR_STATUSES TreeInit(      Tree* ptrTree);
ERROR_STATUSES TreeDestroy(   Tree* ptrTree);
ERROR_STATUSES RecNodeDestroy(Node* ptrNode);

ERROR_STATUSES TreeSetInitialInt(   Tree* ptrTree, int value);
ERROR_STATUSES TreeSetInitialString(Tree* ptrTree, const char* value);
ERROR_STATUSES TreeInsertInt    (Tree* ptrTree, int value);
ERROR_STATUSES NodeInsertInt    (Node* ptrNode, int value);

ERROR_STATUSES NodeCreate      (Node* ptrParent, bool isLeftSon  );

ERROR_STATUSES TreeVerify(   Tree* ptrTree);
ERROR_STATUSES RecNodeVerify(Node* ptrNode);
ERROR_STATUSES NodeVerify(   Node* ptrNode);

ERROR_STATUSES TreeDump(     Tree* ptrTree);
ERROR_STATUSES RecNodeDump(  Node* ptrNode);

ERROR_STATUSES TreePrint(           Tree* ptrTree);
ERROR_STATUSES RecNodePrint(        Node* ptrNode);
ERROR_STATUSES RecNodePrintToLine(Node* ptrNode, char* curLine, size_t* ptrCurLineInd);

ERROR_STATUSES TreeVizDump(     Tree* ptrTree);
ERROR_STATUSES RecNodeVizDump(  Node* ptrNode, char* buffer, size_t* ptrBufferInd, char* curLine);

ERROR_STATUSES GenerateDotFile(Tree* ptrTree, char** ptrBuffer, size_t* ptrBufferSize);
ERROR_STATUSES GenerateDumpFilename(char** ptrFilename);

ERROR_STATUSES ClearLine(char* line);
ERROR_STATUSES AddLine(     char* buffer, size_t* ptrBufferSize, char* toAdd);
ERROR_STATUSES AddLineClean(char* buffer, size_t* ptrBufferSize, char* toAdd);

ERROR_STATUSES UnlinkFile(const char* filename);

ERROR_STATUSES WriteBufferToFile(const char* filename, char* buffer, size_t bufferSize);

ERROR_STATUSES StartUserCycle();
ERROR_STATUSES Guess(Tree* tree);

ERROR_STATUSES GenerateTreeFile(Tree* ptrTree, char** ptrBuffer, size_t* ptrBufferSize);
ERROR_STATUSES SaveTree(        Tree* ptrTree);
ERROR_STATUSES LoadTree(        Tree* ptrTree);

ERROR_STATUSES ReadFile(const char* filename, char** ptrBuffer, size_t* ptrBufferSize);
ERROR_STATUSES GetFileSize(int fileDesc, size_t* ptrFileSize);

ERROR_STATUSES CropLastChar(char* curLine, size_t* ptrCurLineInd);
ERROR_STATUSES GetVizLen(   char* curLine, size_t* res);

int main() {    
    StartUserCycle();

    return 0;
}


ERROR_STATUSES TreeInit(Tree* ptrTree) {
    ASSERT(ptrTree->size == 0   , GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrTree->root == NULL, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);

    ptrTree->size = 1;
    ptrTree->root = (Node*)calloc(1, sizeof(*(ptrTree->root)));

    ASSERT(ptrTree->root != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    ASSERT(ptrTree->root != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);


    ptrTree->root->parent = ptrTree->root;  // Корень - сам себе родитель (BUG)
    
    return OK;
}   

ERROR_STATUSES TreeDestroy(Tree* ptrTree) {
    ASSERT(ptrTree != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ERROR_STATUSES RecNodeDestroyErrorStatus = RecNodeDestroy(ptrTree->root);

    ASSERT(RecNodeDestroyErrorStatus == OK, GetErrorString(RecNodeDestroyErrorStatus),
           RecNodeDestroyErrorStatus);

    //  Как освобождать указатель на дерево, не трогая  root??? Только кастомная обертка на free??
    
    return OK;
}

ERROR_STATUSES RecNodeDestroy(Node* ptrNode) {
    ASSERT(ptrNode != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    if (ptrNode->left != NULL) {
        RecNodeDestroy(ptrNode->left);
    }
    if (ptrNode->right != NULL) {
        RecNodeDestroy(ptrNode->right);
    }

    free(ptrNode);

    return OK;
}

ERROR_STATUSES TreeSetInitialInt(Tree* ptrTree, int value) {
    ASSERT(ptrTree             != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrTree->root       != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ptrTree->root->data.value = value;

    return OK;
}

ERROR_STATUSES TreeSetInitialString(Tree* ptrTree, const char* line) {
    ASSERT(ptrTree             != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrTree->root       != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ptrTree->root->data.line = strdup(line);

    return OK;
}

ERROR_STATUSES TreeInsertInt(Tree* ptrTree, int value) {
    ASSERT(ptrTree != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ERROR_STATUSES NodeInsertErrorStatus = NodeInsertInt(ptrTree->root, value);

    ASSERT(NodeInsertErrorStatus == OK, GetErrorString(NodeInsertErrorStatus),
            NodeInsertErrorStatus);

    ptrTree->size = ptrTree->size + 1;

    return OK;
}

ERROR_STATUSES NodeInsertInt(Node* ptrNode, int value) {
    
    ERROR_STATUSES NodeVerifyErrorStatus = NodeVerify(ptrNode);

    ASSERT(NodeVerifyErrorStatus == OK, GetErrorString(NodeVerifyErrorStatus),
           NodeVerifyErrorStatus);

    bool isLeftSon = false;
    
    while (!(ptrNode->left == NULL && ptrNode->right == NULL)) {        
        if (value < ptrNode->data.value) {
            isLeftSon = true;
            ptrNode = ptrNode->left;
        } else {
            isLeftSon = false;
            ptrNode = ptrNode->right;
        }
    }

    fprintf(stderr, "(NodeInsertInt): ptrNode: %p\n", ptrNode);

    ERROR_STATUSES NodeCreateErrorStatus = NodeCreate(ptrNode, isLeftSon);

    if (isLeftSon) {
        ptrNode->left->data.value = value;
    } else {
        ptrNode->right->data.value = value;
    }

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    return OK;
}

ERROR_STATUSES NodeCreate(Node* ptrParent, bool isLeftSon) {

    ERROR_STATUSES NodeVerifyErrorStatus = NodeVerify(ptrParent);

    ASSERT(NodeVerifyErrorStatus == OK, GetErrorString(NodeVerifyErrorStatus),
           NodeVerifyErrorStatus);

    Node* ptrNode = (Node*)calloc(1, sizeof(*ptrNode));

    ASSERT(ptrNode != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    ptrNode->parent     = ptrParent;

    if (isLeftSon) {
        ptrParent->left = ptrNode;
    } else {
        ptrParent->right = ptrNode;
    }

    fprintf(stderr, "(NodeCreate): added ptrNode: %p, has parent: %p\n", ptrNode, ptrParent);
    fprintf(stderr, "(NodeCreate): isLeftSon: %i\n", isLeftSon);

    return OK;
}

ERROR_STATUSES RecNodeVerify(Node* ptrNode) {
    ERROR_STATUSES NodeVerifyErrorStatus = NodeVerify(ptrNode);

    ASSERT(NodeVerifyErrorStatus == OK, GetErrorString(NodeVerifyErrorStatus),
           NodeVerifyErrorStatus);

    if (ptrNode->left != NULL) {
        NodeVerifyErrorStatus = RecNodeVerify(ptrNode->left);

        ASSERT(NodeVerifyErrorStatus == OK, GetErrorString(NodeVerifyErrorStatus),
               NodeVerifyErrorStatus);
    }
    if (ptrNode->right != NULL) {
        NodeVerifyErrorStatus = RecNodeVerify(ptrNode->right);

        ASSERT(NodeVerifyErrorStatus == OK, GetErrorString(NodeVerifyErrorStatus),
               NodeVerifyErrorStatus);
    }

    return OK;
}

ERROR_STATUSES NodeVerify(Node* ptrNode) {
    ASSERT(ptrNode         != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    // ASSERT(ptrNode->left   != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    // ASSERT(ptrNode->right  != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrNode->parent != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    return OK;
}

ERROR_STATUSES TreeVerify(Tree* ptrTree) {
    ASSERT(ptrTree         != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrTree->size   > 0, GetErrorString(TREE_BAD_SIZE), TREE_BAD_SIZE);

    ASSERT(ptrTree->root   != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    return RecNodeVerify(ptrTree->root);
}

ERROR_STATUSES TreeDump(Tree* ptrTree) {
    ERROR_STATUSES TreeVerifyErrorStatus = TreeVerify(ptrTree);

    ASSERT(TreeVerifyErrorStatus == OK, GetErrorString(TreeVerifyErrorStatus),
           TreeVerifyErrorStatus);
    
    return RecNodeDump(ptrTree->root);
}

ERROR_STATUSES RecNodeDump(Node* ptrNode) {
    fprintf(stderr, "Node at %p\n", ptrNode);
    fprintf(stderr, "Stores %i, %s\n", ptrNode->data.value, ptrNode->data.line);
    fprintf(stderr, "Left son: %p, right son: %p, parent: %p\n", ptrNode->left, ptrNode->right, ptrNode->parent);

    if (ptrNode->left != NULL) {
        RecNodeDump(ptrNode->left);
    }
    if (ptrNode->right != NULL) {
        RecNodeDump(ptrNode->right);
    }
    return OK;
}

ERROR_STATUSES TreePrint(Tree* ptrTree) {
    ERROR_STATUSES TreeVerifyErrorStatus = TreeVerify(ptrTree);

    ASSERT(TreeVerifyErrorStatus == OK, GetErrorString(TreeVerifyErrorStatus),
           TreeVerifyErrorStatus);
    
    return RecNodePrint(ptrTree->root);
}

ERROR_STATUSES RecNodePrint(Node* ptrNode) {
    fprintf(stderr, "(%s", ptrNode->data.line);

    if (ptrNode->left != NULL) {
        RecNodePrint(ptrNode->left);
    }
    if (ptrNode->right != NULL) {
        RecNodePrint(ptrNode->right);
    }

    fprintf(stderr, ")");

    return OK;
}

ERROR_STATUSES RecNodePrintToLine(Node* ptrNode, char* curLine, size_t* ptrCurLineInd) {
    ASSERT(curLine       != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrCurLineInd != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    fprintf(stderr, "(RecNodePrintToLine): START ptrNode: %p, curLine: <%s>, curLineInd: %zu\n", 
                      ptrNode, curLine, *ptrCurLineInd);

    size_t curLineInd = *ptrCurLineInd;

    sprintf(curLine + curLineInd, "(");
    curLineInd++;

    if (!ptrNode) {

        sprintf(curLine + curLineInd, "nil)");
        curLineInd += strlen("nil)");

        *ptrCurLineInd = curLineInd;
        
        fprintf(stderr, "(RecNodePrintToLine): IS LEAF ptrNode: %p, curLine: <%s>, curLineInd: %zu\n", 
                          ptrNode, curLine, *ptrCurLineInd);

        return OK;
    }

    sprintf(curLine + curLineInd, "\"%s\" ", ptrNode->data.line);
    curLineInd += 3 + strlen(ptrNode->data.line);

    *ptrCurLineInd = curLineInd;

    fprintf(stderr, "(RecNodePrintToLine): BEFORE CALL ptrNode: %p, curLine: <%s>, curLineInd: %zu\n", 
                      ptrNode, curLine, *ptrCurLineInd);

    if (ptrNode->left != NULL) {
        RecNodePrintToLine(ptrNode->left, curLine, ptrCurLineInd);
    }
    if (ptrNode->right != NULL) {
        RecNodePrintToLine(ptrNode->right, curLine, ptrCurLineInd);
    }

    curLineInd = *ptrCurLineInd;

    sprintf(curLine + curLineInd, ")");
    curLineInd++;

    *ptrCurLineInd = curLineInd;

    fprintf(stderr, "(RecNodePrintToLine): END ptrNode: %p, curLine: <%s>, curLineInd: %zu\n", 
                      ptrNode, curLine, *ptrCurLineInd);


    return OK;
}

ERROR_STATUSES TreeVizDump(Tree* ptrTree) {
    ERROR_STATUSES TreeVerifyErrorStatus = TreeVerify(ptrTree);

    ASSERT(TreeVerifyErrorStatus == OK, GetErrorString(TreeVerifyErrorStatus),
           TreeVerifyErrorStatus);
    
    char* buffer = NULL;

    size_t bufferSize = 0;

    ERROR_STATUSES GenerateDotFileErrorStatus = GenerateDotFile(ptrTree, &buffer, &bufferSize);

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

    return OK;
}

ERROR_STATUSES RecNodeVizDump(Node* ptrNode, char* buffer, size_t* ptrBufferInd, char* curLine) {

    sprintf(curLine, "\t %zu [color=\"#000000\", fillcolor=\"#ffea5f\", label=\"{address = %p | info = %s }\"];\n",
            ptrNode, ptrNode, ptrNode->data.line);
    
    ERROR_STATUSES AddLineCleanErrorStatus = AddLineClean(buffer, ptrBufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

    if (ptrNode->left != NULL) {
        RecNodeVizDump(ptrNode->left, buffer, ptrBufferInd, curLine);

        sprintf(curLine, "\t %zu->%zu[color=\"#3ba0ff\", label=\"нет\"];\n", ptrNode, ptrNode->left);

        AddLineCleanErrorStatus = AddLineClean(buffer, ptrBufferInd, curLine);

        ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);
    }
    if (ptrNode->right != NULL) {
        RecNodeVizDump(ptrNode->right, buffer, ptrBufferInd, curLine);

        sprintf(curLine, "\t %zu->%zu[color=\"#ff3b3b\", label=\"да\"];\n", ptrNode, ptrNode->right);

        AddLineCleanErrorStatus = AddLineClean(buffer, ptrBufferInd, curLine);

        ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);
    }

    return OK;
}

ERROR_STATUSES GenerateDotFile(Tree* ptrTree, char** ptrBuffer, size_t* ptrBufferSize) {

    char* buffer = (char*)calloc(DotFileSize, sizeof(buffer[0]));
    
    ASSERT(buffer != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
    
    size_t bufferInd = 0;

    strcpy(buffer + bufferInd, DotFileTemplate);
    bufferInd += strlen(DotFileTemplate);

    char* curLine = (char*)calloc(MaxDotLineSize, sizeof(curLine[0]));
   
    ASSERT(curLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    ERROR_STATUSES RecNodeVizDumpErrorStatus = RecNodeVizDump(ptrTree->root, buffer, &bufferInd, curLine);    

    ASSERT(RecNodeVizDumpErrorStatus == OK, GetErrorString(RecNodeVizDumpErrorStatus), 
           RecNodeVizDumpErrorStatus);

    sprintf(curLine, "}");

    ERROR_STATUSES AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

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

ERROR_STATUSES StartUserCycle() {
    printf("------------\n");
    printf("Трепещи! Это акИНАТОР - передовая нейросеть!\n"  );
    printf("Введи число запросов:                       \n"  );
    
    int nQueries = 0;

    if (!scanf("%d", &nQueries) || nQueries <= 0) {
        printf("Введено некорректное число запросов... Самоуничтожение...\n");
        return OK;
    }

    Tree tree = {};

    ERROR_STATUSES TreeInitErrorStatus = TreeInit(&tree);

    ASSERT(TreeInitErrorStatus == OK, GetErrorString(TreeInitErrorStatus),
        TreeInitErrorStatus);

    ERROR_STATUSES LoadTreeErrorStatus = LoadTree(&tree);

    ASSERT(LoadTreeErrorStatus == OK, GetErrorString(LoadTreeErrorStatus), LoadTreeErrorStatus);

    char* inputLine = (char*)calloc(MaxUnicodeLineSize, sizeof(*inputLine));

    ASSERT(inputLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    size_t inputLineSize = MaxObjectLineSize;
    size_t inputLineInd  = 0;

    for (int i = 0; i < nQueries; i++) {
        printf("Что ты желаешь сделать?\n");
        printf("[у]гадать объект, [п]осмотреть базу данных, [в]ыйти\n");

        ClearLine(inputLine);
        inputLineInd = 0;

        if ((inputLineInd = getline(&inputLine, &inputLineSize, stdin)) == -1) {
            printf("Введена неверная опция... Самоуничтожение...\n");
            return AKINATOR_MENU_INPUT_ERROR;
        } 

        CropLastChar(inputLine, &inputLineInd);

        fprintf(stderr, "string after cropping: <%s>\n", inputLine);

        if (strcmp(inputLine, "у") == 0) {
            Guess(&tree);            
        }
        else if (strcmp(inputLine, "п") == 0) {
            printf("г[рафический] или т[екстовый] дамп?\n");
            
            ClearLine(inputLine);
            inputLineInd = 0;
            
            if ((inputLineInd = getline(&inputLine, &inputLineSize, stdin)) == -1) {
                printf("Введена неверная опция... Самоуничтожение...\n");
                return AKINATOR_MENU_INPUT_ERROR;
            }

            CropLastChar(inputLine, &inputLineInd);

            if (strcmp(inputLine, "г") == 0) {
                TreeVizDump(&tree);            
            } 
            else if (strcmp(inputLine, "т") == 0) {
                TreeDump(&tree);
            } 
            else {
                return AKINATOR_MENU_INPUT_ERROR;
            }
        }
        else if (strcmp(inputLine, "в") == 0) {
            printf("Сохранить базу данных?\n");

            ClearLine(inputLine);
            inputLineInd = 0;

            if ((inputLineInd = getline(&inputLine, &inputLineSize, stdin)) == -1) {
                printf("Введена неверная опция... Самоуничтожение...\n");
                return AKINATOR_MENU_INPUT_ERROR;
            }

            CropLastChar(inputLine, &inputLineInd);

            if (strcmp(inputLine, "да") == 0) {
                ERROR_STATUSES SaveTreeErrorStatus = SaveTree(&tree);

                ASSERT(SaveTreeErrorStatus == OK, GetErrorString(SaveTreeErrorStatus),
                       SaveTreeErrorStatus);
                
                printf("Дерево успешно сохранено!\n");
            }
            else if (strcmp(inputLine, "нет") == 0) {
                printf("Дерево удалено!\n");
            }
            else {
                printf("Некорректный ответ на вопрос! Самоуничтожение...\n");
                return UNKNOWN_AKINATOR_ANSWER_OPTION;
            }

            printf("Пока!\n");
            return OK;
        } else {
            printf("Введена неизвестная опция... Последнее предупреждение...\n");

            // return UNKNOWN_AKINATOR_MENU_OPTION;
        }
    }

    return OK;
}

ERROR_STATUSES Guess(Tree* ptrTree) {
    ERROR_STATUSES TreeVerifyErrorStatus = TreeVerify(ptrTree);

    ASSERT(TreeVerifyErrorStatus == OK, GetErrorString(TreeVerifyErrorStatus),
           TreeVerifyErrorStatus);

    Node* curNode  = ptrTree->root;

    char* inputLine = (char*)calloc(MaxUnicodeLineSize, sizeof(*inputLine));

    ASSERT(inputLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
    
    size_t inputLineSize = 0;
    size_t inputLineInd  = 0;

    while (!(curNode->left == NULL && curNode->right == NULL)) {

        fprintf(stderr, "(Guess): curNode: %p\n", curNode);

        printf("%s?\n", curNode->data.line);

        ClearLine(inputLine);
        inputLineInd = 0;

        if ((inputLineInd = getline(&inputLine, &inputLineSize, stdin)) == -1) {
            printf("Не смог считать ответ на вопрос! Самоуничтожение...\n");
            return AKINATOR_QUESTION_INPUT_ERROR;
        }

        CropLastChar(inputLine, &inputLineInd);

        if (strcmp(inputLine, "да") == 0) {
            if (curNode->right == NULL) {
                break;
            }

            curNode = curNode->right;
            
            fprintf(stderr, "(Guess): going to: %p\n", curNode);
        } 
        else if (strcmp(inputLine, "нет") == 0) {
            if (curNode->left == NULL) {
                break;
            }

            curNode = curNode->left;

            fprintf(stderr, "(Guess): going to: %p\n", curNode);
        }
        else {
            printf("Некорректный ответ на вопрос! Самоуничтожение...\n");
            return UNKNOWN_AKINATOR_ANSWER_OPTION;
        }
    }

    // Т.к. мы можем попасть в NULL, а у неё нельзя по parent обратиться

    printf("Это %s?\n", curNode->data.line);

    ClearLine(inputLine);
    inputLineInd = 0;

    if ((inputLineInd = getline(&inputLine, &inputLineSize, stdin)) == -1) {
        printf("Не смог считать ответ на вопрос! Самоуничтожение...\n");
        return AKINATOR_QUESTION_INPUT_ERROR;
    }

    CropLastChar(inputLine, &inputLineInd);

    if (strcmp(inputLine, "да") == 0) {
        printf("Я был прав! Теперь я повелитель Триштатья!\n");
        return OK;
    } 
    else if (strcmp(inputLine, "нет") == 0) {
        printf("Не может быть! Помоги мне стать умнее: скажи, что это было?\n");
        
        ClearLine(inputLine);
        inputLineInd = 0;

        if ((inputLineInd = getline(&inputLine, &inputLineSize, stdin)) == -1) {
            printf("Некорректный ввод... Самоуничтожение...\n");
            return UNKNOWN_AKINATOR_ANSWER_OPTION;
        }

        CropLastChar(inputLine, &inputLineInd);

        printf("А чем %s отличается от %s? %s ...", inputLine, curNode->data.line, inputLine);

        char* traitLine = (char*)calloc(MaxUnicodeLineSize, sizeof(*traitLine));

        ASSERT(traitLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

        size_t traitLineInd = 0;

        char* otherLine = strdup(curNode->data.line);

        if ((traitLineInd = getline(&traitLine, &traitLineInd, stdin)) == -1) {
            printf("Некорректный ввод... Самоуничтожение...\n");
            return UNKNOWN_AKINATOR_ANSWER_OPTION;
        }

        CropLastChar(traitLine, &traitLineInd);

        curNode->data.line = strdup(traitLine);

        ERROR_STATUSES NodeCreateErrorStatus = NodeCreate(curNode, 1);
        
        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), NodeCreateErrorStatus);

        curNode->left->data.line = strdup(otherLine);
        
        NodeCreateErrorStatus = NodeCreate(curNode, 0);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), NodeCreateErrorStatus);

        curNode->right->data.line = inputLine;

        printf("Я запомнил!\n");
    }
    else {
        printf("Некорректный ответ на вопрос! Самоуничтожение...\n");
        return UNKNOWN_AKINATOR_ANSWER_OPTION;
    }

    return OK;
}

ERROR_STATUSES SaveTree(Tree* ptrTree) {
    ERROR_STATUSES TreeVerifyErrorStatus = TreeVerify(ptrTree);

    ASSERT(TreeVerifyErrorStatus == OK, GetErrorString(TreeVerifyErrorStatus),
           TreeVerifyErrorStatus);
    
    char* buffer = NULL;

    size_t bufferSize = 0;

    ERROR_STATUSES GenerateTreeFileErrorStatus = GenerateTreeFile(ptrTree, &buffer, &bufferSize);

    ASSERT(GenerateTreeFileErrorStatus == OK, GetErrorString(GenerateTreeFileErrorStatus),
           GenerateTreeFileErrorStatus);

    ERROR_STATUSES UnlinkFileErrorStatus = UnlinkFile(TreeSaveFilename);

    ASSERT(UnlinkFileErrorStatus == OK, GetErrorString(UnlinkFileErrorStatus), UnlinkFileErrorStatus);

    ERROR_STATUSES WriteBufferToFileErrorStatus = WriteBufferToFile(TreeSaveFilename, buffer, bufferSize);

    ASSERT(WriteBufferToFileErrorStatus == OK, GetErrorString(WriteBufferToFileErrorStatus),
           WriteBufferToFileErrorStatus);

    return OK;
}

ERROR_STATUSES LoadTree(Tree* ptrTree) {
    char* buffer      = NULL;
    size_t bufferSize = 0;

    ERROR_STATUSES ReadFileErrorStatus = ReadFile(TreeFilename, &buffer, &bufferSize);

    ASSERT(ReadFileErrorStatus == OK, GetErrorString(ReadFileErrorStatus), ReadFileErrorStatus);

    fprintf(stderr, "(LoadTree): buffer: <%s>\n", buffer);

    char* curLine = (char*)calloc(MaxObjectLineSize, sizeof(*curLine));

    ASSERT(curLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    size_t bufferInd = 1;  //  wanna skip first '(', because we are already in root

    Tree tree = {};
    TreeInit(&tree);

    Node* curNode = ptrTree->root;

    int nNodes = 1;

    fprintf(stderr, "bufferSize: %i\n", bufferSize);

    for (; bufferInd < bufferSize && *(buffer + bufferInd) != '\0'; ) {
        ClearLine(curLine);
        
        if (*(buffer + bufferInd) == ')') {
            fprintf(stderr, "(LoadTree): Added )\n");

            curNode = curNode->parent;
            
            bufferInd++;
            continue;
        }

        if (*(buffer + bufferInd) == '(') {
            fprintf(stderr, "(LoadTree): going to new node!\n");
            fprintf(stderr, "(LoadTree): curNode: %p, left: %p, right: %p!\n", 
                           curNode, curNode->left, curNode->right);

            
            bool isLeftSon = false;

            if (curNode->left == NULL) {
                isLeftSon = true;
            } 
            else {
                isLeftSon = false;
            }

            ERROR_STATUSES NodeCreateErrorStatus = NodeCreate(curNode, isLeftSon);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), NodeCreateErrorStatus);

            nNodes++;

            bufferInd++;

            if (isLeftSon) {
                curNode = curNode->left;
            } else {
                curNode = curNode->right;
            }
        }

        if (!sscanf(buffer + bufferInd, "\"%[^\"]\"", curLine)) {
            fprintf(stderr, "(LoadTree): buffer: <%s>\n", buffer + bufferInd);

            if (sscanf(buffer + bufferInd, "%[^)]", curLine) && strcmp(curLine, "nil") == 0) {
                fprintf(stderr, "(LoadTree): curLine: <%s>\n", curLine);
                fprintf(stderr, "(LoadTree): (nil) found!\n");
                
                bufferInd += 3;
                continue;
            }
            return UNKNOWN_STRING_LOAD_TREE;            
        } 

        fprintf(stderr, "(LoadTree): bufferInd before: %i, curLine: <%s>\n", bufferInd, curLine);
        
        // *(curLine + strlen(curLine) - 1) =1 '\0';

        curNode->data.line = strdup(curLine);

        fprintf(stderr, "(LoadTree): curNode: %p, added line: <%s>\n", curNode, curNode->data.line);
        
        bufferInd += 3 + strlen(curLine);

        fprintf(stderr, "(LoadTree): bufferInd after : %i\n", bufferInd);
    }

    ptrTree->size = nNodes;

    return OK;
}

ERROR_STATUSES ReadFile(const char* filename, char** ptrBuffer, size_t* ptrBufferSize) {
    ASSERT(filename != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    int fileDesc = open(filename, O_RDONLY);

    ASSERT(fileDesc >= 0, GetErrorString(FILE_ACCESS_DENIED), FILE_ACCESS_DENIED);

    size_t fileSize = 0;

    ERROR_STATUSES GetFileSizeErrorStatus = GetFileSize(fileDesc, &fileSize);

    ASSERT(GetFileSizeErrorStatus == OK, GetErrorString(GetFileSizeErrorStatus),
           GetFileSizeErrorStatus);

    size_t bufferSize = fileSize + sizeof(char);

    LOG(fprintf(stderr, "bufferSize: %zu\n", bufferSize);)

    char* buffer = (char*)calloc(bufferSize, sizeof(buffer[0]));
    
    int charsRead = read(fileDesc, buffer, sizeof(buffer[0]) * bufferSize);
    // fclose(ptrFile);

    buffer[bufferSize - 1] = '\0';

    *ptrBuffer     = buffer;
    *ptrBufferSize = bufferSize;

    return OK;
}

ERROR_STATUSES GetFileSize(int fileDesc, size_t* ptrFileSize) {
    struct stat fileStats = {};

    // fprintf(stderr, "File descriptor: %i\n", fileno(ptrFile));

    int fstatStatus = fstat(fileDesc, &fileStats);

    ASSERT(fstatStatus == 0, GetErrorString(FILE_FSTAT_ERROR), FILE_FSTAT_ERROR);

    *ptrFileSize = fileStats.st_size;
    
    return OK;
}


ERROR_STATUSES GenerateTreeFile(Tree* ptrTree, char** ptrBuffer, size_t* ptrBufferSize) {
    char* buffer = (char*)calloc(TreeFileSize, sizeof(buffer[0]));
    
    ASSERT(buffer != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
    
    size_t bufferInd = 0;

    char* curLine = (char*)calloc(MaxTreeLineSize, sizeof(curLine[0]));
   
    ASSERT(curLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    size_t curLineInd = 0;

    ERROR_STATUSES RecNodePrintToLineErrorStatus = RecNodePrintToLine(ptrTree->root, curLine, &curLineInd);

    ASSERT(RecNodePrintToLineErrorStatus == OK, GetErrorString(RecNodePrintToLineErrorStatus), 
           RecNodePrintToLineErrorStatus);

    fprintf(stderr, "curLine after all recursive calls: <%s>\n", curLine);

    // if (ptrTree->root->left != NULL) {
        //     RecNodePrintToLineErrorStatus = RecNodePrintToLine(ptrTree->root->left, buffer, &bufferInd, curLine);    

        // ASSERT(RecNodePrintToLineErrorStatus == OK, GetErrorString(RecNodePrintToLineErrorStatus), 
        //    RecNodePrintToLineErrorStatus);
    // }
    // if (ptrTree->root->right != NULL) {
    //         RecNodePrintToLineErrorStatus = RecNodePrintToLine(ptrTree->root->right, buffer, &bufferInd, curLine);    

    //     ASSERT(RecNodePrintToLineErrorStatus == OK, GetErrorString(RecNodePrintToLineErrorStatus), 
    //        RecNodePrintToLineErrorStatus);
    // }

    ERROR_STATUSES AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);

    *ptrBuffer     = buffer;
    *ptrBufferSize = bufferInd;

    return OK;
}

ERROR_STATUSES CropLastChar(char* curLine, size_t* ptrCurLineInd) {
    ASSERT(curLine       != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrCurLineInd != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    fprintf(stderr, "(CropLastChar): BEFORE curLine: <%s>, ind: %zu\n", curLine, *ptrCurLineInd);

    curLine[*ptrCurLineInd - 1] = '\0';
    *ptrCurLineInd = *ptrCurLineInd - 1;

    fprintf(stderr, "(CropLastChar): AFTER curLine: <%s>, ind: %zu\n", curLine, *ptrCurLineInd);
    
    return OK;
}

ERROR_STATUSES GetVizLen(   char* curLine, size_t* ptrRes) {
    ASSERT(curLine != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrRes  != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    size_t res = 0;

    char* ch = curLine;

    for (; *ch != '\0'; ch++) {
        res++;
        if ((*ch) & 0x80) {
            ch++;  // skip one byte, if cyrillic
        }
    }

    *ptrRes = res;

    return OK;
}
