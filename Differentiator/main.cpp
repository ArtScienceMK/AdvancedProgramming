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

ERROR_STATUSES Diff(Node* node, Node** ptrRes);

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
    Node* curNode = NULL;

    switch (node->nodeType) {
        case CONST: {
            fprintf(stderr, "(Diff): constant found!\n");

            NodeCreateErrorStatus = NodeCreate(&curNode, CONST, EMPTY_OP, 0, NULL, NULL, NULL, NULL);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);
            
            *ptrRes = curNode;

            fprintf(stderr, "(Diff): res: %p\n", curNode);

            break;
        }  
        case VAR: {
            fprintf(stderr, "(Diff): variable found!\n");

            NodeCreateErrorStatus = NodeCreate(&curNode, CONST, EMPTY_OP, 1, NULL, NULL, NULL, NULL);

            ASSERT(NodeCreateErrorStatus == OK, GetErrorString(NodeCreateErrorStatus), 
                   NodeCreateErrorStatus);
            
            *ptrRes = curNode;

            fprintf(stderr, "(Diff): res: %p\n", curNode);

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
            fprintf(stderr, "(Diff): + found!\n");

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
            break;
        }
        case SUB: {
            fprintf(stderr, "(Diff): - found!\n");

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
            break;
        }
        case MUL: {
            fprintf(stderr, "(Diff): + found!\n");

            Node* CL = NULL, *CR = NULL, *DL = NULL, *DR = NULL;
            
            ERROR_STATUSES NodeCopyErrorStatus = NodeCopy(&CL, node->left);

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
            break;
        }
    }

    return OK;
}
