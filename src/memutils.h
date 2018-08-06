#ifndef MEMUTILS_H
#define MEMUTILS_H

/* memory utils **********************************************************/

#define CREATE(result, type, number)  do {\
        if ((number) * sizeof(type) <= 0)       \
            bug("SYSERR: Zero bytes or less requested at %s:%d.", __FILE__, __LINE__);      \
        if (!((result) = (type *) calloc ((number), sizeof(type))))     \
        { syserr("SYSERR: malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
        if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
        { syserr("SYSERR: realloc failure"); abort(); } } while(0)

char *str_dup(const char *source);
void str_free(char *&str);
void str_reassign(char *&target, const char *newval);
void str_reassign_proto(char *&instance, const char *proto, const char *newval);
void str_reassign_proto(char *&instance, char *proto);

#define OBJ_STR_REASSIGN(obj, field, newval) \
    str_reassign_proto((obj)->field, \
                       GET_OBJ_RNUM(obj) == -1 ? NULL \
                                             : obj_proto[GET_OBJ_RNUM(obj)].field, \
                       newval)

#define MOB_STR_REASSIGN(mob, field, newval) \
    str_reassign_proto((mob)->field, \
                       GET_MOB_RNUM(mob) == -1 ? NULL \
                                             : mob_proto[GET_MOB_RNUM(mob)].field, \
                       newval)

#define FREEPTR(x) \
    do { \
        if (x) ::free(x); \
        x = 0; \
    } while (0)


#define DESTROY_LIST(head, next, temp) \
        while (head) { \
            temp = head->next; \
            ::free(head); \
            head = temp; \
        }

/*
 * the source previously used the same code in many places to remove an item
 * from a list: if it's the list head, change the head, else traverse the
 * list looking for the item before the one to be removed.  Now, we have a
 * macro to do this.  To use, just make sure that there is a variable 'temp'
 * declared as the same type as the list to be manipulated.  BTW, this is
 * a great application for C++ templates but, alas, this is not C++.  Maybe
 * CircleMUD 4.0 will be...
 */
#define REMOVE_FROM_LIST(item, head, next)      \
    if ((item) == (head)) {              \
        head = (item)->next;              \
    } else {                             \
        temp = head;                      \
        while (temp && (temp->next != (item))) \
            temp = temp->next;             \
        if (temp)                         \
            temp->next = (item)->next;     \
    }                                    \
     
#endif
