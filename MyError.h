#ifndef MY_ERROR_H
#define MY_ERROR_H

enum ERROR_STATUSES {
    NOT_ENOUGH_MEMORY = 0, 
    FILE_ACCESS_DENIED = 1,
    FILE_FSTAT_ERROR = 2,
    OUT_OF_RANGE = 3,
    CMP_ERROR = 4
};

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

        default:
            return "Unknown error status!\n";
    }
}

#endif