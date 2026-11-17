#ifdef STACK_DEBUG

#define ON_DEBUG(...)  __VA_ARGS__

#else

#define ON_DEBUG(...)

#endif

#ifdef CANARY_DEBUG

#define ON_CANARY(...) __VA_ARGS__

#else

#define ON_CANARY(...)

#endif

#ifdef HASH_DEBUG

#define ON_HASH(...)   __VA_ARGS__

#else

#define ON_HASH(...)

#endif

#define STRINGIFY( x ) (# x)
#define GET_SOURCE_LOCATION(sourceLocation, varName)\
    sourceLocation.file = strdup(__FILE__);\
    sourceLocation.function = strdup(__FUNCTION__);\
    sourceLocation.line = __LINE__;\
    sourceLocation.name = strdup(STRINGIFY(varName));
    
    
    
#include <math.h>
#include <string.h>

typedef size_t StackElement_t;

#define POISON NAN
#define CANARY ((StackElement_t) 0xEDA)

const double Eps = 1e-3;

#include "MyAssert.h"
#include "MyError.h"
#include "MyStack.h"

#include <stdio.h>
#include <unistd.h>

void SpoilCanary(Stack_t* ptrStack);

unsigned long long GetHash(Stack_t* ptrStack);

// int main() {
    
//     HashesSample();

//     return 0;
// }


ERROR_STATUSES StackInit(Stack_t* ptrStack, size_t _capacity ON_DEBUG(, SourceLocation sourceLocation)) {

    LOG(fprintf(stderr, "(StackInit): want to init %s\n Called from %s:%i %s()\n", 
        sourceLocation.name, sourceLocation.file, sourceLocation.line, sourceLocation.function);)    //  Дед, что писать в name??
    //   Как сделать line ctr+click able??                                                                        
    
    ASSERT(ptrStack != NULL,         GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->data == NULL,   GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrStack->size == 0,      GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    
    ASSERT(ptrStack->capacity == 0,  GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrStack->status == NULL, GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);
    ASSERT(ptrStack->hash == 0,      GetErrorString(BAD_CLEAR_INIT), BAD_CLEAR_INIT);                                                             
    
    
    ptrStack->capacity = _capacity + 2; // stack initializes with size = 0
    ptrStack-> data = (StackElement_t*)calloc(ptrStack->capacity, sizeof(StackElement_t)); 
    
    ASSERT(ptrStack->data != NULL,   GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
    
    ptrStack->data[0] = CANARY;
    
    for (size_t i = 1; i < ptrStack->capacity - 1; i++) {
        ptrStack->data[i] = POISON;
    }
    
    ptrStack->data[ptrStack->capacity - 1] = CANARY;
    ptrStack->status = "Initialized\n";  //  Здесь strdup не нужен
    
    ON_HASH(ptrStack->hash = GetHash(ptrStack));
    
    ON_HASH(LOG(fprintf(stderr, "HASH IN INIT FUNC: %llu\n", ptrStack->hash);))

    ON_DEBUG(StackVerify(ptrStack));    

    return OK;
}

ERROR_STATUSES StackDestroy(Stack_t* ptrStack) {

    ON_DEBUG(StackVerify(ptrStack));

    ASSERT(ptrStack != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    free(ptrStack->data);  // TODO make a wrapper to put a poison to freed memory
    ptrStack->capacity = 0;
    ptrStack->size = 0;
    ptrStack->status = "Destroyed\n";
    ptrStack->hash = 0;

    ptrStack = NULL;
    
    // ON_DEBUG(StackVerify(ptrStack));

    return OK;
}

ERROR_STATUSES StackPush(Stack_t* ptrStack, StackElement_t pushValue) {
    
    ON_DEBUG(StackVerify(ptrStack));

    ASSERT(ptrStack != NULL,       GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->data != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    if (ptrStack->size + 1 == ptrStack->capacity - 1) {
        ptrStack->data = (StackElement_t*)realloc(ptrStack->data, sizeof(StackElement_t) * ptrStack->capacity * 2);
        
        ASSERT(ptrStack->data != NULL, GetErrorString(NOT_ENOUGH_MEMORY), NOT_ENOUGH_MEMORY);
        
        ptrStack->capacity *= 2;
    }

    ptrStack->data[ptrStack->size + 1] = pushValue;
    ptrStack->size++; 

    ON_HASH(ptrStack->hash = GetHash(ptrStack);)

    ON_DEBUG(StackVerify(ptrStack));

    return OK;
}

ERROR_STATUSES StackPop(Stack_t* ptrStack, StackElement_t* ptrValue) {

    ON_DEBUG(StackVerify(ptrStack));
    
    ASSERT(ptrStack != NULL,       GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->data != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->size > 0,     GetErrorString(EMPTY_STACK), EMPTY_STACK);

    *ptrValue = ptrStack->data[ptrStack->size];

    ptrStack->data[ptrStack->size] = POISON;
    ptrStack->size--;
    
    ON_HASH(ptrStack->hash = GetHash(ptrStack);)

    ON_DEBUG(StackVerify(ptrStack));

    return OK;
}

ERROR_STATUSES StackVerify(Stack_t* ptrStack) {

    // LOG(fprintf(stderr, "(StackVerify) starting...\n");)
    
    StackVerifyWithoutHash(ptrStack);

    ON_HASH(ASSERT(ptrStack->hash == GetHash(ptrStack), GetErrorString(HASHES_DIFFER), HASHES_DIFFER);)
    
    return OK;
}

ERROR_STATUSES StackVerifyWithoutHash(Stack_t* ptrStack) {

    // LOG(fprintf(stderr, "(StackVerify) starting...\n");)

    ASSERT(ptrStack != NULL,         GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->data   != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);
    ASSERT(ptrStack->status != NULL, GetErrorString(OUT_OF_RANGE), OUT_OF_RANGE);

    ASSERT(ptrStack->capacity >= ptrStack->size, GetErrorString(STACK_CAP_SIZE), STACK_CAP_SIZE);
    
    ON_CANARY(ASSERT(ptrStack->data[0] == CANARY, GetErrorString(BAD_CANARY), BAD_CANARY);)
    ON_CANARY(ASSERT(ptrStack->data[ptrStack->capacity - 1] == CANARY, GetErrorString(BAD_CANARY), BAD_CANARY);)
    
    return OK;
}

ERROR_STATUSES StackDump(Stack_t stack, SourceLocation sourceLocation) {
    ON_DEBUG(StackVerify(&stack));

    LOG(fprintf(stderr, "\n-----------\n");)
    LOG(fprintf(stderr, "(StackDump): Starting...\n");)

    LOG(fprintf(stderr, "Stack_t <%s> [%p] %s at %s {\n",
                sourceLocation.name, &stack, sourceLocation.function, sourceLocation.file);)  //  TODO make STRINGIFY

    LOG(fprintf(stderr, "capacity = %zu\n", stack.capacity);)
    LOG(fprintf(stderr, "    size = %zu\n", stack.size);)
    LOG(fprintf(stderr, "    hash = %llu\n", stack.hash);)

    if (isatty(fileno(stderr))) {
        LOG(fprintf(stderr, "%s", ORANGE);)
    }

    if (stack.data[0] != CANARY) {
        LOG(fprintf(stderr, "*[0] = %lf (BAD CANARY) should be %lf\n", stack.data[0], CANARY);)               
    } else {
        LOG(fprintf(stderr, "*[0] = %lf (CANARY)\n", stack.data[0]);)
    }

    if (isatty(fileno(stderr))) {
        LOG(fprintf(stderr, "%s", GREEN);)
    }

    for (size_t i = 1; i <= stack.size; i++) {
        LOG(fprintf(stderr, "*[%zu] = %lf\n", i, stack.data[i]);)
    }

    if (isatty(fileno(stderr))) {
        LOG(fprintf(stderr, "%s", RED);)
    }

    for (size_t i = stack.size + 1; i + 1 < stack.capacity; i++) {
        LOG(fprintf(stderr, "[%zu] = %lf (POISON)\n", i, stack.data[i]);)
    }

    if (isatty(fileno(stderr))) {
        LOG(fprintf(stderr, "%s", ORANGE);)
    }

    if (stack.data[stack.capacity - 1] != CANARY) {
        LOG(fprintf(stderr, "*[%zu] = %lf (BAD CANARY) should be %lf\n",
                    stack.capacity - 1, stack.data[stack.capacity - 1], CANARY);)               
    } else {
        LOG(fprintf(stderr, "*[%zu] = %lf (CANARY)\n",
            stack.capacity - 1, stack.data[stack.capacity - 1]);)
    }

    if (isatty(fileno(stderr))) {
        LOG(fprintf(stderr, "%s", RESET);)
    }

    LOG(fprintf(stderr, "}\n");)

    LOG(fprintf(stderr, "(StackDump): Ending...\n");)

    ON_DEBUG(StackVerify(&stack));

    return OK;
}

unsigned long long GetHash(Stack_t* ptrStack) {

    StackVerifyWithoutHash(ptrStack);

    unsigned long long hashMod = 1e9 + 7, hashPow = 239;
    unsigned long long hashValue = 0;
    hashValue += ptrStack->capacity;
    hashValue *= hashPow;
    hashValue %= hashMod;

    hashValue += ptrStack->size;
    hashValue *= hashPow;
    hashValue %= hashMod;

    for (size_t i = 0; i < ptrStack->capacity; i++) {
        hashValue += (unsigned long long)ptrStack->data[i];
        hashValue *= hashPow;
        hashValue %= hashMod;
    }

    char* statusString = (char*)ptrStack->status;
    for (; *statusString != '\0'; statusString++) {
        hashValue += (unsigned long long)*statusString;
        hashValue *= hashPow;
        hashValue %= hashMod;
    }

    StackVerifyWithoutHash(ptrStack);

    return hashValue;

}
