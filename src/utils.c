#include <string.h>
#include "utils.h"

char **explode(char *str, char *separator) {
    char **res = NULL;

    if (str == NULL) { 
        return res;
    }
    
    int cap = 8;
    int count = 0;

    res = (char **) malloc(cap*sizeof (char *));
    char *token = strtok(str, separator);
    while(1) {
        if(count >= cap) {
            cap *= 2;
            res = (char **)realloc(res, cap*sizeof(char *));
        }
        if(token == NULL) {
            res[count] = NULL;
            break;
        }
        res[count] = strdup(token);
        count += 1;
        token = strtok(NULL, separator);
    }
    return res;
}

