#ifndef HR_COLOR_H_INCLUDED
#define HR_COLOR_H_INCLUDED
#include "hr_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HrColor
{
    float r;
    float g;
    float b;
    float a;
}
HrColor;

HR_API extern HrColor HR_COLOR_NULL;
HR_API extern HrColor HR_COLOR_BLACK;
HR_API extern HrColor HR_COLOR_WHITE;
HR_API extern HrColor HR_COLOR_RED;
HR_API extern HrColor HR_COLOR_GREEN;
HR_API extern HrColor HR_COLOR_BLUE;

/* TODO: Add more color constants. */

#ifdef __cplusplus
}
#endif
#endif
