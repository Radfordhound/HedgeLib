#include "hedgelib/hl_radix_tree.h"
#include <string.h>

HlResult hlRadixNodeConstruct(const char* HL_RESTRICT name,
    HlUMax customData, HlRadixNode* HL_RESTRICT node)
{
    /* Setup name and nameLen. */
    if (name)
    {
        const size_t nameLen = strlen(name);

        /* Allocate buffer for name. */
        node->name = HL_ALLOC_ARR(char, nameLen + 1);
        if (!node->name) return HL_ERROR_OUT_OF_MEMORY;

        /* Copy data into name buffer. */
        memcpy(node->name, name, nameLen + 1);

        /* Store name length. */
        node->nameLen = nameLen;
    }
    else
    {
        node->name = NULL;
        node->nameLen = 0;
    }

    /* Construct the rest of the HlRadixNode and return success. */
    node->customData = customData;
    HL_LIST_INIT(node->children);
    
    return HL_RESULT_SUCCESS;
}

static const HlRadixNode* hlINRadixNodeGetClosestMatch(const HlRadixNode* HL_RESTRICT node,
    const char* HL_RESTRICT name, size_t* HL_RESTRICT nodeMatchLen,
    size_t* HL_RESTRICT totalMatchLen)
{
    size_t i;
    for (i = 0; i < node->children.count; ++i)
    {
        const HlRadixNode* childNode = &node->children.data[i];
        size_t matchLen = 0;

        /* Skip this child node if it has no name. */
        if (!childNode->name) continue;

        /*
           Determine the amount of characters in the child node's name
           that match the characters in the given name.
        */
        while (name[matchLen] == childNode->name[matchLen] && name[matchLen])
        {
            ++matchLen;
        }

        /* If the names at least partially match... */
        if (matchLen)
        {
            /* Increase the total match length. */
            *totalMatchLen += matchLen;

            /* Check if the given name was contained *entirely* within the child node's name. */
            if (matchLen == childNode->nameLen)
            {
                /* Recursively check the child node's children for a better match. */
                const HlRadixNode* betterMatch;
                size_t betterMatchLen;

                betterMatch = hlINRadixNodeGetClosestMatch(
                    childNode, &name[matchLen], &betterMatchLen,
                    totalMatchLen);

                /* If the recursive search returned a better match, return it. */
                if (betterMatch)
                {
                    *nodeMatchLen = betterMatchLen;
                    return betterMatch;
                }
            }

            /* The match we originally found is the best match; return it. */
            *nodeMatchLen = matchLen;
            return childNode;
        }
    }

    /* No matches were found at all. */
    *nodeMatchLen = 0;
    return NULL;
}

const HlRadixNode* hlRadixNodeGetClosestMatch(const HlRadixNode* HL_RESTRICT node,
    const char* HL_RESTRICT name, size_t* HL_RESTRICT nodeMatchLen,
    size_t* HL_RESTRICT totalMatchLen)
{
    /* Ensure total match length is set to 0 so we can safely add to it. */
    *totalMatchLen = 0;

    /* Recursively find the closest matching radix node. */
    return hlINRadixNodeGetClosestMatch(node, name, nodeMatchLen, totalMatchLen);
}

size_t hlRadixNodeGetTotalChildCount(const HlRadixNode* node)
{
    size_t totalCount = node->children.count, i;
    for (i = 0; i < node->children.count; ++i)
    {
        totalCount += hlRadixNodeGetTotalChildCount(
            &node->children.data[i]);
    }

    return totalCount;
}

void hlRadixNodeDestruct(HlRadixNode* node)
{
    /* Return early if pointer is NULL. */
    size_t i;
    if (!node) return;

    /* Free name. */
    hlFree(node->name);

    /* Destruct/Free children. */
    for (i = 0; i < node->children.count; ++i)
    {
        hlRadixNodeDestruct(&node->children.data[i]);
    }

    HL_LIST_FREE(node->children);
}

HlResult hlRadixTreeConstruct(HlUMax rootNodeCustomData, HlRadixTree* tree)
{
    HlResult result = hlRadixNodeConstruct(NULL, rootNodeCustomData, &tree->rootNode);
    tree->totalNodeCount = 1;
    return result;
}

HlResult hlRadixTreeInsert(HlRadixTree* HL_RESTRICT tree,
    const char* HL_RESTRICT name, HlUMax customData,
    const HlRadixNode* HL_RESTRICT * HL_RESTRICT newNodePtr)
{
    HlRadixNode *parentNode, *node;
    size_t nodeMatchLen, totalMatchLen;
    HlResult result;

    /* Recursively search for radix node that best matches the given name. */
    node = (HlRadixNode*)hlRadixNodeGetClosestMatch(
        &tree->rootNode, name, &nodeMatchLen, &totalMatchLen);

    /* If any sort of match was found... */
    if (node)
    {
        /*
           Increase the name pointer such that it points to
           the first character that was not matched.
        */
        name += totalMatchLen;

        /*
           If the closest matching node's name had a character that
           didn't match, split the closest matching node up into two
           parts.
        */
        if (node->nameLen != nodeMatchLen)
        {
            HlRadixNode suffixNode;

            /*
               Create a "suffix" node which contains *just* the part of
               the closest matching node's name that *didn't* match.
            */
            result = hlRadixNodeConstruct(&node->name[nodeMatchLen],
                node->customData, &suffixNode);

            if (HL_FAILED(result)) return result;

            /*
               Move the closest matching node's children to the suffix node.

               NOTE: This is safe as the data will, from now on, simply be
               freed as a part of the suffix node rather than the closest
               matching node.
            */
            suffixNode.children = node->children;
            HL_LIST_INIT(node->children);

            /* Insert the suffix node as a child of the closest matching node. */
            result = HL_LIST_PUSH(node->children, suffixNode);
            if (HL_FAILED(result))
            {
                hlRadixNodeDestruct(&suffixNode);
                return result;
            }

            /* Cut off the part of the closest matching node's name that didn't match. */
            node->name[nodeMatchLen] = '\0';
            node->nameLen = nodeMatchLen;

            /*
               "Reset" the closest matching node's customData as we
               essentially just transferred it into the suffix node.
            */
            node->customData = 0;

            /* Increase total node count. */
            ++tree->totalNodeCount;
        }

        /* The new node is to be inserted as a child of the closest matching node. */
        parentNode = node;
    }
    
    /* Otherwise, the new node is to be inserted as a child of the root node. */
    else
    {
        parentNode = &tree->rootNode;
    }

    /* Create the new node and insert it into the tree. */
    {
        HlRadixNode newNode;
        result = hlRadixNodeConstruct(name, customData, &newNode);
        if (HL_FAILED(result)) return result;

        result = HL_LIST_PUSH(parentNode->children, newNode);
        if (HL_FAILED(result))
        {
            hlRadixNodeDestruct(&newNode);
            return result;
        }
    }

    /* Increase total node count. */
    ++tree->totalNodeCount;

    /* Set the new node pointer if necessary and return result. */
    if (newNodePtr)
    {
        *newNodePtr = &HL_LIST_GET_LAST(parentNode->children);
    }

    return result;
}

void hlRadixTreeReset(HlRadixTree* tree)
{
    HL_LIST_CLEAR(tree->rootNode.children);
    tree->totalNodeCount = 1;
}

void hlRadixTreeDestruct(HlRadixTree* tree)
{
    hlRadixNodeDestruct(&tree->rootNode);
}
