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

int main() {
    Tree tree = {};

    LoadTree(&tree);

    TreeVizDump(&tree); 

    SaveTree(&tree);

    TreeDestroy(&tree);

    
    return 0;
}
