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

ERROR_STATUSES NodeCreate(       Node* ptrParent, bool isLeftSon, int value);
ERROR_STATUSES NodeCreateString( Node* ptrParent, bool isLeftSon, const char* line);


ERROR_STATUSES TreeVerify(   Tree* ptrTree);
ERROR_STATUSES RecNodeVerify(Node* ptrNode);
ERROR_STATUSES NodeVerify(   Node* ptrNode);

ERROR_STATUSES TreeDump(     Tree* ptrTree);
ERROR_STATUSES RecNodeDump(  Node* ptrNode);

ERROR_STATUSES TreePrint(           Tree* ptrTree);
ERROR_STATUSES RecNodePrint(        Node* ptrNode);
ERROR_STATUSES RecNodePrintToBuffer(Node* ptrNode, char* buffer, size_t* ptrBufferInd, char* curLine);

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

ERROR_STATUSES RusToTrans(const char* from, char** to);

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

    Node* prevNode = NULL;

    bool isLeftSon = false;

    while (ptrNode != NULL) {
        
        prevNode = ptrNode;
        
        if (value < ptrNode->data.value) {
            ptrNode = ptrNode->left;
            isLeftSon = true;
        } else {
            ptrNode = ptrNode->right;
            isLeftSon = false;
        }
    }

    fprintf(stderr, "(NodeInsertInt): ptrNode: %p, prevNode: %p\n", ptrNode, prevNode);

    ERROR_STATUSES NodeCreateErrorStatus = NodeCreate(prevNode, isLeftSon, value);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    return OK;
}

ERROR_STATUSES NodeCreate(Node* ptrParent, bool isLeftSon, int value) {

    ERROR_STATUSES NodeVerifyErrorStatus = NodeVerify(ptrParent);

    ASSERT(NodeVerifyErrorStatus == OK, GetErrorString(NodeVerifyErrorStatus),
           NodeVerifyErrorStatus);

    Node* ptrNode = (Node*)calloc(1, sizeof(*ptrNode));

    ASSERT(ptrNode != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    ptrNode->parent     = ptrParent;
    ptrNode->data.value = value;

    if (isLeftSon) {
        ptrParent->left = ptrNode;
    } else {
        ptrParent->right = ptrNode;
    }

    fprintf(stderr, "ptrNode: %p\n", ptrNode);

    return OK;
}

ERROR_STATUSES NodeCreateString(Node* ptrParent, bool isLeftSon, const char* line) {

    ERROR_STATUSES NodeVerifyErrorStatus = NodeVerify(ptrParent);

    ASSERT(NodeVerifyErrorStatus == OK, GetErrorString(NodeVerifyErrorStatus),
           NodeVerifyErrorStatus);

    Node* ptrNode = (Node*)calloc(1, sizeof(*ptrNode));

    ASSERT(ptrNode != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    ptrNode->parent     = ptrParent;
    ptrNode->data.line  = strdup(line);

    if (isLeftSon) {
        ptrParent->left = ptrNode;
    } else {
        ptrParent->right = ptrNode;
    }

    fprintf(stderr, "ptrNode: %p\n", ptrNode);

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

ERROR_STATUSES RecNodePrintToBuffer(Node* ptrNode, char* buffer, size_t* ptrBufferInd, char* curLine) {
    
    if (ptrNode->parent->left == ptrNode) {
        sprintf(curLine, "(L%s ", ptrNode->data.line);
    }
    else {
        sprintf(curLine, "(R%s ", ptrNode->data.line);
    }
    
    ERROR_STATUSES AddLineCleanErrorStatus = AddLineClean(buffer, ptrBufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

    if (ptrNode->left != NULL) {
        RecNodePrintToBuffer(ptrNode->left, buffer, ptrBufferInd, curLine);
    }
    if (ptrNode->right != NULL) {
        RecNodePrintToBuffer(ptrNode->right, buffer, ptrBufferInd, curLine);
    }

    sprintf(curLine, ")");

    AddLineCleanErrorStatus = AddLineClean(buffer, ptrBufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);

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

        sprintf(curLine, "\t %zu->%zu[color=\"#3ba0ff\"];\n", ptrNode, ptrNode->left);

        AddLineCleanErrorStatus = AddLineClean(buffer, ptrBufferInd, curLine);

        ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus),
           AddLineCleanErrorStatus);
    }
    if (ptrNode->right != NULL) {
        RecNodeVizDump(ptrNode->right, buffer, ptrBufferInd, curLine);

        sprintf(curLine, "\t %zu->%zu[color=\"#ff3b3b\"];\n", ptrNode, ptrNode->right);

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

    TreeSetInitialString(&tree, "neizvestnyi");

    ERROR_STATUSES LoadTreeErrorStatus = LoadTree(&tree);

    ASSERT(LoadTreeErrorStatus == OK, GetErrorString(LoadTreeErrorStatus), LoadTreeErrorStatus);

    char* inputLine = (char*)calloc(MaxUnicodeLineSize, sizeof(*inputLine));

    ASSERT(inputLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    for (int i = 0; i < nQueries; i++) {
        printf("Что ты желаешь сделать?\n");
        printf("[У]гадать объект, [П]осмотреть базу данных, [В]ыйти\n");

        ClearLine(inputLine);

        if (!scanf("%s", inputLine)) {
            printf("Введена неверная опция... Самоуничтожение...\n");
            return AKINATOR_MENU_INPUT_ERROR;
        } 

        if (strcmp(inputLine, "У") == 0) {
            Guess(&tree);            
        }
        else if (strcmp(inputLine, "П") == 0) {
            TreeVizDump(&tree);            
        }
        else if (strcmp(inputLine, "В") == 0) {
            printf("Сохранить базу данных?\n");

            ClearLine(inputLine);

            if (!scanf("%s", inputLine)) {
                printf("Введена неверная опция... Самоуничтожение...\n");
                return AKINATOR_MENU_INPUT_ERROR;
            }

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
    
    bool isLeftSon = false;

    while (!(curNode->left == NULL && curNode->right == NULL)) {

        printf("%s?\n", curNode->data.line);

        ClearLine(inputLine);

        if (!scanf("%s", inputLine)) {
            printf("Не смог считать ответ на вопрос! Самоуничтожение...\n");
            return AKINATOR_QUESTION_INPUT_ERROR;
        }

        if (strcmp(inputLine, "да") == 0) {
            isLeftSon = false;
            curNode = curNode->right;
        } 
        else if (strcmp(inputLine, "нет") == 0) {
            isLeftSon = true;
            curNode = curNode->left;
        }
        else {
            printf("Некорректный ответ на вопрос! Самоуничтожение...\n");
            return UNKNOWN_AKINATOR_ANSWER_OPTION;
        }
    }

    printf("Это %s?\n", curNode->data.line);

    ClearLine(inputLine);

    if (!scanf("%s", inputLine)) {
        printf("Не смог считать ответ на вопрос! Самоуничтожение...\n");
        return AKINATOR_QUESTION_INPUT_ERROR;
    }

    if (strcmp(inputLine, "да") == 0) {
        printf("Я был прав! Теперь я повелитель Триштатья!\n");
        return OK;
    } 
    else if (strcmp(inputLine, "нет") == 0) {
        printf("Не может быть! Помоги мне стать умнее: скажи, что это было?\n");
        
        ClearLine(inputLine);

        if (!scanf("%s", inputLine)) {
            printf("Некорректный ввод... Самоуничтожение...\n");
            return UNKNOWN_AKINATOR_ANSWER_OPTION;
        }

        printf("А чем %s отличается от %s? %s ...", inputLine, curNode->data.line, inputLine);

        char* traitLine = (char*)calloc(MaxUnicodeLineSize, sizeof(*traitLine));

        ASSERT(traitLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

        char* otherLine = strdup(curNode->data.line);

        if (!scanf("%s", traitLine)) {
            printf("Некорректный ввод... Самоуничтожение...\n");
            return UNKNOWN_AKINATOR_ANSWER_OPTION;
        }


        ERROR_STATUSES NodeCreateErrorStatus = NodeCreateString(curNode, 0, (const char*)inputLine);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), NodeCreateErrorStatus);

        NodeCreateErrorStatus = NodeCreateString(curNode, 1, (const char*)otherLine);

            
        // if (isLeftSon) {
        //     NodeCreateErrorStatus = NodeCreateString(prevNode->left, 0, (const char*)inputLine);
        // } 
        // else {
        //     NodeCreateErrorStatus = NodeCreateString(prevNode->right, 0, (const char*)inputLine);
        // }

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), NodeCreateErrorStatus);

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

    size_t bufferInd = 0;

    Tree tree = {};
    TreeInit(&tree);

    Node* curNode = ptrTree->root;

    int nNodes = 1;

    fprintf(stderr, "bufferSize: %i\n", bufferSize);

    for (; bufferInd < bufferSize && *(buffer + bufferInd) != '\0'; ) {
        ClearLine(curLine);

        ASSERT(curLine != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

        bool isLeftSon = false;
        
        if (*(buffer + bufferInd) == ')') {
            fprintf(stderr, "(LoadTree): Added )\n");

            curNode = curNode->parent;
            
            bufferInd++;
            continue;
        }
        if (sscanf(buffer + bufferInd, "(L%s", curLine)) {
            isLeftSon = true;
        }
        else if (sscanf(buffer + bufferInd, "(R%s", curLine)) {
            isLeftSon = false;
        } 
        else {
            return UNKNOWN_STRING_LOAD_TREE;
        }

        fprintf(stderr, "(LoadTree): bufferInd before: %i, curLine: <%s>\n", bufferInd, curLine);
        
        ERROR_STATUSES NodeCreateErrorStatus = NodeCreateString(curNode, isLeftSon, curLine);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), NodeCreateErrorStatus);

        if (isLeftSon) {
            curNode = curNode->left;
        } 
        else {
            curNode = curNode->right;
        }

        nNodes++;

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

    ERROR_STATUSES RecNodePrintToBufferErrorStatus = OK;
    
    if (ptrTree->root->left != NULL) {
            RecNodePrintToBufferErrorStatus = RecNodePrintToBuffer(ptrTree->root->left, buffer, &bufferInd, curLine);    

        ASSERT(RecNodePrintToBufferErrorStatus == OK, GetErrorString(RecNodePrintToBufferErrorStatus), 
           RecNodePrintToBufferErrorStatus);
    }
    if (ptrTree->root->right != NULL) {
            RecNodePrintToBufferErrorStatus = RecNodePrintToBuffer(ptrTree->root->right, buffer, &bufferInd, curLine);    

        ASSERT(RecNodePrintToBufferErrorStatus == OK, GetErrorString(RecNodePrintToBufferErrorStatus), 
           RecNodePrintToBufferErrorStatus);
    }

    ERROR_STATUSES AddLineCleanErrorStatus = AddLineClean(buffer, &bufferInd, curLine);

    ASSERT(AddLineCleanErrorStatus == OK, GetErrorString(AddLineCleanErrorStatus), AddLineCleanErrorStatus);

    *ptrBuffer     = buffer;
    *ptrBufferSize = bufferInd;

    return OK;
}
