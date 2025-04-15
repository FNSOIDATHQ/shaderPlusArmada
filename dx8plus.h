#ifndef __DX8PLUS_H__
#define __DX8PLUS_H__

#include "loadHookTool.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DLL_EXPORT int dot3MeshVBRender (int,int,int,DWORD**);
    DLL_EXPORT void MVBcreateShader ();
    DLL_EXPORT int* compilePixelShader (const int*);
    DLL_EXPORT int64_t createShader (UINT*,UINT*);
    DLL_EXPORT UINT setPixelShader (int);
    DLL_EXPORT int drawLight (int,int,int,DWORD**);
    DLL_EXPORT void disablePixelShaderInAlpha ();

#ifdef __cplusplus
}
#endif

#endif // __DX8PLUS_H__
