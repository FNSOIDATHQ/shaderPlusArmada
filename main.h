#ifndef __MAIN_H__
#define __MAIN_H__

//cpp std library
#include <string>
#include <vector>

//utilities
#include "loadHookTool.h"
#include "dx8plus.h"
#include "dx9plus.h"

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

    DLL_EXPORT bool activate ();

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
