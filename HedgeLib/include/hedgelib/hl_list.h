#ifndef HL_LIST_H_INCLUDED
#define HL_LIST_H_INCLUDED
#include "hl_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_LIST(type) struct { type* data; size_t count; size_t capacity; }
#define HL_LIST_INIT(list) { (list).data = 0; (list).count = 0; (list).capacity = 0; }

#define HL_INLIST_GROW(list)\
    /* Double the list's capacity. */\
    ((list).capacity = ((list).capacity) ? (list).capacity * 2 : 1,\
\
    /* Reallocate the list's data buffer. */\
    (HlBool)((*(void**)(&(list).data) = hlRealloc((list).data,\
        sizeof(*((list).data)) * (list).capacity)) != 0))

    /*
        The list needs to grow if the list's count + the
        given grow amount exceeds the list's capacity.
    */
#define HL_INLIST_NEEDS_GROW(list, growAmount)\
    (HlBool)(((list).count + (growAmount)) > (list).capacity)

#define HL_INLIST_MAYBE_GROW(list, reqCount)\
    (HL_INLIST_NEEDS_GROW(list, reqCount) ?     /* If the list needs to grow... */\
    HL_INLIST_GROW(list) :                      /* Enlargen it and return whether that succeeds. */\
    (HlBool)1)                                  /* Otherwise, just return 1; we "succeeded". */

#define HL_LIST_PUSH(list, obj)\
    (HL_INLIST_MAYBE_GROW(list, 1) ?            /* Enlarge the list's data buffer if necessary. */\
    (list).data[(list).count++] = (obj),        /* Increment list count and copy object into list. */\
    HL_RESULT_SUCCESS : HL_ERROR_OUT_OF_MEMORY) /* Return HlResult indicating whether push succeeded. */

#define HL_LIST_POP(list) --(list).count
#define HL_LIST_GET_LAST(list) ((list).data[(list).count - 1])
#define HL_LIST_FREE(list) hlFree((list).data)

/* Offset table typedef. */
typedef HL_LIST(size_t) HlOffTable;

#ifdef __cplusplus
}
#endif
#endif
