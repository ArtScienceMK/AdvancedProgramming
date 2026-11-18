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
    
typedef size_t StackElement_t;

#include "MyError.h"
#include "MyAssert.h"
#include "MyStack.h"

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
const int MaxDefLineSize     = 600;
const int MaxTraitsCnt      = 50;

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
ERROR_STATUSES Guess(         Tree* tree);
ERROR_STATUSES GiveDefinition(Tree* tree);
ERROR_STATUSES Compare(       Tree* tree);

ERROR_STATUSES GetDefinition(Node* root,          char* inputLine, char* definitionLine, size_t* ptrDefinitionLineSize);
ERROR_STATUSES RecDefinition(Node* curNode, const char* toFind,    Stack_t* ptrStack, bool* found);

ERROR_STATUSES GetCompare(  char** common, char** diff1, char** diff2, size_t* ptrCommonInd, size_t* ptrDiff1Ind, size_t* ptrDiff2Ind, 
                            Tree* ptrTree, char* object1Line, char* object2Line);

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
        printf("[у]гадать объект, [п]осмотреть базу данных, [в]ыйти\n [д]ать определение, [с]равнить\n");

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
                printf("Дерево не было сохранено!\n");
            }
            else {
                printf("Некорректный ответ на вопрос! Самоуничтожение...\n");
                return UNKNOWN_AKINATOR_ANSWER_OPTION;
            }

            printf("Пока!\n");
            return OK;
        }
        else if (strcmp(inputLine, "д") == 0) {
            GiveDefinition(&tree);
        }
        else if (strcmp(inputLine, "с") == 0) {
            Compare(&tree);
        }
        else {
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

ERROR_STATUSES GiveDefinition(Tree* ptrTree) {
    ERROR_STATUSES TreeVerifyErrorStatus = TreeVerify(ptrTree);

    ASSERT(TreeVerifyErrorStatus == OK, GetErrorString(TreeVerifyErrorStatus),
           TreeVerifyErrorStatus);

    Node* curNode  = ptrTree->root;

    char* inputLine = (char*)calloc(MaxUnicodeLineSize, sizeof(*inputLine));

    ASSERT(inputLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
    
    size_t inputLineSize = 0;
    size_t inputLineInd  = 0;

    printf("Введи название объекта, чьё определение хочешь получить:");

    if ((inputLineInd = getline(&inputLine, &inputLineSize, stdin)) == -1) {
        printf("Введено некорректное название...\n");
        printf("Самоуничтожение...\n");
        return AKINATOR_OBJECT_INPUT_ERROR;
    }

    CropLastChar(inputLine, &inputLineInd);

    char* definitionLine = (char*) calloc(MaxObjectLineSize, sizeof(definitionLine[0]));

    ASSERT(definitionLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
    
    size_t definitionLineSize = 0;

    ERROR_STATUSES GetDefinitionErrorStatus = GetDefinition(ptrTree->root, inputLine, definitionLine, &definitionLineSize);

    ASSERT(GetDefinitionErrorStatus == OK, GetErrorString(GetDefinitionErrorStatus),
           GetDefinitionErrorStatus);

    if (strcmp(definitionLine, "") == 0) {
        printf("Такого объекта нет в базе\n");
    } 
    else {
        printf("%s - это %s\n", inputLine, definitionLine);
    }

    return OK;
}

ERROR_STATUSES GetDefinition(Node* root, char* inputLine, char* definitionLine, size_t* ptrDefinitionLineSize) {
    ASSERT(root           != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(inputLine      != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(definitionLine != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    Stack_t stack   = {};
    size_t stackInitCap = 30;

    SourceLocation sourceLocation1 = {};
    GET_SOURCE_LOCATION(sourceLocation1, stack);

    ERROR_STATUSES StackInitErrorStatus = StackInit(&stack, stackInitCap, sourceLocation1);

    ASSERT(StackInitErrorStatus == OK, GetErrorString(StackInitErrorStatus), 
           StackInitErrorStatus);

    bool found = false;

    ERROR_STATUSES RecDefinitionErrorStatus = RecDefinition(root, inputLine, &stack, &found);

    ASSERT(RecDefinitionErrorStatus == OK, GetErrorString(RecDefinitionErrorStatus),
           RecDefinitionErrorStatus);

    size_t definitionLineInd = 0;

    while (stack.size > 1) {
        
        StackElement_t popValue = 0;

        ERROR_STATUSES StackPopErrorStatus = StackPop(&stack, &popValue);

        ASSERT(StackPopErrorStatus == OK, GetErrorString(StackPopErrorStatus),
               StackPopErrorStatus);

        Node* curNode = (Node*)popValue;    

        fprintf(stderr, "(GetDefinition): popped curNode: %p, has line: <%s>\n", 
                curNode, curNode->data.line);

        StackElement_t childPopValue = 0;

        StackPopErrorStatus = StackPop(&stack, &childPopValue);

        ASSERT(StackPopErrorStatus == OK, GetErrorString(StackPopErrorStatus),
               StackPopErrorStatus);

        Node* childNode = (Node*)childPopValue;    

        fprintf(stderr, "(GetDefinition): popped childNode: %p, has line: <%s>\n", 
                childNode, childNode->data.line);
            
        ERROR_STATUSES StackPushErrorStatus = StackPush(&stack, (size_t)(childNode));

        ASSERT(StackPushErrorStatus == OK, GetErrorString(StackPushErrorStatus),
               StackPushErrorStatus);

        ERROR_STATUSES AddLineErrorStatus = OK;

        if (childNode == curNode->left) {
            fprintf(stderr, "(GetDefinition): this statement is not true!\n");

            AddLineErrorStatus = AddLine(definitionLine, &definitionLineInd, "не ");

            ASSERT(AddLineErrorStatus == OK, GetErrorString(AddLineErrorStatus),
               AddLineErrorStatus);
        }

        AddLineErrorStatus = AddLine(definitionLine, &definitionLineInd, curNode->data.line);

        ASSERT(AddLineErrorStatus == OK, GetErrorString(AddLineErrorStatus),
               AddLineErrorStatus);

        AddLineErrorStatus = AddLine(definitionLine, &definitionLineInd, " ");

        ASSERT(AddLineErrorStatus == OK, GetErrorString(AddLineErrorStatus),
               AddLineErrorStatus);

        fprintf(stderr, "(GetDefinition): cur definition line: <%s>\n", 
                definitionLine);

    }

    *ptrDefinitionLineSize = definitionLineInd;

    return OK;
}

ERROR_STATUSES RecDefinition(Node* curNode, const char* toFind, Stack_t* ptrStack, bool* found) {

    ASSERT(toFind   != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrStack != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    if (curNode == NULL) {
        return OK;
    }

    if (strcmp(curNode->data.line, toFind) == 0) {
        *found = true;

        StackPush(ptrStack, (StackElement_t)curNode);
        return OK;
    }

    RecDefinition(curNode->left, toFind, ptrStack, found);

    if (*found) {
        StackPush(ptrStack, (StackElement_t)curNode);
        return OK;
    }

    RecDefinition(curNode->right, toFind, ptrStack, found);

    if (*found) {
        StackPush(ptrStack, (StackElement_t)curNode);
    }

    return OK;
}

const char* GetErrorString(ERROR_STATUSES errorStatus) {
    switch (errorStatus) {
        case OK:
            return "Program works fine! But this sentence shouldn't be printed to user!\n";
        
        case FILE_ACCESS_DENIED:
            return "Cannot access file!\n";
                
        case FILE_FSTAT_ERROR:
            return "Unknown fstat() error!\n";
                    
        case OUT_OF_RANGE:
            return "Index is out of range!\n";

        case CMP_ERROR:
            return "Comarator error!\n";
        
        case NOT_ENOUGH_MEMORY:
            return "Not enough memory!\n";

        case PUSH_NOT_EQUAL_POP:
            return "Last pushed element to stack is not equal first popped!\n";

        case BAD_CLEAR_INIT:
            return "Value was initialized with zeroes, but it's not true BEFORE initialization function!\n";

        case STACK_DESTROY_ERROR:
            return "Error happened when destroying stack!\n";
        
        case EMPTY_STACK:
            return "Trying to pop from the empty stack!\n";

        case STACK_CAP_SIZE:
            return "Stack capacity is less than size!\n";

        case BAD_CANARY:
            return "Canary value was changed!\n";

        case HASHES_DIFFER:
            return "Hashes values differ: hash inside structure not equal to just counted hash\n";

        case EXECUTION_ERROR:
            return "Error happened when executing!\n";

        case READ_FILE_ERROR:
            return "Error happened when reading file!\n";

        case UNLINK_FILE_ERROR:
            return "Error happened when unlinking file!\n";

        case UNKNOWN_OPCODE:
            return "Unknown opcode found when executing!\n";

        case ZERO_DIVISION_ERROR:
            return "Zero division error happened!\n"; 
        
        case OPCODE_INPUT_ERROR:
            return "Error happened while executing in opcode input!\n";
        
        case ARGS_INPUT_ERROR:
            return "Error happened while executing in args input!\n";   
        
        case ASSEMBLE_ERROR:
            return "Error happened while assembling!\n";

        case STRCMP_ERROR:
            return "Error happened while comparing strings!\n";
        
        case ASSEMBLE_VERIFICATION_ERROR:
            return "Incorrect line found while assembling!\n";
        
        case INT_TO_STRING_ERROR:
            return "Error happened while converting int to string!\n";
        
        case DISASSEMBLE_ERROR:
            return "Error happened while disassembling!\n";

        case READING_OPCODE_DISASSEMBLE_ERROR:
            return "Error happened when reading opcode while disassembling!\n";
        
        case WRITE_INDEX_TO_FILE_ERROR:
            return "Error happened when writing index to file!\n";

        case READING_COMMAND_ASSEMBLE_ERROR:
            return "Error happened when reading command while assembling!\n";

        case UNKNOWN_REGISTER_NAME:
            return "Got an unknown register name!\n";

        case GET_ARGUMENT_TYPE_ERROR:
            return "Error happened when getting argument type!\n";

        case IS_VALID_REGISTER_NAME_ERROR:
            return "Error happened when validating register name!\n";

        case NO_ARGUMENT_TYPE_ERROR:
            return "Argument type missing!\n";

        case UNKNOWN_ARGUMENT_TYPE:
            return "Unknown argument type!\n";

        case JUMP_ADDRESS_IS_OUT_OF_RANGE:
            return "Jump address is out of allocated memory range!\n";
        
        case NOT_INITIALIZED_JUMP_ADDRESS:
            return "Jump address was not initialized!\n";

        case HALT_EXIT:
            return "HALT command was called!\n";

        case IS_VALID_REGISTER_NAME_STRING_ERROR:
            return "Error happened when validating register name string!\n";
        
        case READ_LABEL_ERROR:
            return "Error happened when reading label!\n";

        case LABEL_REDEFINITION:
            return "Got a label redefinition!\n";

        case UNKNOWN_LABEL:
            return "Got an unknown label!\n";
        
        case NOT_INITIALIZED_LABEL:
            return "Label was not initialized!\n";

        case READING_MEMORY_INDEX_ERROR:
            return "When reading memory index error happened!\n";

        case BAD_MEMORY_ADDRESS:
            return "Memory address is out of allocated (for this vm) RAM!\n";
        
        case BAD_LIST_SIZE:
            return "List has size <= 0\n";

        case LIST_CAP_SIZE:
            return "List capacity less than size!\n";

        case BAD_LIST_HEAD:
            return "Incorrect head index in the list!\n";

        case BAD_LIST_TAIL:
            return "Incorrect tail index in the list!\n";
        
        case BAD_LIST_ZERO_ELEM:
            return "Incorrect zero element in list! Should have all zeros!\n";

        case LIST_CYCLE:
            return "List has cycles!\n";

        case SYSTEM_CALL_ERROR:
            return "Error while executing system()!\n";

        case NO_EDGE_ERROR:
            return "List node has less than two edges!\n";

        case UNKNOWN_NODE_STATUS:
            return "List node has unknown status!\n";

        case NOT_USED_NODE:
            return "List node is not used!\n";

        case NODE_INSERT_INT_ERROR:
            return "Error happened when inserting integer value in the node!\n";

        case TREE_BAD_SIZE:
            return "Tree has not positive size!\n";
        
        case UNKNOWN_AKINATOR_MENU_OPTION:
            return "Unknown menu option entered in akinator program!\n";

        case AKINATOR_MENU_INPUT_ERROR:
            return "Error happened when proceeding user input in akinator menu!\n";

        case AKINATOR_QUESTION_INPUT_ERROR:
            return "Error happened when proceeding user input while answering question!\n";

        case UNKNOWN_AKINATOR_ANSWER_OPTION:
            return "Incorrect akinator answer option!\n";

        case AKINATOR_OBJECT_INPUT_ERROR:
            return "Error when proceeding object input!\n";

        case UNKNOWN_STRING_LOAD_TREE:
            return "Got an unknown string when loading tree!\n";

        case AKINATOR_COMPARE_ERROR:
            return "An error happened when comparing in akinator!\n";

        default:
            return "Unknown error status!\n";
    }
}

ERROR_STATUSES Compare(Tree* ptrTree) {
    ERROR_STATUSES TreeVerifyErrorStatus = TreeVerify(ptrTree);

    ASSERT(TreeVerifyErrorStatus == OK, GetErrorString(TreeVerifyErrorStatus),
           TreeVerifyErrorStatus);

    Node* curNode  = ptrTree->root;

    char* object1Line = (char*)calloc(MaxUnicodeLineSize, sizeof(*object1Line));

    ASSERT(object1Line != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
    
    size_t object1LineSize = MaxObjectLineSize;
    size_t object1LineInd  = 0;

    printf("Введи название первого объекта для сравнения:");

    if ((object1LineInd = getline(&object1Line, &object1LineSize, stdin)) == -1) {
        printf("Введено некорректное название...\n");
        printf("Самоуничтожение...\n");
        return AKINATOR_OBJECT_INPUT_ERROR;
    }

    CropLastChar(object1Line, &object1LineInd);

    char* object2Line = (char*)calloc(MaxUnicodeLineSize, sizeof(*object2Line));

    ASSERT(object2Line != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    size_t object2LineSize = MaxObjectLineSize;
    size_t object2LineInd  = 0;

    printf("Введи название второго объекта для сравнения:");

    if ((object2LineInd = getline(&object2Line, &object2LineSize, stdin)) == -1) {
        printf("Введено некорректное название...\n");
        printf("Самоуничтожение...\n");
        return AKINATOR_OBJECT_INPUT_ERROR;
    }

    CropLastChar(object2Line, &object2LineInd);

    char** common = NULL, **diff1 = NULL, **diff2 = NULL;

    common = (char**)calloc(MaxTraitsCnt, sizeof(*common));

    ASSERT(common != NULL, GetErrorString(NOT_ENOUGH_MEMORY), 
           NOT_ENOUGH_MEMORY);

    size_t commonInd = 0;

    diff1 = (char**)calloc(MaxTraitsCnt, sizeof(*diff1));

    ASSERT(diff1 != NULL, GetErrorString(NOT_ENOUGH_MEMORY), 
           NOT_ENOUGH_MEMORY);

    size_t diff1Ind = 0;

    diff2 = (char**)calloc(MaxTraitsCnt, sizeof(*diff2));

    ASSERT(diff2 != NULL, GetErrorString(NOT_ENOUGH_MEMORY), 
           NOT_ENOUGH_MEMORY);

    size_t diff2Ind = 0;
    

    ERROR_STATUSES GetCompareErrorStatus = GetCompare(common, diff1, diff2, &commonInd, &diff1Ind, &diff2Ind, ptrTree, object1Line, object2Line);

    ASSERT(GetCompareErrorStatus == OK, GetErrorString(GetCompareErrorStatus), GetCompareErrorStatus);

    // fprintf(stderr, "common: %p, ind: %zu\n", common, commonInd);

    // for (size_t i = 0; i < commonInd; i++) {
    //     fprintf(stderr, "common[%zu]: <%s>\n", i, common[i]);
    // }

    printf("%s и %s схожи в том, что ", object1Line, object2Line);

    for (size_t i = 0; i < commonInd; i++) {
        printf("%s ", common[i]);
    }

    printf("\n");

    printf("%s отличается от %s тем, что ", object1Line, object2Line);

    for (size_t i = 0; i < diff1Ind; i++) {
        printf("%s ", diff1[i]);
    }

    printf("\n");

    printf("А %s отличается от %s тем, что ", object2Line, object1Line);

    for (size_t i = 0; i < diff2Ind; i++) {
        printf("%s ", diff2[i]);
    }

    printf("\n");
    
    // for (; def1LineInd < def1LineSize; def1LineInd++) {
    //     ClearLine(cur1Line);
        
    //     cur1LineInd = 0;

    //     if (!(cur1LineInd += sscanf(def1Line + def1LineInd, "%s", cur1Line + cur1LineInd))) {
    //         fprintf(stderr, "(Compare): cannot read from first line. def1Line: <%s>, def1LineInd: %zu\n",
    //                          def1Line + def1LineInd, def1LineInd);
    //         return AKINATOR_COMPARE_ERROR;
    //     }

    //     if (strcmp(cur1Line, "не") == 0) {
    //         if (!(cur1LineInd += sscanf(def1Line + def1LineInd, "%s", cur1Line + cur1LineInd))) {
    //             fprintf(stderr, "(Compare): cannot read from first line. def1Line: <%s>, def1LineInd: %zu\n",
    //                              def1Line + def1LineInd, def1LineInd);
    //             return AKINATOR_COMPARE_ERROR;
    //         }
    //     }

    //     *(diff1 + diff1Ind) = strdup(cur1Line);
    //     diff1Ind++;
    // }

    // for (; def2LineInd < def2LineSize; def2LineInd++) {

    // }

    return OK;
}

ERROR_STATUSES GetCompare(char** common, char** diff1, char** diff2, size_t* ptrCommonInd, size_t* ptrDiff1Ind, size_t* ptrDiff2Ind,
                          Tree* ptrTree, char* object1Line, char* object2Line) {

    size_t commonInd = *ptrCommonInd;
    size_t diff1Ind = *ptrDiff1Ind;
    size_t diff2Ind = *ptrDiff2Ind;

    Stack_t stack1 = {}, stack2 = {};
    
    SourceLocation sourceLocation1 = {};
    GET_SOURCE_LOCATION(sourceLocation1, stack1);

    ERROR_STATUSES StackInitErrorStatus = StackInit(&stack1, (size_t)30, sourceLocation1);

    ASSERT(StackInitErrorStatus == OK, GetErrorString(StackInitErrorStatus),
           StackInitErrorStatus);

    SourceLocation sourceLocation2 = {};
    GET_SOURCE_LOCATION(sourceLocation2, stack2);


    StackInitErrorStatus = StackInit(&stack2, (size_t)30, sourceLocation2);

    ASSERT(StackInitErrorStatus == OK, GetErrorString(StackInitErrorStatus),
           StackInitErrorStatus);

    bool found = false;

    ERROR_STATUSES RecDefinitionErrorStatus = RecDefinition(ptrTree->root, object1Line, &stack1, &found);

    ASSERT(RecDefinitionErrorStatus == OK, GetErrorString(RecDefinitionErrorStatus),
           RecDefinitionErrorStatus);

    found = false;

    RecDefinitionErrorStatus = RecDefinition(ptrTree->root, object2Line, &stack2, &found);

    ASSERT(RecDefinitionErrorStatus == OK, GetErrorString(RecDefinitionErrorStatus),
           RecDefinitionErrorStatus);

    while (stack1.size > 1 && stack2.size > 1) {
        StackElement_t popValue1 = 0;

        ERROR_STATUSES StackPopErrorStatus = StackPop(&stack1, &popValue1);

        ASSERT(StackPopErrorStatus == OK, GetErrorString(StackPopErrorStatus),
               StackPopErrorStatus);

        Node* curNode1 = (Node*)popValue1;    

        fprintf(stderr, "(GetCompare): popped curNode: %p, has line: <%s>\n", 
                curNode1, curNode1->data.line);

        StackElement_t childPopValue1 = 0;

        StackPopErrorStatus = StackPop(&stack1, &childPopValue1);

        ASSERT(StackPopErrorStatus == OK, GetErrorString(StackPopErrorStatus),
               StackPopErrorStatus);

        Node* childNode1 = (Node*)childPopValue1;    

        fprintf(stderr, "(GetCompare): popped childNode: %p, has line: <%s>\n", 
                childNode1, childNode1->data.line);
            
        ERROR_STATUSES StackPushErrorStatus = StackPush(&stack1, (size_t)(childNode1));

        ASSERT(StackPushErrorStatus == OK, GetErrorString(StackPushErrorStatus),
               StackPushErrorStatus);
        
        StackElement_t popValue2 = 0;
        
        StackPopErrorStatus = StackPop(&stack2, &popValue2);

        ASSERT(StackPopErrorStatus == OK, GetErrorString(StackPopErrorStatus),
               StackPopErrorStatus);

        Node* curNode2 = (Node*)popValue2;    

        fprintf(stderr, "(GetDefinition): popped curNode: %p, has line: <%s>\n", 
                curNode2, curNode2->data.line);

        StackElement_t childPopValue2 = 0;

        StackPopErrorStatus = StackPop(&stack2, &childPopValue2);

        ASSERT(StackPopErrorStatus == OK, GetErrorString(StackPopErrorStatus),
               StackPopErrorStatus);

        Node* childNode2 = (Node*)childPopValue2;    

        fprintf(stderr, "(GetCompare): popped childNode: %p, has line: <%s>\n", 
                childNode2, childNode2->data.line);
            
        StackPushErrorStatus = StackPush(&stack2, (size_t)(childNode2));

        ASSERT(StackPushErrorStatus == OK, GetErrorString(StackPushErrorStatus),
               StackPushErrorStatus);

        ERROR_STATUSES AddLineErrorStatus = OK;


        if (curNode1 == curNode2 && childNode1 == childNode2) {
            common[commonInd] = (char*)calloc(MaxObjectLineSize, sizeof(*common[commonInd]));

            ASSERT(common[commonInd] != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

            size_t curCommonLineInd = 0;

            if (childNode1 == curNode1->left) {
                fprintf(stderr, "(GetCompare): this statement is not true!\n");
                
                AddLineErrorStatus = AddLine(common[commonInd], &curCommonLineInd, "не ");

                ASSERT(AddLineErrorStatus == OK, GetErrorString(AddLineErrorStatus),
                       AddLineErrorStatus);
            }

            AddLineErrorStatus = AddLine(common[commonInd], &curCommonLineInd, curNode1->data.line);

            ASSERT(AddLineErrorStatus == OK, GetErrorString(AddLineErrorStatus),
                   AddLineErrorStatus);

            commonInd++;

        } else {
            diff1[diff1Ind] = (char*)calloc(MaxObjectLineSize, sizeof(*diff1[diff1Ind]));

            ASSERT(diff1[diff1Ind] != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

            size_t curDiff1LineInd = 0;


            diff2[diff2Ind] = (char*)calloc(MaxObjectLineSize, sizeof(*diff2[diff2Ind]));

            ASSERT(diff2[diff2Ind] != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

            size_t curDiff2LineInd = 0;


            if (childNode1 == curNode1->left) {
                fprintf(stderr, "(GetCompare): this statement is not true!\n");
                 AddLineErrorStatus = AddLine(diff1[diff1Ind], &curDiff1LineInd, "не ");

                ASSERT(AddLineErrorStatus == OK, GetErrorString(AddLineErrorStatus),
                AddLineErrorStatus);
            }

            if (childNode2 == curNode2->left) {
                fprintf(stderr, "(GetCompare): this statement is not true!\n");
                 AddLineErrorStatus = AddLine(diff2[diff2Ind], &curDiff2LineInd, "не ");

                ASSERT(AddLineErrorStatus == OK, GetErrorString(AddLineErrorStatus),
                AddLineErrorStatus);
            }

            AddLineErrorStatus = AddLine(diff1[diff1Ind], &curDiff1LineInd, curNode1->data.line);

            ASSERT(AddLineErrorStatus == OK, GetErrorString(AddLineErrorStatus),
                   AddLineErrorStatus);

            AddLineErrorStatus = AddLine(diff2[diff2Ind], &curDiff2LineInd, curNode2->data.line);

            ASSERT(AddLineErrorStatus == OK, GetErrorString(AddLineErrorStatus),
                   AddLineErrorStatus);
            
            diff1Ind++;
            diff2Ind++;
        }
    }

    

    *ptrCommonInd = commonInd;
    *ptrDiff1Ind  = diff1Ind;
    *ptrDiff2Ind  = diff2Ind;

    return OK;
}
