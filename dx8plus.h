#ifndef __DX8PLUS_H__
#define __DX8PLUS_H__

#include "loadHookTool.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DLL_EXPORT int __stdcall dot3MeshVBRender (int,int,int,DWORD**);
    DLL_EXPORT void __stdcall MVBcreateShader ();
    DLL_EXPORT int* __stdcall compilePixelShader (const int*);
    DLL_EXPORT int64_t __stdcall createShader (UINT*,UINT*);
    DLL_EXPORT UINT __stdcall setPixelShader (int);
    DLL_EXPORT int __stdcall drawLight (int,int,int,DWORD**);
    DLL_EXPORT void disablePixelShaderInAlpha ();

#ifdef __cplusplus
}
#endif

#endif // __DX8PLUS_H__
