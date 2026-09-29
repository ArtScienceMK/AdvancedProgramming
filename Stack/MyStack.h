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
};

ERROR_STATUSES StackInit(Stack_t* ptrStack, size_t _capacity ON_DEBUG(, const char* name,
                                                                        const char* function,
                                                                        const char* file,
                                                                        int         line));
ERROR_STATUSES StackDestroy(Stack_t* ptrStack); 

ERROR_STATUSES StackPush(Stack_t* ptrStack, StackElement_t pushValue);
ERROR_STATUSES StackPop(Stack_t* ptrStack, double* ptrValue);

ERROR_STATUSES StackVerify(Stack_t* ptrStack);
ERROR_STATUSES StackDump(Stack_t stack);

#endif /*MY_STACK_H*/