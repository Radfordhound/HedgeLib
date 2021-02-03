#ifndef HL_RADIX_TREE_H_INCLUDED
#define HL_RADIX_TREE_H_INCLUDED
#include "hl_list.h"
#include <stdio.h>

typedef struct HlRadixNode
{
    char* name;
    size_t nameLen;
    HlUMax customData;
    HL_LIST(struct HlRadixNode) children;
}
HlRadixNode;

typedef struct HlRadixTree
{
    HlRadixNode rootNode;
    size_t totalNodeCount;
}
HlRadixTree;

HL_API HlResult hlRadixNodeConstruct(const char* HL_RESTRICT name,
    HlUMax customData, HlRadixNode* HL_RESTRICT node);

HL_API const HlRadixNode* hlRadixNodeGetClosestMatch(const HlRadixNode* HL_RESTRICT node,
    const char* HL_RESTRICT name, size_t* HL_RESTRICT nodeMatchLen,
    size_t* HL_RESTRICT totalMatchLen);

HL_API size_t hlRadixNodeGetTotalChildCount(const HlRadixNode* node);
HL_API void hlRadixNodeDestruct(HlRadixNode* node);

HL_API HlResult hlRadixTreeConstruct(HlUMax rootNodeCustomData, HlRadixTree* tree);
HL_API HlResult hlRadixTreeInsert(HlRadixTree* HL_RESTRICT tree,
    const char* HL_RESTRICT name, HlUMax customData,
    const HlRadixNode* HL_RESTRICT * HL_RESTRICT newNodePtr);

/* TODO: Create a function that allows you to sort the radix tree alphabetically. */

HL_API void hlRadixTreeReset(HlRadixTree* tree);
HL_API void hlRadixTreeDestruct(HlRadixTree* tree);

#endif
