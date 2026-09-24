#ifndef ASSERT_H
#define ASSERT_H

#include <stdlib.h>

#define NOT !

#define INDEX_OK(index, upperBound) (0 <= index && index < upperBound)

#define BLUE   "\033[94m"
#define PINK   "\033[95m"

#define GREEN  "\033[92m"

#define ORANGE "\033[93m"
#define RED    "\033[91m"

#define RESET "\033[0m"

#ifdef DEBUG

#define LOG(...) __VA_ARGS__

// #define FMT(x) (#x ":" x)

#define ASSERT(condition, sentence, exitStatus)\
    do {\
        if (NOT (condition)) {\
            printf("%s %s:%i %s():\n", sentence, __FILE__, __LINE__, __FUNCTION__);\
            exit(exitStatus);\
        }\
    } while (0)

#define DEBUG_ARRAY(arr, leftBound, leftIndex, midIndex, rightIndex, rightBound, midValue, arrSize)\
    printf("leftBound: %i, leftIndex: %i, midIndex: %i, rightIndex: %i, rightBound: %i",\
            leftBound, leftIndex, midIndex, rightIndex, rightBound);\
    \
    printf(", %smidValue: %i\n%s", GREEN, midValue, RESET);\
    \
    do {\
        for (int i = 0; i < leftBound; i++) {\
            printf("%i ", arr[i]);\
        }\
        printf(BLUE);\
        for (int i = leftBound; i < leftIndex; i++) {\
            printf("%i ", arr[i]);\
        }\
        \
        printf("%s%i ", PINK, arr[leftIndex]);\
        \
        printf(RESET);\
        \
        for (int i = leftIndex + 1; i < rightIndex; i++) {\
            printf("%i ", arr[i]);\
        }\
        \
        if (rightIndex >= leftIndex + 1) {\
            printf("%s%i ", ORANGE, arr[rightIndex]);\
        }\
        \
        printf(RED);\
        \
        for (int i = rightIndex + 1; i <= rightBound; i++) {\
            printf("%i ", arr[i]);\
        }\
        \
        printf(RESET);\
        \
        for (int i = rightBound + 1; i < arrSize; i++) {\
            printf("%i ", arr[i]);\
        }\
        printf("\n");\
        \
    } while (0)
    
#else

#define LOG(...)

#define DEBUG_ARRAY(arr, leftBound, leftIndex, midIndex, rightIndex, rightBound, midValue, arrSize)

// #define FMT(x)

#define ASSERT(condition, sentence, exitStatus)

#endif

#endif