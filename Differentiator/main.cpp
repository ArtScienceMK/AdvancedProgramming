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

    TreeDestroy(&tree);

    
    return 0;
}

const char* GetErrorString(ERROR_STATUSES errorStatus) {
    switch (errorStatus) {
        case OK:
            return "Program works fine! But this sentence shouldn't be printed to user!\n";
        
        case FILE_ACCESS_DENIED:
            return "Cannot access file!\n";
                
        case FILE_FSTAT_ERROR:
            return "Unknown fstat() error!\n";
                    
        case OUT_OF_RANGE:
            return "Index is out of range!\n";

        case CMP_ERROR:
            return "Comarator error!\n";
        
        case NOT_ENOUGH_MEMORY:
            return "Not enough memory!\n";

        case PUSH_NOT_EQUAL_POP:
            return "Last pushed element to stack is not equal first popped!\n";

        case BAD_CLEAR_INIT:
            return "Value was initialized with zeroes, but it's not true BEFORE initialization function!\n";

        case STACK_DESTROY_ERROR:
            return "Error happened when destroying stack!\n";
        
        case EMPTY_STACK:
            return "Trying to pop from the empty stack!\n";

        case STACK_CAP_SIZE:
            return "Stack capacity is less than size!\n";

        case BAD_CANARY:
            return "Canary value was changed!\n";

        case HASHES_DIFFER:
            return "Hashes values differ: hash inside structure not equal to just counted hash\n";

        case EXECUTION_ERROR:
            return "Error happened when executing!\n";

        case READ_FILE_ERROR:
            return "Error happened when reading file!\n";

        case UNLINK_FILE_ERROR:
            return "Error happened when unlinking file!\n";

        case UNKNOWN_OPCODE:
            return "Unknown opcode found when executing!\n";

        case ZERO_DIVISION_ERROR:
            return "Zero division error happened!\n"; 
        
        case OPCODE_INPUT_ERROR:
            return "Error happened while executing in opcode input!\n";
        
        case ARGS_INPUT_ERROR:
            return "Error happened while executing in args input!\n";   
        
        case ASSEMBLE_ERROR:
            return "Error happened while assembling!\n";

        case STRCMP_ERROR:
            return "Error happened while comparing strings!\n";
        
        case ASSEMBLE_VERIFICATION_ERROR:
            return "Incorrect line found while assembling!\n";
        
        case INT_TO_STRING_ERROR:
            return "Error happened while converting int to string!\n";
        
        case DISASSEMBLE_ERROR:
            return "Error happened while disassembling!\n";

        case READING_OPCODE_DISASSEMBLE_ERROR:
            return "Error happened when reading opcode while disassembling!\n";
        
        case WRITE_INDEX_TO_FILE_ERROR:
            return "Error happened when writing index to file!\n";

        case READING_COMMAND_ASSEMBLE_ERROR:
            return "Error happened when reading command while assembling!\n";

        case UNKNOWN_REGISTER_NAME:
            return "Got an unknown register name!\n";

        case GET_ARGUMENT_TYPE_ERROR:
            return "Error happened when getting argument type!\n";

        case IS_VALID_REGISTER_NAME_ERROR:
            return "Error happened when validating register name!\n";

        case NO_ARGUMENT_TYPE_ERROR:
            return "Argument type missing!\n";

        case UNKNOWN_ARGUMENT_TYPE:
            return "Unknown argument type!\n";

        case JUMP_ADDRESS_IS_OUT_OF_RANGE:
            return "Jump address is out of allocated memory range!\n";
        
        case NOT_INITIALIZED_JUMP_ADDRESS:
            return "Jump address was not initialized!\n";

        case HALT_EXIT:
            return "HALT command was called!\n";

        case IS_VALID_REGISTER_NAME_STRING_ERROR:
            return "Error happened when validating register name string!\n";
        
        case READ_LABEL_ERROR:
            return "Error happened when reading label!\n";

        case LABEL_REDEFINITION:
            return "Got a label redefinition!\n";

        case UNKNOWN_LABEL:
            return "Got an unknown label!\n";
        
        case NOT_INITIALIZED_LABEL:
            return "Label was not initialized!\n";

        case READING_MEMORY_INDEX_ERROR:
            return "When reading memory index error happened!\n";

        case BAD_MEMORY_ADDRESS:
            return "Memory address is out of allocated (for this vm) RAM!\n";
        
        case BAD_LIST_SIZE:
            return "List has size <= 0\n";

        case LIST_CAP_SIZE:
            return "List capacity less than size!\n";

        case BAD_LIST_HEAD:
            return "Incorrect head index in the list!\n";

        case BAD_LIST_TAIL:
            return "Incorrect tail index in the list!\n";
        
        case BAD_LIST_ZERO_ELEM:
            return "Incorrect zero element in list! Should have all zeros!\n";

        case LIST_CYCLE:
            return "List has cycles!\n";

        case SYSTEM_CALL_ERROR:
            return "Error while executing system()!\n";

        case NO_EDGE_ERROR:
            return "List node has less than two edges!\n";

        case UNKNOWN_NODE_STATUS:
            return "List node has unknown status!\n";

        case NOT_USED_NODE:
            return "List node is not used!\n";

        case NODE_INSERT_INT_ERROR:
            return "Error happened when inserting integer value in the node!\n";

        case TREE_BAD_SIZE:
            return "Tree has not positive size!\n";
        
        case UNKNOWN_AKINATOR_MENU_OPTION:
            return "Unknown menu option entered in akinator program!\n";

        case AKINATOR_MENU_INPUT_ERROR:
            return "Error happened when proceeding user input in akinator menu!\n";

        case AKINATOR_QUESTION_INPUT_ERROR:
            return "Error happened when proceeding user input while answering question!\n";

        case UNKNOWN_AKINATOR_ANSWER_OPTION:
            return "Incorrect akinator answer option!\n";

        case AKINATOR_OBJECT_INPUT_ERROR:
            return "Error when proceeding object input!\n";

        case UNKNOWN_STRING_LOAD_TREE:
            return "Got an unknown string when loading tree!\n";

        case AKINATOR_COMPARE_ERROR:
            return "An error happened when comparing in akinator!\n";

        case LOAD_TREE_UNKNOWN_LINE_ERROR:
            return "Got an unknown line when loading a tree from file!\n";
        
        case STRING_TO_DOUBLE_ERROR:
            return "An error happened when converting string to double!\n";

        default:
            return "Unknown error status!\n";
    }
}
