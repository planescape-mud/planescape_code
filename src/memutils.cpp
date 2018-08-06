#include "sysdep.h"
#include "memutils.h"
#include "logstream.h"

/* 
 * Create a duplicate of a string 
 */
char *str_dup(const char *source) {
    char *new_z = NULL;
    if (source) {
        CREATE(new_z, char, strlen(source) + 1);
        return (strcpy(new_z, source));
    }
    CREATE(new_z, char, 1);
    return (strcpy(new_z, ""));
}

/* 
 * Frees memory allocated for the string, accepts NULL to simplify callers 
 */
void str_free(char *&str) {
    if (!str)
        return;

    free(str);
    str = 0;
}

/* 
 * Disposes old value and allocates new one, accepts NULLs 
 */
void str_reassign(char *&target, const char *newval) {
    str_free(target);
    target = str_dup(newval);
}

/* 
 * If instance and proto fields doesn't differ, just assign new value.
 * If they do, it means that instance field was already assigned,
 * so dispose old value and assign new one.
 */
void str_reassign_proto(char *&instance, const char *proto, const char *newval) {
    if (instance != proto)
        str_free(instance); 
    
    instance = str_dup(newval);
}

/*
 * Restores instance field back to proto value, freeing memory occupied
 * by instance, if applicable.
 */
void str_reassign_proto(char *&instance, char *proto) {
    if (instance != proto)
        str_free(instance); 
    
    instance = proto;
}

