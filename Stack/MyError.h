#ifndef MY_ERROR_H
#define MY_ERROR_H

enum ERROR_STATUSES {
    OK                  = 0, 
    FILE_ACCESS_DENIED  = 1,
    FILE_FSTAT_ERROR    = 2,
    OUT_OF_RANGE        = 3,
    CMP_ERROR           = 4, 
    NOT_ENOUGH_MEMORY   = 5, 
    PUSH_NOT_EQUAL_POP  = 6, 
    BAD_CLEAR_INIT      = 7, 
    STACK_DESTROY_ERROR = 8,
    EMPTY_STACK         = 9,
    STACK_CAP_SIZE      = 10, 
    BAD_CANARY          = 11
};

const char* GetErrorString(ERROR_STATUSES errorStatus);

const char* GetErrorString(ERROR_STATUSES errorStatus) {
    switch (errorStatus) {
        case NOT_ENOUGH_MEMORY:
            return "Not enough memory!\n";
        
        case FILE_ACCESS_DENIED:
            return "Cannot access file!\n";
            
        case FILE_FSTAT_ERROR:
            return "Unknown fstat() error!\n";
        
        case OUT_OF_RANGE:
            return "Index is out of range!\n";

        case CMP_ERROR:
            return "Comarator error!\n";

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

        default:
            return "Unknown error status!\n";
    }
}

#endif