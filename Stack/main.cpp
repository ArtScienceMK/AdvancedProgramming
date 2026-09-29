#ifdef STACK_DEBUG

#define ON_DEBUG(...)  __VA_ARGS__

#else

#define ON_DEBUG(...)

#define stringify( x ) (# x)

#endif

typedef double StackElement_t;

#include "math.h"

#define POISON NAN
#define CANARY ((StackElement_t) 0xEDA)

#include "MyAssert.h"
#include "MyError.h"
#include "MyStack.h"

#include <stdio.h>

ERROR_STATUSES OkSample(void);
ERROR_STATUSES BadClearInitSample(void);
ERROR_STATUSES NotEnoughMemorySample(void);
ERROR_STATUSES PushEmptySample(void);

int main() {
    
    ERROR_STATUSES sampleErrorStatus = OkSample();

    return 0;
}

ERROR_STATUSES OkSample() {
    Stack_t stk1 = {};
    
    ERROR_STATUSES StackInitErrorStatus = StackInit(&stk1, 5 ON_DEBUG(, __FILE_NAME__, __FUNCTION__, __FILE__, __LINE__));
      // нужно ли менять нейминг в статусах ошибок? Или стоит ввести новое правило? Дед?
    
    ASSERT(StackInitErrorStatus == OK, GetErrorString(StackInitErrorStatus), StackInitErrorStatus);

    double checkValuePush = 5;

    ERROR_STATUSES StackPushErrorStatus = StackPush(&stk1, checkValuePush);

    ASSERT(StackPushErrorStatus == OK, GetErrorString(StackPushErrorStatus), StackPushErrorStatus);

    StackDump(stk1);

    double checkValuePop = 0;
    ERROR_STATUSES StackPopErrorStatus = StackPop(&stk1, &checkValuePop);

    ASSERT(StackPopErrorStatus == OK, GetErrorString(StackPopErrorStatus), StackPopErrorStatus);
    
    ASSERT(checkValuePop == checkValuePush, GetErrorString(PUSH_NOT_EQUAL_POP), PUSH_NOT_EQUAL_POP);

    StackDump(stk1);

    ERROR_STATUSES StackDestroyErrorStatus = StackDestroy(&stk1);

    ASSERT(StackDestroyErrorStatus == OK, GetErrorString(StackDestroyErrorStatus), StackDestroyErrorStatus);

    return OK;
}

ERROR_STATUSES BadClearInitSample() {
    Stack_t stk1 = {};
    
    stk1.capacity = 12;

    ERROR_STATUSES StackInitErrorStatus = StackInit(&stk1, 1 ON_DEBUG(, __FILE_NAME__, __FUNCTION__, __FILE__, __LINE__));
    
    ASSERT(StackInitErrorStatus == OK, GetErrorString(StackInitErrorStatus), StackInitErrorStatus);

    StackDump(stk1);

    ERROR_STATUSES StackDestroyErrorStatus = StackDestroy(&stk1);

    ASSERT(StackDestroyErrorStatus == OK, GetErrorString(StackDestroyErrorStatus), StackDestroyErrorStatus);

    return OK;
}

ERROR_STATUSES NotEnoughMemorySample() {
    Stack_t stk1 = {};
    
    ERROR_STATUSES StackInitErrorStatus = StackInit(&stk1, -1 ON_DEBUG(, __FILE_NAME__, __FUNCTION__, __FILE__, __LINE__));
    
    ASSERT(StackInitErrorStatus == OK, GetErrorString(StackInitErrorStatus), StackInitErrorStatus);

    StackDump(stk1);

    ERROR_STATUSES StackDestroyErrorStatus = StackDestroy(&stk1);

    ASSERT(StackDestroyErrorStatus == OK, GetErrorString(StackDestroyErrorStatus), StackDestroyErrorStatus);

    return OK;
}

ERROR_STATUSES PushEmptySample() {
    Stack_t stk1 = {};
    
    ERROR_STATUSES StackInitErrorStatus = StackInit(&stk1, 3 ON_DEBUG(, __FILE_NAME__, __FUNCTION__, __FILE__, __LINE__));
    
    ASSERT(StackInitErrorStatus == OK, GetErrorString(StackInitErrorStatus), StackInitErrorStatus);

    StackDump(stk1);

    double checkValuePop = 0;

    ERROR_STATUSES StackPopErrorStatus = StackPop(&stk1, &checkValuePop);
    
    ASSERT(StackPopErrorStatus == OK, GetErrorString(StackPopErrorStatus), StackPopErrorStatus);

    ERROR_STATUSES StackDestroyErrorStatus = StackDestroy(&stk1);

    ASSERT(StackDestroyErrorStatus == OK, GetErrorString(StackDestroyErrorStatus), StackDestroyErrorStatus);

    return OK;
}

ERROR_STATUSES StackInit(Stack_t* ptrStack, size_t _capacity ON_DEBUG(, const char* name,
                                                                        const char* function,
                                                                        const char* file, 
                                                                        int         line)) {

    LOG(fprintf(stderr, "%s\n %s:%i %s()\n", name, file, line, function);)    //  Дед, что писать в name??
    //   Как сделать line ctr+click able??                                                                        
    
    ASSERT(ptrStack != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->data == NULL, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrStack->size == 0, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    
    ASSERT(ptrStack->capacity == 0, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);



    ptrStack->capacity = _capacity + 2; // stack initializes with size = 0
    ptrStack-> data = (StackElement_t*)calloc(ptrStack->capacity, sizeof(StackElement_t)); 

    ASSERT(ptrStack->data != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);

    ptrStack->data[0] = CANARY;

    for (int i = 1; i < ptrStack->capacity - 1; i++) {
        ptrStack->data[i] = POISON;
    }

    ptrStack->data[ptrStack->capacity - 1] = CANARY;
    ON_DEBUG(StackVerify(ptrStack));

    return OK;
}

ERROR_STATUSES StackDestroy(Stack_t* ptrStack) {

    ON_DEBUG(StackVerify(ptrStack));

    ASSERT(ptrStack != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    free(ptrStack->data);  // TODO make a wrapper to put a poison to freed memory
    ptrStack->capacity = 0;
    ptrStack->size = 0;

    ptrStack = NULL;
    
    // ON_DEBUG(StackVerify(ptrStack));

    return OK;
}

ERROR_STATUSES StackPush(Stack_t* ptrStack, StackElement_t pushValue) {
    
    ON_DEBUG(StackVerify(ptrStack));

    ASSERT(ptrStack != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->data != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    if (ptrStack->size + 1 == ptrStack->capacity - 1) {
        ptrStack->data = (StackElement_t*)realloc(ptrStack->data, sizeof(StackElement_t) * ptrStack->capacity * 2);
        
        ASSERT(ptrStack->data != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
        
        ptrStack->capacity *= 2;
    }

    ptrStack->data[ptrStack->size + 1] = pushValue;
    ptrStack->size++; 

    ON_DEBUG(StackVerify(ptrStack));

    return OK;
}

ERROR_STATUSES StackPop(Stack_t* ptrStack, double* ptrValue) {

    ON_DEBUG(StackVerify(ptrStack));
    
    ASSERT(ptrStack != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->data != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->size > 0, GetErrorString(EMPTY_STACK), EMPTY_STACK);

    *ptrValue = ptrStack->data[ptrStack->size];

    ptrStack->data[ptrStack->size] = POISON;
    ptrStack->size--;
    
    ON_DEBUG(StackVerify(ptrStack));

    return OK;
}

ERROR_STATUSES StackVerify(Stack_t* ptrStack) {
    ASSERT(ptrStack != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->capacity >= ptrStack->size, GetErrorString(STACK_CAP_SIZE), STACK_CAP_SIZE);

    ASSERT(ptrStack->data != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    
    ASSERT(ptrStack->data[0] == CANARY, GetErrorString(BAD_CANARY), BAD_CANARY);
    ASSERT(ptrStack->data[ptrStack->capacity - 1] == CANARY, GetErrorString(BAD_CANARY), BAD_CANARY);

    return OK;
}

ERROR_STATUSES StackDump(Stack_t stack) {
    ON_DEBUG(StackVerify(&stack);)

    LOG(fprintf(stderr, "\n-----------\n");)
    LOG(fprintf(stderr, "(StackDump): Starting...\n");)

    LOG(fprintf(stderr, "Stack_t <stack> [%p] %s at %s {\n", &stack, __FUNCTION__, __FILE__);)  //  TODO make stringify

    LOG(fprintf(stderr, "capacity = %i\n", stack.capacity);)
    LOG(fprintf(stderr, "    size = %i\n", stack.size);)

    LOG(fprintf(stderr, "%s *[0] = %lf (CANARY)\n", ORANGE, stack.data[0]);)

    LOG(fprintf(stderr, "%s", GREEN);)

    for (int i = 1; i <= stack.size; i++) {
        LOG(fprintf(stderr, "*[%i] = %lf\n", i, stack.data[i]);)
    }

    LOG(fprintf(stderr, "%s", RED);)

    for (int i = stack.size + 1; i < stack.capacity - 1; i++) {
        LOG(fprintf(stderr, "[%i] = %lf (POISON)\n", i, stack.data[i]);)
    }

    LOG(fprintf(stderr, "%s *[%i] = %lf (CANARY)\n",
                ORANGE, stack.capacity - 1, stack.data[stack.capacity - 1]);)

    LOG(fprintf(stderr, "%s", RESET);)

    LOG(fprintf(stderr, "}\n");)

    LOG(fprintf(stderr, "(StackDump): Ending...\n");)

    return OK;
}
