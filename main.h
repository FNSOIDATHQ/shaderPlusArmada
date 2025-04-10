#ifndef __MAIN_H__
#define __MAIN_H__

//cpp std library
#include <string>

//DX library
//from dx9.0c sdk
#include <d3d8.h>
#include <d3dx8.h>

//hook library
//use this when coding, switch to dynamic load when compiling
//#include "..\hookTools\hookTool.h"

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

//for easier coding
struct Vector3{
    float x,y,z;
};
struct Matrix{
    Vector3 right,up,front,position;
};

DLL_EXPORT int dot3MeshVBRender(int, int, int, DWORD **);
DLL_EXPORT void MVBcreateShader();
DLL_EXPORT int* compilePixelShader(const int*);
DLL_EXPORT int64_t createShader(UINT*,UINT*);
DLL_EXPORT UINT setPixelShader(int);
DLL_EXPORT int drawLight(int, int, int, DWORD **);
DLL_EXPORT void disablePixelShaderInAlpha();


#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
