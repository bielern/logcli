#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include "utils.h"

/**
 * Get memory space and check, wheter I have it
 */
void *
xmalloc(size_t size){
    void * pointer;
    pointer = malloc(size);
    if(pointer == NULL){
        error(1, 0, "Could not allocate enough space!");
    }
    return pointer;
}

/**
 * reallocate space and check it
 */
void *
xrealloc(void * pointer, size_t size){
    pointer = realloc(pointer, size);
    if(pointer == NULL){
        error(1, 0, "Could not reallocate space!");
    }
    return pointer;
}

/**
 * safe version of strcat
 */
void
cat(char **to, char * from){
    int length = strlen(*to) + strlen(from) + 1;
    *to = (char *) xrealloc(*to, length * sizeof(char));
    if(strcat(*to, from) != *to){
        error(1, 0, "Could not concate the string %s\n", from);
    }
}
/* EOF */
