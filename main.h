#ifndef __MAIN_H__
#define __MAIN_H__

//separate code to avoid dx8/9 library conflict
#include "dx8plus.h"
#include "dx9plus.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DLL_EXPORT bool activate ();

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
