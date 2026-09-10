#ifndef ASSERT_H
#define ASSERT_H

#include <stdlib.h>

#define NOT !

#define INDEX_OK(index, upperBound) (0 <= index && index < upperBound)

#ifdef DEBUG

// #define FMT(x) (#x ":" x)

#define ASSERT(condition, sentence, exitStatus)\
    do {\
        if (NOT (condition)) {\
            printf("%s %s:%i %s():\n", sentence, __FILE__, __LINE__, __FUNCTION__);\
            exit(exitStatus);\
        }\
    } while (0)\

#else

// #define FMT(x)

#define ASSERT(condition, sentence, exitStatus)

#endif

#endif