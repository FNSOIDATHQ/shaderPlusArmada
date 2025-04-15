#ifndef __LOADHOOKTOOL_H__
#define __LOADHOOKTOOL_H__

#include "utilities.h"

//hook tool
extern void (*hookJMP)(void*,void*);
extern void* (*hookVTable)(void**,size_t,void*);
extern void (*writeVarToAddress)(UINT,UINT,void*);
extern void (*writeVarToAddressP)(void*,UINT,void*);
extern void (*writeNopsToAddress)(UINT,UINT);
extern void* (*getClassFunctionAddress)(DWORD*,int);
extern UINT (*getThisPtrFromECX)();
extern void (*moveVarToECX)(UINT);

#ifdef __cplusplus
extern "C"
{
#endif

    DLL_EXPORT bool loadTools ();

#ifdef __cplusplus
}
#endif

#endif // __LOADHOOKTOOL_H__
