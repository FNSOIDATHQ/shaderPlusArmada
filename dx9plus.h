#ifndef __DX9PLUS_H__
#define __DX9PLUS_H__

#include <windows.h>
#include <stdint.h>

 //fix bug in compile
#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma comment(linker, "/NODEFAULTLIB:libci.lib")

#ifdef BUILD_DLL
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

#define DLL_IMPORT __declspec(dllimport)

#ifdef __cplusplus
extern "C"
{
#endif

    DLL_EXPORT int dot3MeshVBRender9 (int,int,int,DWORD**);

#ifdef __cplusplus
}
#endif

#endif // __DX9PLUS_H__
