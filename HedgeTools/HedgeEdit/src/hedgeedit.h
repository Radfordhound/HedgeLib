#ifndef HEDGEEDIT_H_INCLUDED
#define HEDGEEDIT_H_INCLUDED
#include "hedgelib/hl_internal.h"

typedef struct HrInstance HrInstance;
typedef struct HrWindow HrWindow;

extern HrInstance* Instance;
extern HrWindow Window;

HlResult hedgeEditRun(void);
#endif
