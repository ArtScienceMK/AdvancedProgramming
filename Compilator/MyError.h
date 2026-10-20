#ifndef _MY_ERROR_H
#define _MY_ERROR_H

enum ERROR_STATUSES {
    OK                                   = 0, 
    FILE_ACCESS_DENIED                   = 1,
    FILE_FSTAT_ERROR                     = 2,
    OUT_OF_RANGE                         = 3,
    CMP_ERROR                            = 4, 
    NOT_ENOUGH_MEMORY                    = 5, 
    PUSH_NOT_EQUAL_POP                   = 6, 
    BAD_CLEAR_INIT                       = 7, 
    STACK_DESTROY_ERROR                  = 8,
    EMPTY_STACK                          = 9,
    STACK_CAP_SIZE                       = 10, 
    BAD_CANARY                           = 11,
    HASHES_DIFFER                        = 12,
    EXECUTION_ERROR                      = 13,
    READ_FILE_ERROR                      = 14,
    WRITE_BUFFER_TO_FILE_ERROR           = 15,
    UNLINK_FILE_ERROR                    = 16,
    WRITE_SEP_TO_FILE_ERROR              = 17,
    UNKNOWN_OPCODE                       = 18,
    ZERO_DIVISION_ERROR                  = 19,
    OPCODE_INPUT_ERROR                   = 20,
    ARGS_INPUT_ERROR                     = 21,
    ASSEMBLE_ERROR                       = 22,
    STRCMP_ERROR                         = 23,
    ASSEMBLE_VERIFICATION_ERROR          = 24,
    INT_TO_STRING_ERROR                  = 25,
    DISASSEMBLE_ERROR                    = 26,
    READING_OPCODE_DISASSEMBLE_ERROR     = 27,
    WRITE_INDEX_TO_FILE_ERROR            = 28,
    READING_COMMAND_ASSEMBLE_ERROR       = 29,
    UNKNOWN_REGISTER_NAME                = 30,
    GET_ARGUMENT_TYPE_ERROR              = 31,
    IS_VALID_REGISTER_NAME_ERROR         = 32,
    NO_ARGUMENT_TYPE_ERROR               = 33,
    UNKNOWN_ARGUMENT_TYPE                = 34,
    JUMP_ADDRESS_IS_OUT_OF_RANGE         = 35,
    NOT_INITIALIZED_JUMP_ADDRESS         = 36,
    HALT_EXIT                            = 37,
    IS_VALID_REGISTER_NAME_STRING_ERROR  = 38,
    READ_LABEL_ERROR                     = 39,
    LABEL_REDEFINITION                   = 40,
    UNKNOWN_LABEL                        = 41,
    NOT_INITIALIZED_LABEL                = 42
};

const char* GetErrorString(ERROR_STATUSES errorStatus);

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

        default:
            return "Unknown error status!\n";
    }
}

#endif /*_MY_ERROR_H*/