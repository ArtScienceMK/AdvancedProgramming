#ifndef MY_ERROR_H
#define MY_ERROR_H

enum ERROR_STATUSES {
    NOT_ENOUGH_MEMORY = 0
};

const char* GetErrorString(ERROR_STATUSES errorStatus) {
    switch (errorStatus) {
        case NOT_ENOUGH_MEMORY:
            return "Not enough memory!\n";
        
        default:
            return "Unknown error status!\n";
    }
}

#endif