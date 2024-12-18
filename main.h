#ifndef __MAIN_H__
#define __MAIN_H__

//cpp std library+windows sdk
#include <windows.h>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <cstdint>
#include <cstring>
#include <string>

//DX library
//from dx9.0c sdk
#include <d3d8.h>
#include <d3dx8.h>

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

//hook tool
DLL_EXPORT void hookJMP(void*,void*);
DLL_EXPORT void* hookVTable(void**,size_t,void*);
DLL_EXPORT void writeVarToAddress(UINT,UINT,void*);
DLL_EXPORT void writeVarToAddressP(void*,UINT,void*);
DLL_EXPORT void* getClassFunctionAddress(DWORD*,int);
DLL_EXPORT UINT getThisPtrFromECX();
DLL_EXPORT void moveVarToECX(UINT);

//TODO:
DLL_EXPORT void hookTrampoline(void*, void*,void**);



//shader+

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
