#ifdef DEBUG

#define ON_DEBUG(...)  __VA_ARGS__

#else

#define ON_DEBUG(...)

#endif

#define MAINFILE

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

#include "Tree.h"

const double e = exp(1);

ERROR_STATUSES Diff(     Node* node, Node** ptrRes);

ERROR_STATUSES DiffConst(Node* node, Node** ptrRes);
ERROR_STATUSES DiffVar  (Node* node, Node** ptrRes);

ERROR_STATUSES DiffAdd(  Node* node, Node** ptrRes);
ERROR_STATUSES DiffSub(  Node* node, Node** ptrRes);
ERROR_STATUSES DiffMul(  Node* node, Node** ptrRes);
ERROR_STATUSES DiffDiv(  Node* node, Node** ptrRes);

ERROR_STATUSES DiffPow(  Node* node, Node** ptrRes);
ERROR_STATUSES DiffLog(  Node* node, Node** ptrRes);

ERROR_STATUSES DiffSin(  Node* node, Node** ptrRes);
ERROR_STATUSES DiffCos(  Node* node, Node** ptrRes);
ERROR_STATUSES DiffTg (  Node* node, Node** ptrRes);


int main() {
    Tree tree = {};

    LoadTree(&tree);

    Tree diffTree = {};
    TreeInit(&diffTree);

    Diff(tree.root, &(diffTree.root));

    fprintf(stderr, "diffTree.root: %p\n", diffTree.root);

    TreeVizDump(&diffTree); 

    SaveTree(&diffTree);

    TreeDestroy(&tree);

    
    return 0;
}

ERROR_STATUSES Diff(Node* node, Node** ptrRes) {
    ASSERT(node   != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrRes != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    fprintf(stderr, "(Diff): node: %p, ptrRes: %p\n", node, ptrRes);

    ERROR_STATUSES NodeCreateErrorStatus = OK, DiffErrorStatus = OK;
    ERROR_STATUSES NodeCopyErrorStatus = OK;

    Node* curNode = NULL;

    switch (node->nodeType) {
        case CONST: {
            ERROR_STATUSES DiffConstErrorStatus = DiffConst(node, ptrRes);

            ASSERT(DiffConstErrorStatus == OK, GetErrorString(DiffConstErrorStatus), DiffConstErrorStatus);

            break;
        }  
        case VAR: {
            ERROR_STATUSES DiffVarErrorStatus = DiffVar(node, ptrRes);

            ASSERT(DiffVarErrorStatus == OK, GetErrorString(DiffVarErrorStatus), DiffVarErrorStatus);

            break;
        }
        case OP: {
            break;
        }

        default: {
            return UNKNOWN_OP_TYPE;
        }
    }

    fprintf(stderr, "(Diff): operator found!\n");

    switch (node->opType) {
        case ADD: {
            ERROR_STATUSES DiffAddErrorStatus = DiffAdd(node, ptrRes);

            ASSERT(DiffAddErrorStatus == OK, GetErrorString(DiffAddErrorStatus), 
                   DiffErrorStatus);

            break;
        }
        case SUB: {
            ERROR_STATUSES DiffSubErrorStatus = DiffSub(node, ptrRes);

            ASSERT(DiffSubErrorStatus == OK, GetErrorString(DiffSubErrorStatus), 
                   DiffSubErrorStatus);

            break;
        }
        case MUL: {
            ERROR_STATUSES DiffMulErrorStatus = DiffMul(node, ptrRes);

            ASSERT(DiffMulErrorStatus == OK, GetErrorString(DiffMulErrorStatus), 
                   DiffMulErrorStatus);

            break;
        }
        case DIV: {
            ERROR_STATUSES DiffDivErrorStatus = DiffDiv(node, ptrRes);

            ASSERT(DiffDivErrorStatus == OK, GetErrorString(DiffDivErrorStatus), 
                   DiffDivErrorStatus);
            
            break;
        }
        case POW: {
            ERROR_STATUSES DiffPowErrorStatus = DiffPow(node, ptrRes);

            ASSERT(DiffPowErrorStatus == OK, GetErrorString(DiffPowErrorStatus),
                   DiffPowErrorStatus);

            break;
        }
        case LOG: {
            ERROR_STATUSES DiffLogErrorStatus = DiffLog(node, ptrRes);

            ASSERT(DiffLogErrorStatus == OK, GetErrorString(DiffLogErrorStatus), 
                   DiffLogErrorStatus);

            break;
        }
        case SIN: {
            ERROR_STATUSES DiffSinErrorStatus = DiffSin(node, ptrRes);

            ASSERT(DiffSinErrorStatus == OK, GetErrorString(DiffSinErrorStatus), 
                   DiffSinErrorStatus);

            break;
        }
        case COS: {
            ERROR_STATUSES DiffCosErrorStatus = DiffCos(node, ptrRes);

            ASSERT(DiffCosErrorStatus == OK, GetErrorString(DiffCosErrorStatus), 
                   DiffCosErrorStatus);
                
            break;
        }
        case TG: {
            ERROR_STATUSES DiffTgErrorStatus = DiffTg(node, ptrRes);

            ASSERT(DiffTgErrorStatus == OK, GetErrorString(DiffTgErrorStatus), 
                   DiffTgErrorStatus);

            break;
        }

    }

    return OK;
}

ERROR_STATUSES DiffConst(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): constant found!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK;
    
    Node* curNode = NULL;

    NodeCreateErrorStatus = NodeCreate(&curNode, CONST, EMPTY_OP, 0, NULL, NULL, NULL, NULL);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
            NodeCreateErrorStatus);

    *ptrRes = curNode;

    fprintf(stderr, "(Diff): res: %p\n", curNode);
    return OK;
}

ERROR_STATUSES DiffVar(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): variable found!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK;

    Node* curNode = NULL;

    NodeCreateErrorStatus = NodeCreate(&curNode, CONST, EMPTY_OP, 1, NULL, NULL, NULL, NULL);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
            NodeCreateErrorStatus);
    
    *ptrRes = curNode;

    fprintf(stderr, "(Diff): res: %p\n", curNode);

    return OK;
}

ERROR_STATUSES DiffAdd(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): + found!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    Node* leftSon = NULL, *rightSon = NULL;

    DiffErrorStatus = Diff(node->left, &leftSon);

    ASSERT(DiffErrorStatus == OK, GetErrorString(DiffErrorStatus), DiffErrorStatus);

    DiffErrorStatus = Diff(node->right, &rightSon);

    ASSERT(DiffErrorStatus == OK, GetErrorString(DiffErrorStatus), DiffErrorStatus);

    fprintf(stderr, "(Diff): leftSon: %p, rightSon: %p\n", leftSon, rightSon);

    NodeCreateErrorStatus = NodeCreate(&curNode, OP, ADD, NAN, NULL, leftSon, rightSon, NULL);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
            NodeCreateErrorStatus);
    
    *ptrRes = curNode;

    fprintf(stderr, "(Diff): res: %p\n", curNode);

    return OK;
}

ERROR_STATUSES DiffSub(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): - found!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    Node* leftSon = NULL, *rightSon = NULL;

    DiffErrorStatus = Diff(node->left, &leftSon);

    ASSERT(DiffErrorStatus == OK, GetErrorString(DiffErrorStatus), DiffErrorStatus);

    DiffErrorStatus = Diff(node->right, &rightSon);

    ASSERT(DiffErrorStatus == OK, GetErrorString(DiffErrorStatus), DiffErrorStatus);

    fprintf(stderr, "(Diff): leftSon: %p, rightSon: %p\n", leftSon, rightSon);

    NodeCreateErrorStatus = NodeCreate(&curNode, OP, SUB, NAN, NULL, leftSon, rightSon, NULL);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
            NodeCreateErrorStatus);
    
    *ptrRes = curNode;

    fprintf(stderr, "(Diff): res: %p\n", curNode);

    return OK;
}

ERROR_STATUSES DiffMul(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): + found!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    Node* CL = NULL, *CR = NULL, *DL = NULL, *DR = NULL;
    
    NodeCopyErrorStatus = NodeCopy(&CL, node->left);

    ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), NodeCopyErrorStatus);

    NodeCopyErrorStatus = NodeCopy(&CR, node->right);

    ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), NodeCopyErrorStatus);

    DiffErrorStatus = Diff(node->left, &DL);

    ASSERT(DiffErrorStatus == OK, GetErrorString(DiffErrorStatus), DiffErrorStatus);

    DiffErrorStatus = Diff(node->right, &DR);

    ASSERT(DiffErrorStatus == OK, GetErrorString(DiffErrorStatus), DiffErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode, OP, ADD, NAN, NULL, NULL, NULL, NULL);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->left, OP, MUL, NAN, NULL, DL, CR, curNode);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->right, OP, MUL, NAN, NULL, CL, DR, curNode);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    *ptrRes = curNode;

    fprintf(stderr, "(Diff): res: %p\n", curNode);

    return OK;
}

ERROR_STATUSES DiffDiv(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): / found!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;


    Node* CL = NULL, *CR = NULL, *CCR = NULL, *DL = NULL, *DR = NULL;
    
    NodeCopyErrorStatus = NodeCopy(&CL, node->left);

    ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), NodeCopyErrorStatus);

    NodeCopyErrorStatus = NodeCopy(&CR, node->right);

    ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), NodeCopyErrorStatus);

    NodeCopyErrorStatus = NodeCopy(&CCR, node->right);

    ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), NodeCopyErrorStatus);

    DiffErrorStatus = Diff(node->left, &DL);

    ASSERT(DiffErrorStatus == OK, GetErrorString(DiffErrorStatus), DiffErrorStatus);

    DiffErrorStatus = Diff(node->right, &DR);

    ASSERT(DiffErrorStatus == OK, GetErrorString(DiffErrorStatus), DiffErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode, OP, DIV, NAN, NULL, NULL, NULL, NULL);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->left, OP, SUB, NAN, NULL, NULL, NULL, curNode);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->left->left, OP, MUL, NAN, NULL, DL, CR, curNode->left);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);
    
    NodeCreateErrorStatus = NodeCreate(&curNode->left->right, OP, MUL, NAN, NULL, CL, DR, curNode->left);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->right, OP, POW, NAN, NULL, CCR, NULL, curNode);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->right->right, CONST, EMPTY_OP, 2, NULL, NULL, NULL, curNode->right);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    *ptrRes = curNode;

    fprintf(stderr, "(Diff): res: %p\n", curNode);

    return OK;
}

ERROR_STATUSES DiffPow(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): ^ found!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    if (node->left->nodeType == CONST) {
        if (node->right->nodeType == CONST) {
            ERROR_STATUSES DiffConstErrorStatus = DiffConst(node, ptrRes);

            ASSERT(DiffConstErrorStatus == OK, GetErrorString(DiffConstErrorStatus), 
                   DiffConstErrorStatus);

            return OK;
        } 
        else {
            NodeCreateErrorStatus = NodeCreate(&curNode, OP, MUL, NAN, NULL, NULL, NULL, NULL);
        
            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
                    NodeCreateErrorStatus);

            Node* CP = NULL;
            
            NodeCopyErrorStatus = NodeCopy(&CP, node);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                    NodeCopyErrorStatus);

            curNode->left = CP;
            CP->parent    = curNode;

            NodeCreateErrorStatus = NodeCreate(&curNode->right, OP, LOG, NAN, NULL, NULL, NULL, NULL);
        
            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
                    NodeCreateErrorStatus);
                
            NodeCreateErrorStatus = NodeCreate(&curNode->right->left, CONST, EMPTY_OP, node->left->value, 
                                                NULL, NULL, NULL, curNode->right);
        
            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
                    NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->right->right, CONST, EMPTY_OP, e, 
                                                NULL, NULL, NULL, curNode->right);
        
            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
                    NodeCreateErrorStatus);
        }
    } 
    else {
        if (node->right->nodeType == CONST) {
            NodeCreateErrorStatus = NodeCreate(&curNode, OP, MUL, NAN, NULL, NULL, NULL, NULL);
        
            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
                    NodeCreateErrorStatus);

            Node* CR = NULL;

            NodeCopyErrorStatus = NodeCopy(&CR, node->right);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                    NodeCopyErrorStatus);

            curNode->left = CR;
            CR->parent    = curNode;

            Node* CP = NULL;

            NodeCopyErrorStatus = NodeCopy(&CP, node);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                    NodeCopyErrorStatus);

            CP->right->value -= 1;

            curNode->right = CP;
            CP->parent     = curNode;
        } 
        else {
            NodeCreateErrorStatus = NodeCreate(&curNode, OP, MUL, NAN, NULL, NULL, NULL, NULL);
        
            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
                    NodeCreateErrorStatus);
            
            Node* CP = NULL;

            NodeCopyErrorStatus = NodeCopy(&CP, node);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                    NodeCopyErrorStatus);

            curNode->left = CP;
            CP->parent    = curNode;

            NodeCreateErrorStatus = NodeCreate(&curNode->right, OP, ADD, NAN, NULL, NULL, NULL, curNode);
        
            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
                    NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->right->left, OP, LOG, NAN, NULL, NULL, NULL, curNode->right);
        
            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
                    NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->right->left->left, VAR, EMPTY_OP, NAN, node->left->varName,
                                                NULL, NULL, curNode->right->left);
        
            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
                    NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->right->left->right, CONST, EMPTY_OP, e, NULL, NULL, NULL, 
                                                curNode->right->left);
        
            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
                    NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->right->right, CONST, EMPTY_OP, 1, NULL, NULL, NULL, 
                                                curNode->right);
        
            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
                    NodeCreateErrorStatus);
        }
    }

    *ptrRes = curNode;

    fprintf(stderr, "(Diff): res: %p\n", curNode);
    
    return OK;
}

ERROR_STATUSES DiffLog(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): log found!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    if (node->left->nodeType == CONST && node->right->nodeType == CONST) {
        ERROR_STATUSES DiffConstErrorStatuses = DiffConst(node, ptrRes);

        ASSERT(DiffConstErrorStatuses == OK, GetErrorString(DiffConstErrorStatuses), 
               DiffConstErrorStatuses);

        return OK;
    }
    if (node->left->nodeType == VAR && node->right->nodeType == VAR) {
        NodeCreateErrorStatus = NodeCreate(&curNode, CONST, EMPTY_OP, 1, NULL, NULL, NULL, NULL);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
            NodeCreateErrorStatus);

        *ptrRes = curNode;

        fprintf(stderr, "(Diff): res: %p\n", curNode);

        return OK;
    }

    NodeCreateErrorStatus = NodeCreate(&curNode, OP, DIV, NAN, NULL, NULL, NULL, NULL);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
           NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->left, CONST, EMPTY_OP, 1, NULL, NULL, NULL, curNode);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);
        
    NodeCreateErrorStatus = NodeCreate(&curNode->right, OP, MUL, NAN, NULL, NULL, NULL, curNode);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    if (node->left->nodeType == CONST) {
        NodeCreateErrorStatus = NodeCreate(&curNode->right->left, CONST, EMPTY_OP, node->left->value, NULL, NULL, NULL, curNode->right);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

        NodeCreateErrorStatus = NodeCreate(&curNode->right->right, OP, LOG, NAN, NULL, NULL, NULL, curNode->right);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

        NodeCreateErrorStatus = NodeCreate(&curNode->right->right->left, VAR, EMPTY_OP, NAN, node->right->varName,
                                           NULL, NULL, curNode->right->right);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

        NodeCreateErrorStatus = NodeCreate(&curNode->right->right->right, CONST, EMPTY_OP, e, NULL, NULL, NULL, curNode->right->right);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);
    }
    else {
        NodeCreateErrorStatus = NodeCreate(&curNode->right->left, VAR, EMPTY_OP, NAN, node->left->varName, NULL, NULL, curNode->right);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

        NodeCreateErrorStatus = NodeCreate(&curNode->right->right, OP, LOG, NAN, NULL, NULL, NULL, curNode->right);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

        NodeCreateErrorStatus = NodeCreate(&curNode->right->right->left, CONST, EMPTY_OP, node->right->value, NULL, NULL, NULL, curNode->right->right);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

        NodeCreateErrorStatus = NodeCreate(&curNode->right->right->right, CONST, EMPTY_OP, e, NULL, NULL, NULL, curNode->right->right);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);
    }

    *ptrRes = curNode;

    fprintf(stderr, "(Diff): res: %p\n", curNode);
    
    return OK;
}

ERROR_STATUSES DiffSin(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): found sin!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    switch (node->left->nodeType) {
        case CONST: {
            ERROR_STATUSES DiffConstErrorStatus = DiffConst(node, ptrRes);

            ASSERT(DiffConstErrorStatus == OK, GetErrorString(DiffConstErrorStatus), 
                   DiffConstErrorStatus);
            
            return OK;
        }
        case VAR: {
            NodeCopyErrorStatus = NodeCopy(&curNode, node);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                   NodeCopyErrorStatus);

            curNode->opType = COS;

            break;
        }
        case OP: {
            NodeCreateErrorStatus = NodeCreate(&curNode, OP, MUL, NAN, NULL, NULL, NULL, NULL);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCopyErrorStatus = NodeCopy(&curNode->left, node);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                   NodeCopyErrorStatus);

            curNode->left->opType = COS;
            curNode->left->parent = curNode;

            Node* CL = NULL;

            NodeCopyErrorStatus = NodeCopy(&CL, node->left);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                   NodeCopyErrorStatus);
        
            DiffErrorStatus = Diff(CL, &curNode->right);

            curNode->right->parent = curNode;

            break;
        }
        default: {
            return UNKNOWN_OP_TYPE;
        }
    }

    *ptrRes = curNode;
    
    fprintf(stderr, "(Diff): res: %p\n", curNode);
    
    return OK;
}

ERROR_STATUSES DiffCos(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): found cos!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    switch (node->left->nodeType) {
        case CONST: {
            ERROR_STATUSES DiffConstErrorStatus = DiffConst(node, ptrRes);

            ASSERT(DiffConstErrorStatus == OK, GetErrorString(DiffConstErrorStatus), 
                   DiffConstErrorStatus);
            
            return OK;
        }
        case VAR: {
            NodeCreateErrorStatus = NodeCreate(&curNode, OP, MUL, NAN, NULL, NULL, NULL, NULL);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->left, CONST, EMPTY_OP, -1, NULL, NULL, NULL, curNode);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);
                   
            NodeCopyErrorStatus = NodeCopy(&curNode->right, node);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                   NodeCopyErrorStatus);

            curNode->right->opType = SIN;
            curNode->right->parent = curNode;

            break;
        }
        case OP: {
            NodeCreateErrorStatus = NodeCreate(&curNode, OP, MUL, NAN, NULL, NULL, NULL, NULL);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->left, OP, MUL, NAN, NULL, NULL, NULL, curNode);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->left->left, CONST, EMPTY_OP, -1, NULL, NULL, NULL, curNode->left);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCopyErrorStatus = NodeCopy(&curNode->left->right, node);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                   NodeCopyErrorStatus);

            curNode->left->right->opType = SIN;
            curNode->left->right->parent = curNode->left;

            Node* CL = NULL;

            NodeCopyErrorStatus = NodeCopy(&CL, node->left);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                   NodeCopyErrorStatus);
        
            DiffErrorStatus = Diff(CL, &curNode->right);

            curNode->right->parent = curNode;

            break;
        }
        default: {
            return UNKNOWN_OP_TYPE;
        }
    }

    *ptrRes = curNode;
    
    fprintf(stderr, "(Diff): res: %p\n", curNode);
    
    return OK;
}

ERROR_STATUSES DiffTg(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): tg found!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    switch (node->left->nodeType) {
        case CONST: {
            ERROR_STATUSES DiffConstErrorStatus = DiffConst(node, ptrRes);

            ASSERT(DiffConstErrorStatus == OK, GetErrorString(DiffConstErrorStatus), 
                   DiffConstErrorStatus);
            
            return OK;
        }
        case VAR: {
            NodeCreateErrorStatus = NodeCreate(&curNode, OP, DIV, NAN, NULL, NULL, NULL, NULL);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->left, CONST, EMPTY_OP, 1, NULL, NULL, NULL, curNode);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->right, OP, POW, NAN, NULL, NULL, NULL, curNode);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);


            NodeCreateErrorStatus = NodeCreate(&curNode->right->left, OP, COS, NAN, NULL, NULL, NULL, curNode->right);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->right->left->left, VAR, EMPTY_OP, NAN, node->left->varName,
                                               NULL, NULL, curNode->right->left);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->right->right, CONST, EMPTY_OP, 2, NULL,
                                               NULL, NULL, curNode->right);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            break;
        }
        case OP: {
            NodeCreateErrorStatus = NodeCreate(&curNode, OP, MUL, NAN, NULL, NULL, NULL, NULL);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->left, OP, DIV, NAN, NULL, NULL, NULL, NULL);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->left->left, CONST, EMPTY_OP, 1, NULL, NULL, NULL, curNode->left);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->left->right, OP, POW, NAN, NULL, NULL, NULL, curNode->left);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);


            NodeCreateErrorStatus = NodeCreate(&curNode->left->right->left, OP, COS, NAN, NULL, NULL, NULL, curNode->left->right);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            Node* CP = NULL;

            NodeCopyErrorStatus = NodeCopy(&CP, node->left);

            curNode->left->right->left->left = CP;
            CP->parent                       = curNode->left->right->left;
            

            NodeCreateErrorStatus = NodeCreate(&curNode->left->right->right, CONST, EMPTY_OP, 2, NULL,
                                               NULL, NULL, curNode->left->right);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            DiffErrorStatus = Diff(CP, &curNode->right);
            
            curNode->right->parent = curNode;

            break;
        }
        default: {
            return UNKNOWN_OP_TYPE;
        }
    }

    *ptrRes = curNode;

    fprintf(stderr, "(Diff): res: %p\n", curNode);

    return OK;
}
