#include "hedgerender/hr_state_group.h"

const HrStateGroup* hrStateGroupGetFirstWithState(
    const HrStateGroup** stateGroups,
    size_t stateGroupCount, HrStateType type)
{
    /* Return pointer to first State Group with the given state type. */
    size_t i;
    for (i = 0; i < stateGroupCount; ++i)
    {
        if (stateGroups[i]->statesMask & type)
            return stateGroups[i];
    }

    return NULL;
}

const HrStateGroup* hrStateGroupGetFirstWithResource(
    const HrStateGroup** stateGroups,
    size_t stateGroupCount, HrStateResourceType type)
{
    /* Return pointer to first State Group with the given resource type. */
    size_t i;
    for (i = 0; i < stateGroupCount; ++i)
    {
        if (stateGroups[i]->resourcesMask & type)
            return stateGroups[i];
    }

    return NULL;
}
