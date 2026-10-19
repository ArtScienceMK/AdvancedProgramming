#ifndef MY_STACK_H
#define MY_STACK_H

struct Stack_t {
    #ifdef STACK_DEBUG
        const char* name;
        const char* file;
        int         line;
    #endif

    StackElement_t* data;
    size_t size;
    size_t capacity;

    const char* status;

    unsigned long long hash; 
};

ERROR_STATUSES StackInit(Stack_t* ptrStack, size_t _capacity ON_DEBUG(, SourceLocation sourceLocation));
ERROR_STATUSES StackDestroy(Stack_t* ptrStack); 

ERROR_STATUSES StackPush(Stack_t* ptrStack, StackElement_t pushValue);
ERROR_STATUSES StackPop(Stack_t* ptrStack, StackElement_t* ptrValue);

ERROR_STATUSES StackVerify(Stack_t* ptrStack);
ERROR_STATUSES StackVerifyWithoutHash(Stack_t* ptrStack);

ERROR_STATUSES StackDump(Stack_t stack, SourceLocation sourceLocation);

ERROR_STATUSES CountHash(Stack_t* ptrStack);

#endif /*MY_STACK_H*/