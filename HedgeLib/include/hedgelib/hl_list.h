#ifndef HL_LIST_H_INCLUDED
#define HL_LIST_H_INCLUDED
#include "hl_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
   @defgroup lists Lists
   @brief Code related to HL_LISTS (essentially C89 std::vectors).
*/

/**
   @brief Defines an HL_LIST of the given type.
   Think of this as typing std::vector<type>.

   This is intended to be used in the following situations:

   1) Typedefs:

   typedef HL_LIST(int) IntList;

   2) Variable declarations:

   struct Whatever
   {
       HL_LIST(int) ints;
       float anotherVariable;
   };

   If you need to use HL_LISTs in other situations (e.g. as a function
   argument), please use a typedef. Example:

   typedef HL_LIST(int) IntList;
   void myFunction(IntList* list);

   @param[in] type  The type the list should contain (E.G. int).

   @return An HL_LIST struct of the given type.
   @ingroup lists
*/
#define HL_LIST(type) struct { type* data; size_t count; size_t capacity; }

/**
   @brief "Initializes" the given list. You must call this on *your* HL_LISTs
   before using them. This *DOES NOT* apply to lists that are given to you
   from HedgeLib functions; these will initialize the lists for you.

   Currently, this simply sets every value in the list to 0.
   It would be safe to, for example, do this instead:

   HL_LIST(int) lists = { 0 };

   However, this approach is not recommended as it may not continue to
   work in the future if lists are changed.

   @param[in] list  The HL_LIST to initialize.

   @ingroup lists
*/
#define HL_LIST_INIT(list) (((list).data = NULL), ((list).count = 0), ((list).capacity = 0))

/* Use capacity == 0 as a flag which means the list does not own its data pointer. */
#define HL_IN_LIST_OWNS_DATA_PTR(list) ((list).capacity != 0)

#ifdef __cplusplus
}

/*
   In C++, it's not valid to store a void* in an
   int*, for example, without casting. We could do
   this: *((void**)&list.data) = ptr; but it's
   technically not standard to assume that
   sizeof(void*) == sizeof(int*).

   While this obviously is very unlikely to be an
   issue in practice, it's still better to be compliant
   if possible. So, seeing as this is C++, we just use
   a template.
*/
template<typename T>
inline void hlINListSetDataCPP(T*& dataPtr, void* ptr)
{
    dataPtr = (T*)ptr;
}

#define HL_IN_LIST_SET_DATA(list, ptr) hlINListSetDataCPP((list).data, ptr)

extern "C" {
#else
/*
   In C, void* can auto-cast to int*, for example, so we just do that.
*/
#define HL_IN_LIST_SET_DATA(list, ptr) (list).data = ptr
#endif

HL_API void* hlINListReserve(void* HL_RESTRICT data,
    size_t size, size_t count, size_t desiredCapacity,
    size_t* HL_RESTRICT capacity);

#define HL_IN_LIST_RESERVE(list, c)\
    /* Resize the data buffer and set the new pointer. */\
    (HL_IN_LIST_SET_DATA(list, hlINListReserve((void*)(list).data,\
    sizeof(*((list).data)), (list).count, c, &(list).capacity)),\
\
    /* "Return" whether the reallocation of the buffer succeeded. */\
    (HlBool)((list).data != NULL))

/**
   @brief Pre-allocates enough space to store the given amount of values within the given list.
   @param[in] list  The HL_LIST to reserve space in.
   @param[in] c     The amount of values to reserve space for.

   @return An HlResult indicating whether pre-allocation succeeded or not.
   @ingroup lists
*/
#define HL_LIST_RESERVE(list, c)\
    (HL_IN_LIST_RESERVE(list, c) ?              /* Resize the list's data buffer as requested. */\
    HL_RESULT_SUCCESS : HL_ERROR_OUT_OF_MEMORY) /* Return HlResult indicating whether resize succeeded. */

HL_API void* hlINListGrow(void* HL_RESTRICT data,
    size_t size, size_t count, size_t* HL_RESTRICT capacity);

#define HL_IN_LIST_GROW(list)\
    /* Grow the data buffer and set the new pointer. */\
    (HL_IN_LIST_SET_DATA(list, hlINListGrow((void*)(list).data,\
    sizeof(*((list).data)), (list).count, &(list).capacity)),\
\
    /* "Return" whether the reallocation of the buffer succeeded. */\
    (HlBool)((list).data != NULL))

/*
    The list needs to grow if the list's count + 1
    exceeds the list's capacity.
*/
#define HL_IN_LIST_NEEDS_GROW(list)\
    (HlBool)(((list).count + 1) > (list).capacity)

#define HL_IN_LIST_MAYBE_GROW(list)\
    (HL_IN_LIST_NEEDS_GROW(list) ?              /* If the list needs to grow... */\
    HL_IN_LIST_GROW(list) :                     /* Enlargen it and return whether that succeeds. */\
    HL_TRUE)                                    /* Otherwise, just return true; we "succeeded". */

/**
   @brief Adds the given value to the end of the given list.
   @param[in] list  The HL_LIST to add the given value to.
   @param[in] v     The value to add to the end of the given list.

   @return An HlResult indicating whether pushing succeeded or not.
   @ingroup lists
*/
#define HL_LIST_PUSH(list, v)\
    (HL_IN_LIST_MAYBE_GROW(list) ?              /* Enlarge the list's data buffer if necessary. */\
    (list).data[(list).count++] = (v),          /* Increment list count and copy object into list. */\
    HL_RESULT_SUCCESS : HL_ERROR_OUT_OF_MEMORY) /* Return HlResult indicating whether push succeeded. */

/**
   @brief Removes the last value from the given list.
   @param[in] list  The HL_LIST to remove the last value from.

   @ingroup lists
*/
#define HL_LIST_POP(list) (--(list).count)

HL_API void hlINListRemove(void* data, size_t size,
    size_t count, size_t index);

/**
   @brief Removes the value at the given index from the given list.
   @param[in] list  The HL_LIST to remove the given value from.
   @param[in] index The zero-based index of the value to remove from the given list.

   @ingroup lists
*/
#define HL_LIST_REMOVE(list, index)\
    (HL_LIST_POP(list), ((index) != (list).count) ?\
    hlINListRemove((list).data, sizeof(*((list).data)),\
        (list).count, (size_t)(index)), 1 : 1)

/**
   @brief Retrieves the last value from the given list.
   @param[in] list  The HL_LIST to get the last value from.

   @return The last value in the given list.
   @ingroup lists
*/
#define HL_LIST_GET_LAST(list) ((list).data[(list).count - 1])

/**
   @brief Removes all values from the given list.
   @param[in] list  The HL_LIST to clear.

   @ingroup lists
*/
#define HL_LIST_CLEAR(list) ((list).count = 0)

/**
   @brief Frees the data in the given list if the list owns its data pointer.
   @param[in] list  The HL_LIST to free.

   @return A boolean specifying whether hlFree was called (if the list owned its data pointer).
   @ingroup lists
*/
#define HL_LIST_FREE(list) ((HL_IN_LIST_OWNS_DATA_PTR(list)) ?\
    hlFree((void*)((list).data)), HL_TRUE : HL_FALSE)

/** @brief Typedef of HL_LIST(size_t) meant to be used for storing offset positions. */
typedef HL_LIST(size_t) HlOffTable;

typedef enum HlStrTableEntryFlags
{
    HL_STR_TABLE_ENTRY_FLAGS_NONE = 0,
    HL_STR_TABLE_ENTRY_FLAGS_DOES_OWN = 1
}
HlStrTableEntryFlags;

/** @brief String table entry. */
typedef struct HlStrTableEntry
{
    /** @brief See HlStrTableEntryFlags. */
    HlStrTableEntryFlags flags;
    /** @brief The string being referenced. */
    const char* str;
    /** @brief The (absolute) position of the string offset within the file. */
    size_t offPos;
    /** @brief The length of the string, not counting the null-terminator. */
    size_t len;
}
HlStrTableEntry;

/** @brief Typedef of HL_LIST(HlStrTableEntry) meant to be used for storing string entries. */
typedef HL_LIST(HlStrTableEntry) HlStrTable;

HL_API void hlOffTableSort(HlOffTable* offTable);

HL_API HlResult hlStrTableAddStrRefNativeEx(HlStrTable* HL_RESTRICT strTable,
    const HlNChar* HL_RESTRICT str, size_t offPos, size_t len);

HL_API HlResult hlStrTableAddStrRefNative(HlStrTable* HL_RESTRICT strTable,
    const HlNChar* HL_RESTRICT str, size_t offPos);

HL_API HlResult hlStrTableAddStrRefUTF8Ex(HlStrTable* HL_RESTRICT strTable,
    const char* HL_RESTRICT str, size_t offPos, size_t len);

HL_API HlResult hlStrTableAddStrRefUTF8(HlStrTable* HL_RESTRICT strTable,
    const char* HL_RESTRICT str, size_t offPos);

HL_API HlResult hlStrTableAddStrNativeEx(HlStrTable* HL_RESTRICT strTable,
    const HlNChar* HL_RESTRICT str, size_t offPos, size_t len);

HL_API HlResult hlStrTableAddStrNative(HlStrTable* HL_RESTRICT strTable,
    const HlNChar* HL_RESTRICT str, size_t offPos);

HL_API HlResult hlStrTableAddStrUTF8Ex(HlStrTable* HL_RESTRICT strTable,
    const char* HL_RESTRICT str, size_t offPos, size_t len);

HL_API HlResult hlStrTableAddStrUTF8(HlStrTable* HL_RESTRICT strTable,
    const char* HL_RESTRICT str, size_t offPos);

HL_API void hlStrTableDestruct(HlStrTable* strTable);

#ifdef __cplusplus
}
#endif
#endif
