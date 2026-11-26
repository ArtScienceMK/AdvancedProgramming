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

#define CHECK_CREATE ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),\
                            NodeCreateErrorStatus)

#define CHECK_COPY ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus),\
                          NodeCopyErrorStatus)
    
#define CHECK_DIFF ASSERT(DiffErrorStatus == OK, GetErrorString(DiffErrorStatus),\
                          DiffErrorStatus)

#define CONST(node, value, parent) (NodeCreateErrorStatus = NodeCreate(&node, CONST, EMPTY_OP, value, NULL, NULL, NULL, parent));\
                   CHECK_CREATE

#define VAR(node, varName, parent) (NodeCreateErrorStatus = NodeCreate(&node, VAR, EMPTY_OP, NAN, varName, NULL, NULL, parent));\
                   CHECK_CREATE

#define ADD(node, parent) (NodeCreateErrorStatus = NodeCreate(&node, OP, OP_ADD, NAN, NULL, NULL, NULL, parent));\
                   CHECK_CREATE

#define SUB(node, parent) (NodeCreateErrorStatus = NodeCreate(&node, OP, OP_SUB, NAN, NULL, NULL, NULL, parent));\
                   CHECK_CREATE

#define MUL(node, parent) (NodeCreateErrorStatus = NodeCreate(&node, OP, OP_MUL, NAN, NULL, NULL, NULL, parent));\
                   CHECK_CREATE

#define DIV(node, parent) (NodeCreateErrorStatus = NodeCreate(&node, OP, OP_DIV, NAN, NULL, NULL, NULL, parent));\
                   CHECK_CREATE

#define POW(node, parent) (NodeCreateErrorStatus = NodeCreate(&node, OP, OP_POW, NAN, NULL, NULL, NULL, parent));\
                   CHECK_CREATE

#define LOG(node, parent) (NodeCreateErrorStatus = NodeCreate(&node, OP, OP_LOG, NAN, NULL, NULL, NULL, parent));\
                   CHECK_CREATE

#define SIN(node, parent) (NodeCreateErrorStatus = NodeCreate(&node, OP, OP_SIN, NAN, NULL, NULL, NULL, parent));\
                   CHECK_CREATE

#define COS(node, parent) (NodeCreateErrorStatus = NodeCreate(&node, OP, OP_COS, NAN, NULL, NULL, NULL, parent));\
                   CHECK_CREATE
    
#define TG( node, parent) (NodeCreateErrorStatus = NodeCreate(&node, OP, OP_TG,  NAN, NULL, NULL, NULL, parent));\
                   CHECK_CREATE
            
#define CTG(node, parent) (NodeCreateErrorStatus = NodeCreate(&node, OP, OP_CTG, NAN, NULL, NULL, NULL, parent));\
                   CHECK_CREATE
        
#define SH(node, parent) (NodeCreateErrorStatus = NodeCreate(&node, OP, OP_SH, NAN, NULL, NULL, NULL, parent));\
                   CHECK_CREATE

#define CH(node, parent) (NodeCreateErrorStatus = NodeCreate(&node, OP, OP_CH, NAN, NULL, NULL, NULL, parent));\
                   CHECK_CREATE

                   
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

ERROR_STATUSES Diff(        Node* node, Node** ptrRes);

ERROR_STATUSES DiffConst(   Node* node, Node** ptrRes);
ERROR_STATUSES DiffVar  (   Node* node, Node** ptrRes);
ERROR_STATUSES DiffOp   (   Node* node, Node** ptrRes);

ERROR_STATUSES DiffAdd(     Node* node, Node** ptrRes);
ERROR_STATUSES DiffSub(     Node* node, Node** ptrRes);
ERROR_STATUSES DiffMul(     Node* node, Node** ptrRes);
ERROR_STATUSES DiffDiv(     Node* node, Node** ptrRes);

ERROR_STATUSES DiffPow(     Node* node, Node** ptrRes);
ERROR_STATUSES DiffLog(     Node* node, Node** ptrRes);

ERROR_STATUSES DiffSin(     Node* node, Node** ptrRes);
ERROR_STATUSES DiffCos(     Node* node, Node** ptrRes);
ERROR_STATUSES DiffTg (     Node* node, Node** ptrRes);
ERROR_STATUSES DiffCtg(     Node* node, Node** ptrRes);

ERROR_STATUSES DiffArcsin(  Node* node, Node** ptrRes);
ERROR_STATUSES DiffArccos(  Node* node, Node** ptrRes);
ERROR_STATUSES DiffArctg(   Node* node, Node** ptrRes);
ERROR_STATUSES DiffArcctg(  Node* node, Node** ptrRes);

ERROR_STATUSES DiffSh(      Node* node, Node** ptrRes);
ERROR_STATUSES DiffCh(      Node* node, Node** ptrRes);
ERROR_STATUSES DiffTh(      Node* node, Node** ptrRes);


int main() {
    Tree tree = {};

    LoadTree(&tree);

    Tree diffTree = {};
    TreeInit(&diffTree);

    Diff(tree.root, &(diffTree.root));

    // fprintf(stderr, "diffTree.root: %p\n", diffTree.root);

    TreeVizDump(&diffTree); 

    SaveTree(&diffTree);

    TreeDestroy(&tree);
    TreeDestroy(&diffTree);
    
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
            ERROR_STATUSES DiffOpErrorStatus = DiffOp(node, ptrRes);

            ASSERT(DiffOpErrorStatus == OK, GetErrorString(DiffOpErrorStatus), DiffOpErrorStatus);

            break;
        }

        default: {
            return UNKNOWN_NODE_TYPE;
        }
    }

    return OK;
}

ERROR_STATUSES DiffConst(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): constant found!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK;
    
    Node* curNode = NULL;

    NodeCreateErrorStatus = NodeCreate(&curNode, CONST, EMPTY_OP, 0, NULL, NULL, NULL, NULL);

    CHECK_CREATE

    *ptrRes = curNode;

    fprintf(stderr, "(Diff): res: %p\n", curNode);
    return OK;
}

ERROR_STATUSES DiffVar(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): variable found!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK;

    Node* curNode = NULL;

    NodeCreateErrorStatus = NodeCreate(&curNode, CONST, EMPTY_OP, 1, NULL, NULL, NULL, NULL);

    CHECK_CREATE
    
    *ptrRes = curNode;

    fprintf(stderr, "(Diff): res: %p\n", curNode);

    return OK;
}

ERROR_STATUSES DiffOp(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): operator found!\n");

    switch (node->opType) {
        case OP_ADD: {
            ERROR_STATUSES DiffAddErrorStatus = DiffAdd(node, ptrRes);

            ASSERT(DiffAddErrorStatus == OK, GetErrorString(DiffAddErrorStatus), 
                   DiffAddErrorStatus);

            break;
        }
        case OP_SUB: {
            ERROR_STATUSES DiffSubErrorStatus = DiffSub(node, ptrRes);

            ASSERT(DiffSubErrorStatus == OK, GetErrorString(DiffSubErrorStatus), 
                   DiffSubErrorStatus);

            break;
        }
        case OP_MUL: {
            ERROR_STATUSES DiffMulErrorStatus = DiffMul(node, ptrRes);

            ASSERT(DiffMulErrorStatus == OK, GetErrorString(DiffMulErrorStatus), 
                   DiffMulErrorStatus);

            break;
        }
        case OP_DIV: {
            ERROR_STATUSES DiffDivErrorStatus = DiffDiv(node, ptrRes);

            ASSERT(DiffDivErrorStatus == OK, GetErrorString(DiffDivErrorStatus), 
                   DiffDivErrorStatus);
            
            break;
        }
        case OP_POW: {
            ERROR_STATUSES DiffPowErrorStatus = DiffPow(node, ptrRes);

            ASSERT(DiffPowErrorStatus == OK, GetErrorString(DiffPowErrorStatus),
                   DiffPowErrorStatus);

            break;
        }
        case OP_LOG: {
            ERROR_STATUSES DiffLogErrorStatus = DiffLog(node, ptrRes);

            ASSERT(DiffLogErrorStatus == OK, GetErrorString(DiffLogErrorStatus), 
                   DiffLogErrorStatus);

            break;
        }
        case OP_SIN: {
            ERROR_STATUSES DiffSinErrorStatus = DiffSin(node, ptrRes);

            ASSERT(DiffSinErrorStatus == OK, GetErrorString(DiffSinErrorStatus), 
                   DiffSinErrorStatus);

            break;
        }
        case OP_COS: {
            ERROR_STATUSES DiffCosErrorStatus = DiffCos(node, ptrRes);

            ASSERT(DiffCosErrorStatus == OK, GetErrorString(DiffCosErrorStatus), 
                   DiffCosErrorStatus);
                
            break;
        }
        case OP_TG: {
            ERROR_STATUSES DiffTgErrorStatus = DiffTg(node, ptrRes);

            ASSERT(DiffTgErrorStatus == OK, GetErrorString(DiffTgErrorStatus), 
                   DiffTgErrorStatus);

            break;
        }
        case OP_CTG: {
            ERROR_STATUSES DiffCtgErrorStatus = DiffCtg(node, ptrRes);

            ASSERT(DiffCtgErrorStatus == OK, GetErrorString(DiffCtgErrorStatus), 
                   DiffCtgErrorStatus);
                
            break;
        }
        case OP_ARCSIN: {
            ERROR_STATUSES DiffArcsinErrorStatus = DiffArcsin(node, ptrRes);

            ASSERT(DiffArcsinErrorStatus == OK, GetErrorString(DiffArcsinErrorStatus), 
                   DiffArcsinErrorStatus);
                
            break;
        }
        case OP_ARCCOS: {
            ERROR_STATUSES DiffArccosErrorStatus = DiffArccos(node, ptrRes);

            ASSERT(DiffArccosErrorStatus == OK, GetErrorString(DiffArccosErrorStatus), 
                   DiffArccosErrorStatus);
                
            break;
        }
        case OP_ARCTG: {
            ERROR_STATUSES DiffArctgErrorStatus = DiffArctg(node, ptrRes);

            ASSERT(DiffArctgErrorStatus == OK, GetErrorString(DiffArctgErrorStatus), 
                   DiffArctgErrorStatus);
                
            break;
        }
        case OP_ARCCTG: {
            ERROR_STATUSES DiffArcctgErrorStatus = DiffArcctg(node, ptrRes);

            ASSERT(DiffArcctgErrorStatus == OK, GetErrorString(DiffArcctgErrorStatus), 
                   DiffArcctgErrorStatus);

            break;
        }
        case OP_SH: {
            ERROR_STATUSES DiffShErrorStatus = DiffSh(node, ptrRes);

            ASSERT(DiffShErrorStatus == OK, GetErrorString(DiffShErrorStatus), 
                   DiffShErrorStatus);

            break;
        }
        case OP_CH: {
            ERROR_STATUSES DiffChErrorStatus = DiffCh(node, ptrRes);

            ASSERT(DiffChErrorStatus == OK, GetErrorString(DiffChErrorStatus), 
                   DiffChErrorStatus);

            break;
        }
        case OP_TH: {
            ERROR_STATUSES DiffThErrorStatus = DiffTh(node, ptrRes);

            ASSERT(DiffThErrorStatus == OK, GetErrorString(DiffThErrorStatus), 
                   DiffThErrorStatus);

            break;
        }
        default: {
            return UNKNOWN_OP_TYPE;
        }

    }

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

    NodeCreateErrorStatus = NodeCreate(&curNode, OP, OP_ADD, NAN, NULL, leftSon, rightSon, NULL);

    CHECK_CREATE
    
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

    NodeCreateErrorStatus = NodeCreate(&curNode, OP, OP_SUB, NAN, NULL, leftSon, rightSon, NULL);

    CHECK_CREATE
    
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

    NodeCreateErrorStatus = NodeCreate(&curNode, OP, OP_ADD, NAN, NULL, NULL, NULL, NULL);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->left, OP, OP_MUL, NAN, NULL, DL, CR, curNode);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->right, OP, OP_MUL, NAN, NULL, CL, DR, curNode);

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

    NodeCreateErrorStatus = NodeCreate(&curNode, OP, OP_DIV, NAN, NULL, NULL, NULL, NULL);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->left, OP, OP_SUB, NAN, NULL, NULL, NULL, curNode);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->left->left, OP, OP_MUL, NAN, NULL, DL, CR, curNode->left);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);
    
    NodeCreateErrorStatus = NodeCreate(&curNode->left->right, OP, OP_MUL, NAN, NULL, CL, DR, curNode->left);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->right, OP, OP_POW, NAN, NULL, CCR, NULL, curNode);

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
            MUL(curNode, NULL);
            
            Node* CP = NULL;
            
            NodeCopyErrorStatus = NodeCopy(&CP, node);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                    NodeCopyErrorStatus);

            curNode->left = CP;
            CP->parent    = curNode;

            NodeCreateErrorStatus = NodeCreate(&curNode->right, OP, OP_LOG, NAN, NULL, NULL, NULL, NULL);
        
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
            MUL(curNode, NULL);
            
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
            MUL(curNode, NULL);

            Node* CP = NULL;

            NodeCopyErrorStatus = NodeCopy(&CP, node);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                    NodeCopyErrorStatus);

            curNode->left = CP;
            CP->parent    = curNode;

            NodeCreateErrorStatus = NodeCreate(&curNode->right, OP, OP_ADD, NAN, NULL, NULL, NULL, curNode);
        
            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
                    NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->right->left, OP, OP_LOG, NAN, NULL, NULL, NULL, curNode->right);
        
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

        CHECK_CREATE

        *ptrRes = curNode;

        fprintf(stderr, "(Diff): res: %p\n", curNode);

        return OK;
    }

    NodeCreateErrorStatus = NodeCreate(&curNode, OP, OP_DIV, NAN, NULL, NULL, NULL, NULL);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
           NodeCreateErrorStatus);

    NodeCreateErrorStatus = NodeCreate(&curNode->left, CONST, EMPTY_OP, 1, NULL, NULL, NULL, curNode);

    ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);
        
    MUL(curNode->right, curNode);
            
    if (node->left->nodeType == CONST) {
        NodeCreateErrorStatus = NodeCreate(&curNode->right->left, CONST, EMPTY_OP, node->left->value, NULL, NULL, NULL, curNode->right);

        ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus),
            NodeCreateErrorStatus);

        NodeCreateErrorStatus = NodeCreate(&curNode->right->right, OP, OP_LOG, NAN, NULL, NULL, NULL, curNode->right);

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

        NodeCreateErrorStatus = NodeCreate(&curNode->right->right, OP, OP_LOG, NAN, NULL, NULL, NULL, curNode->right);

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

            curNode->opType = OP_COS;

            break;
        }
        case OP: {
            MUL(curNode, NULL);
            
            NodeCopyErrorStatus = NodeCopy(&curNode->left, node);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                   NodeCopyErrorStatus);

            curNode->left->opType = OP_COS;
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
            MUL(curNode, NULL);
            
            NodeCreateErrorStatus = NodeCreate(&curNode->left, CONST, EMPTY_OP, -1, NULL, NULL, NULL, curNode);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);
                   
            NodeCopyErrorStatus = NodeCopy(&curNode->right, node);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                   NodeCopyErrorStatus);

            curNode->right->opType = OP_SIN;
            curNode->right->parent = curNode;

            break;
        }
        case OP: {
            MUL(curNode, NULL);
            
            MUL(curNode->left, curNode);

            NodeCreateErrorStatus = NodeCreate(&curNode->left->left, CONST, EMPTY_OP, -1, NULL, NULL, NULL, curNode->left);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCopyErrorStatus = NodeCopy(&curNode->left->right, node);

            ASSERT(NodeCopyErrorStatus == OK, GetErrorString(NodeCopyErrorStatus), 
                   NodeCopyErrorStatus);

            curNode->left->right->opType = OP_SIN;
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
            NodeCreateErrorStatus = NodeCreate(&curNode, OP, OP_DIV, NAN, NULL, NULL, NULL, NULL);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->left, CONST, EMPTY_OP, 1, NULL, NULL, NULL, curNode);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);

            NodeCreateErrorStatus = NodeCreate(&curNode->right, OP, OP_POW, NAN, NULL, NULL, NULL, curNode);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);


            NodeCreateErrorStatus = NodeCreate(&curNode->right->left, OP, OP_COS, NAN, NULL, NULL, NULL, curNode->right);

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
            MUL(curNode, NULL);

            DIV(curNode->left, curNode);

            CONST(curNode->left->left, 1, curNode->left);

            POW(curNode->left->right, curNode->left);

            COS(curNode->left->right->left, curNode->left->right);

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

ERROR_STATUSES DiffCtg(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): found ctg!\n");

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
            DIV(curNode, NULL);

            CONST(curNode->left, -1, curNode);

            POW(curNode->right, curNode);

            SIN(curNode->right->left, curNode->right);

            VAR(curNode->right->left->left, node->left->varName, curNode->right->left);

            CONST(curNode->right->right, 2, curNode->right);

            break;
        }
        case OP: {
            MUL(curNode, NULL);

            DIV(curNode->left, curNode);

            CONST(curNode->left->left, -1, curNode->left);

            POW(curNode->left->right, curNode->left);

            SIN(curNode->left->right->left, curNode->left->right);

            NodeCopyErrorStatus = NodeCopy(&curNode->left->right->left->left, node->left);
            curNode->left->right->left->left->parent = curNode->left->right->left;
            CHECK_COPY

            CONST(curNode->left->right->right, 2, curNode->left->right);

            Node* CP = NULL;

            NodeCopyErrorStatus = NodeCopy(&CP, node->left);

            CHECK_COPY

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

ERROR_STATUSES DiffArcsin(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): found arcsin!\n");

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
            DIV(curNode, NULL);

            CONST(curNode->left, 1, curNode);

            POW(curNode->right, curNode);

            SUB(curNode->right->left, curNode->right);

            CONST(curNode->right->left->left, 1, curNode->right->left);

            POW(curNode->right->left->right, curNode->right->left);

            VAR(curNode->right->left->right->left, node->left->varName, curNode->right->left->right);

            CONST(curNode->right->left->right->right, 2, curNode->right->left->right);

            CONST(curNode->right->right, (double)1 / 2, curNode->right);

            break;
        }
        case OP: {
            MUL(curNode, NULL);

            DIV(curNode->left, curNode);

            CONST(curNode->left->left, 1, curNode->left);

            POW(curNode->left->right, curNode->left);

            SUB(curNode->left->right->left, curNode->left->right);

            CONST(curNode->left->right->left->left, 1, curNode->left->right->left);

            POW(curNode->left->right->left->right, curNode->left->right->left);

            Node* CP = NULL;

            NodeCopyErrorStatus = NodeCopy(&CP, node->left);

            CHECK_COPY

            curNode->left->right->left->right->left         = CP;
            curNode->left->right->left->right->left->parent = curNode->left->right->left->right;

            CONST(curNode->left->right->left->right->right, 2, curNode->left->right->left->right);

            CONST(curNode->left->right->right, (double)1 / 2, curNode->left->right);

            DiffErrorStatus = Diff(CP, &curNode->right);

            CHECK_DIFF
            
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

ERROR_STATUSES DiffArccos(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): found arccos!\n");

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
            MUL(curNode, NULL);

            CONST(curNode->left, -1, curNode);

            ERROR_STATUSES DiffArcsinErrorStatus = DiffArcsin(node, &curNode->right);

            ASSERT(DiffArcsinErrorStatus == OK, GetErrorString(DiffArcsinErrorStatus), 
                   DiffArcsinErrorStatus);

            break;
        }
        case OP: {
            MUL(curNode, NULL);

            CONST(curNode->left, -1, curNode);

            ERROR_STATUSES DiffArcsinErrorStatus = DiffArcsin(node, &curNode->right);

            ASSERT(DiffArcsinErrorStatus == OK, GetErrorString(DiffArcsinErrorStatus), 
                   DiffArcsinErrorStatus);

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

ERROR_STATUSES DiffArctg(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): found arctg!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    switch (node->left->nodeType) {
        case CONST: {
            ERROR_STATUSES DiffConstErrorStatus = DiffConst(node, ptrRes);

            ASSERT(DiffConstErrorStatus == OK, GetErrorString(DiffConstErrorStatus), 
                   DiffConstErrorStatus);
            
            return OK;
        }
        case VAR:

        case OP: {
            MUL(curNode, NULL);

            DIV(curNode->left, curNode);

            CONST(curNode->left->left, 1, curNode->left);

            ADD(curNode->left->right, curNode->left);

            CONST(curNode->left->right->left, 1, curNode->left->right);

            POW(curNode->left->right->right, curNode->left->right);

            Node* CP = NULL;

            NodeCopyErrorStatus = NodeCopy(&CP, node->left);

            CHECK_COPY
            
            curNode->left->right->right->left = CP;
            curNode->left->right->right->left->parent = curNode->left->right->right;

            CONST(curNode->left->right->right->right, 2, curNode->left->right->right);

            DiffErrorStatus = Diff(CP, &curNode->right);

            CHECK_DIFF

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

ERROR_STATUSES DiffArcctg(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): found arcctg!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    switch (node->left->nodeType) {
        case CONST: {
            ERROR_STATUSES DiffConstErrorStatus = DiffConst(node, ptrRes);

            ASSERT(DiffConstErrorStatus == OK, GetErrorString(DiffConstErrorStatus), 
                   DiffConstErrorStatus);
            
            return OK;
        }
        case VAR:

        case OP: {
            MUL(curNode, NULL);

            CONST(curNode->left, -1, curNode);

            ERROR_STATUSES DiffArctgErrorStatus = DiffArctg(node, &curNode->right);

            ASSERT(DiffArctgErrorStatus == OK, GetErrorString(DiffArctgErrorStatus), 
                   DiffArctgErrorStatus);

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

ERROR_STATUSES DiffSh(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): found sh!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    switch (node->left->nodeType) {
        case CONST: {
            ERROR_STATUSES DiffConstErrorStatus = DiffConst(node, ptrRes);

            ASSERT(DiffConstErrorStatus == OK, GetErrorString(DiffConstErrorStatus), 
                   DiffConstErrorStatus);
            
            return OK;
        }
        case VAR:

        case OP: {
            MUL(curNode, NULL);

            CH(curNode->left, curNode);

            Node* CP = NULL;

            NodeCopyErrorStatus = NodeCopy(&CP, node->left);

            CHECK_COPY

            curNode->left->left         = CP;
            curNode->left->left->parent = curNode->left;

            DiffErrorStatus = Diff(node->left, &curNode->right);

            CHECK_DIFF
            
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

ERROR_STATUSES DiffCh(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): found ch!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    switch (node->left->nodeType) {
        case CONST: {
            ERROR_STATUSES DiffConstErrorStatus = DiffConst(node, ptrRes);

            ASSERT(DiffConstErrorStatus == OK, GetErrorString(DiffConstErrorStatus), 
                   DiffConstErrorStatus);
            
            return OK;
        }
        case VAR:

        case OP: {
            MUL(curNode, NULL);

            SH(curNode->left, curNode);

            Node* CP = NULL;

            NodeCopyErrorStatus = NodeCopy(&CP, node->left);

            CHECK_COPY

            curNode->left->left         = CP;
            curNode->left->left->parent = curNode->left;

            DiffErrorStatus = Diff(node->left, &curNode->right);

            CHECK_DIFF
            
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

ERROR_STATUSES DiffTh(Node* node, Node** ptrRes) {
    fprintf(stderr, "(Diff): found th!\n");

    ERROR_STATUSES NodeCreateErrorStatus = OK, NodeCopyErrorStatus = OK, DiffErrorStatus = OK;

    Node* curNode = NULL;

    switch (node->left->nodeType) {
        case CONST: {
            ERROR_STATUSES DiffConstErrorStatus = DiffConst(node, ptrRes);

            ASSERT(DiffConstErrorStatus == OK, GetErrorString(DiffConstErrorStatus), 
                   DiffConstErrorStatus);
            
            return OK;
        }
        case VAR:

        case OP: {
            MUL(curNode, NULL);

            DIV(curNode->left, curNode);

            CONST(curNode->left->left, -1, curNode->left);

            POW(curNode->left->right, curNode->left);

            SH(curNode->left->right->left, curNode->left->right);

            Node* CP = NULL;

            NodeCopyErrorStatus = NodeCopy(&CP, node->left);

            CHECK_COPY 

            curNode->left->right->left->left         = CP;
            curNode->left->right->left->left->parent = curNode->left->right->left;

            CONST(curNode->left->right->right, 2, curNode->left->right);

            DiffErrorStatus = Diff(CP, &curNode->right);

            CHECK_DIFF

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
