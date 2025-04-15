#include "loadHookTool.h"

 void (*hookJMP)(void*,void*);
 void* (*hookVTable)(void**,size_t,void*);
 void (*writeVarToAddress)(UINT,UINT,void*);
 void (*writeVarToAddressP)(void*,UINT,void*);
 void (*writeNopsToAddress)(UINT,UINT);
 void* (*getClassFunctionAddress)(DWORD*,int);
 UINT (*getThisPtrFromECX)();
 void (*moveVarToECX)(UINT);

bool loadTools () {
    HINSTANCE hookTool = LoadLibrary (".\\dll\\hookTools.dll");
    if(hookTool == NULL) {
        MessageBoxA (0,"can not load hookTools!","test",MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }

    //load hook tools
    hookJMP = (void (*)(void*,void*))GetProcAddress (hookTool,"hookJMP");
    hookVTable = (void* (*)(void**,size_t,void*))GetProcAddress (hookTool,"hookVTable");
    writeVarToAddress = (void (*)(UINT,UINT,void*))GetProcAddress (hookTool,"writeVarToAddress");
    writeVarToAddressP = (void (*)(void*,UINT,void*))GetProcAddress (hookTool,"writeVarToAddressP");
    writeNopsToAddress = (void (*)(UINT,UINT))GetProcAddress (hookTool,"writeNopsToAddress");
    getClassFunctionAddress = (void* (*)(DWORD*,int))GetProcAddress (hookTool,"getClassFunctionAddress");
    getThisPtrFromECX = (UINT (*)())GetProcAddress (hookTool,"getThisPtrFromECX");
    moveVarToECX = (void (*)(UINT))GetProcAddress (hookTool,"moveVarToECX");

    return TRUE;
}
