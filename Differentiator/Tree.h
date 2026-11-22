#ifndef _TREE_H
#define _TREE_H

typedef double TreeElement_t;

enum NODE_TYPES {
    EMPTY_NODE = 0,
    OP         = 1,
    CONST      = 2,
    VAR        = 3,
};

enum OP_TYPES {
    EMPTY_OP = '0',
    ADD      = '+',
    SUB      = '-',
    MUL      = '*',
    DIV      = '/',
    POW      = '^',
    LOG      = 'L',
    SIN      = 'S',
    COS      = 'C',
    TG       = 'T',
    CTG      = 'G',
    ARCSIN   = 's',
    ARCCOS   = 'c',
    ARCTG    = 't',
    ARCCTG   = 'g',
    SH       = 'H',
    CH       = 'J',
    TH       = 'K',
};

struct Node {
    NODE_TYPES nodeType;
    OP_TYPES   opType;
    double value;
    char* varName;

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

ERROR_STATUSES TreeInsert    (Tree* ptrTree, TreeElement_t value);
ERROR_STATUSES NodeInsert    (Node* ptrNode, TreeElement_t value);

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

ERROR_STATUSES StringToDouble(char* curLine, TreeElement_t* ptrRes);

ERROR_STATUSES UnlinkFile(const char* filename);

ERROR_STATUSES WriteBufferToFile(const char* filename, char* buffer, size_t bufferSize);

ERROR_STATUSES GenerateTreeFile(Tree* ptrTree, char** ptrBuffer, size_t* ptrBufferSize);
ERROR_STATUSES SaveTree(        Tree* ptrTree);
ERROR_STATUSES LoadTree(        Tree* ptrTree);

ERROR_STATUSES ReadFile(const char* filename, char** ptrBuffer, size_t* ptrBufferSize);
ERROR_STATUSES GetFileSize(int fileDesc, size_t* ptrFileSize);

ERROR_STATUSES CropLastChar(char* curLine, size_t* ptrCurLineInd);
ERROR_STATUSES GetVizLen(   char* curLine, size_t* res);

ERROR_STATUSES PrintOpType(OP_TYPES opType,     char** ptrBuffer, size_t* ptrBufferSize);
ERROR_STATUSES PrintDouble(double value, char** ptrBuffer, size_t* ptrBufferSize);

#endif /*  TREE_H  */