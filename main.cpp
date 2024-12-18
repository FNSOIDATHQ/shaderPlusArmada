#include "main.h"

//Armada function
int* (*CD3MVB)(const int*);
int64_t* (__thiscall*ST3D_DeviceDirectX8_CreateShader)(void*,UINT*,UINT*);
UINT(__stdcall*getShaderHandle)(int);

//FO function
int (__stdcall*D3MVB_R)(int,int,int,DWORD**);
int (__stdcall*DR)(int,int,int,DWORD**);

//Armada pointer
//int* storm3D=(int*)0x7AD508;
Matrix* cameraToNode=(Matrix*)0x7AD5E0;
DWORD** curTextureSet;
UINT StormDevice;

//DX pointer
//this will be update every time shader create is called
//that is where the first time we need this pointer
IDirect3DDevice8* device;

//pixel shader
//i do not know why but shader in armada is managed by a hash map
//in real game there is probably less than 10 shader instances(maybe 1 actually)
//so i only use a variable to save pixel shader
LPVOID psCompiled;
DWORD PSHandle;

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    //only hook when dll is attaching
    if(fdwReason==DLL_PROCESS_ATTACH){

        int (*init)();
        int (*createHook)(LPVOID,LPVOID,LPVOID*);
        int (*enableHook)(LPVOID);

        HINSTANCE hookToolKit = LoadLibrary(".\\dll\\MinHook.x86.dll");
        if(hookToolKit==NULL){
            MessageBoxA(0, "can not load hookToolKit!", "test", MB_OK | MB_ICONINFORMATION);
            return TRUE;
        }

        //load minHook, we use its hookTrampoline function, I haven't imp my own version
        init = (int (*)())GetProcAddress(hookToolKit, "MH_Initialize");
        if(init==NULL){
            MessageBoxA(0, "can not  load MH_Initialize!", "test", MB_OK | MB_ICONINFORMATION);
        }
        createHook = (int (*)(LPVOID,LPVOID,LPVOID*))GetProcAddress(hookToolKit, "MH_CreateHook");
        if(createHook==NULL){
            MessageBoxA(0, "can not  load MH_CreateHook!", "test", MB_OK | MB_ICONINFORMATION);
        }
        enableHook = (int (*)(LPVOID))GetProcAddress(hookToolKit, "MH_EnableHook");
        if(enableHook==NULL){
            MessageBoxA(0, "can not  load MH_EnableHook!", "test", MB_OK | MB_ICONINFORMATION);
        }

        init();

//======compile shader
        char newVertexShaderPath[]="shaders\\dx8\\vertex\\vs.nvv";
        writeVarToAddressP((void*)0x72B580,sizeof(newVertexShaderPath),(void*)newVertexShaderPath);
        createHook((LPVOID)0x626E50, (LPVOID)compilePixelShader, reinterpret_cast<LPVOID*>(&CD3MVB));
        enableHook((LPVOID)0x626E50);


//======create shader
        //this will cause a crash, possiblely because FO using a different call method in its work
        //createHook((LPVOID)0x6226B0, (LPVOID)createPixelShader, reinterpret_cast<LPVOID*>(&ST3D_DeviceDirectX8_CreateShader));
        //enableHook((LPVOID)0x6226B0);

        ST3D_DeviceDirectX8_CreateShader=reinterpret_cast<int64_t* (__thiscall*)(void*,UINT*,UINT*)>(hookVTable((void**)0x6BC6AC,18,(void*)createShader));

//======set shader
        //set shader in getShaderHandle
        createHook((LPVOID)0x62C270, (LPVOID)setPixelShader, reinterpret_cast<LPVOID*>(&getShaderHandle));
        enableHook((LPVOID)0x62C270);

        //rework drawLight
        //createHook((LPVOID)0x5A9E610C, (LPVOID)drawLight, reinterpret_cast<LPVOID*>(&DR));
        //enableHook((LPVOID)0x5A9E610C);

//======disable shader when enter fixed function pipeline
        //at the end of dot3MeshVB::Render
        //hookTrampoline((void*)6452640,(void*)dot3MeshVBRender);
        createHook((LPVOID)0x5A9E6320, (LPVOID)dot3MeshVBRender, reinterpret_cast<LPVOID*>(&D3MVB_R));
        enableHook((LPVOID)0x5A9E6320);

        //at the last if of dot3MeshVB::Render
        //this is a hack, most of time do not hook function at middle of it
        uint8_t jmpInstruction[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0};

        const uint32_t relativeAddr = (uint32_t)disablePixelShaderInAlpha - ((uint32_t)0x5A9E67D1 + 5);
        memcpy(jmpInstruction + 1, &relativeAddr, 4);

        writeVarToAddress(0x5A9E67D1,sizeof(jmpInstruction),(void*)jmpInstruction);




        FreeLibrary(hookToolKit);
    }
    return TRUE; // succesful
}


//============================================hook tool start

//reference:
//https://kylehalladay.com/blog/2020/11/13/Hooking-By-Example.html
//https://ocw.cs.pub.ro/courses/so/laboratoare/resurse/injections

//armada is 32bit so this is enough for now
//simple JMP will break original function
void hookJMP(void* oriFunc, void* replFunc){
    DWORD oldProtectValue;
    //func(StartAddress,size,protectStatus,oldStatus)
    //i am not pretty sure, but 5 not 1024 is completely enough
    VirtualProtect(oriFunc, 1024, PAGE_EXECUTE_READWRITE, &oldProtectValue);

    //32 bit relative jump opcode is E9, takes 1 32 bit operand for jump offset
    uint8_t jmpInstruction[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };

    //to fill out the last 4 bytes of jmpInstruction, we need the offset between
    //the payload function and the instruction immediately AFTER the jmp instruction
    const uint32_t relativeAddr = (uint32_t)replFunc - ((uint32_t)oriFunc + sizeof(jmpInstruction));
    memcpy(jmpInstruction + 1, &relativeAddr, 4);

    //install the hook
    memcpy(oriFunc, jmpInstruction, sizeof(jmpInstruction));

    VirtualProtect(oriFunc, 1024, oldProtectValue, &oldProtectValue);
}

//TODO:
//run our own code and back to original
void hookTrampoline(void* oriFunc, void* replFunc,void** trampolinePtr){
    DWORD oldProtectValue;
    VirtualProtect(oriFunc, 1024, PAGE_EXECUTE_READWRITE, &oldProtectValue);

    //void* hookMemory = AllocatePageNearAddress(oriFunc);
    //uint32_t trampolineSize = BuildTrampoline(oriFunc, hookMemory);
    //*trampolinePtr = hookMemory;

    //32 bit relative jump opcode is E9, takes 1 32 bit operand for jump offset
    uint8_t jmpInstruction[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };

    //to fill out the last 4 bytes of jmpInstruction, we need the offset between
    //the payload function and the instruction immediately AFTER the jmp instruction
    const uint32_t relativeAddr = (uint32_t)replFunc - ((uint32_t)oriFunc + sizeof(jmpInstruction));
    memcpy(jmpInstruction + 1, &relativeAddr, 4);

    //install the hook
    memcpy(oriFunc, jmpInstruction, sizeof(jmpInstruction));

    VirtualProtect(oriFunc, 1024, oldProtectValue, &oldProtectValue);
}

//reference https://www.codereversing.com/archives/596
//return original function ptr
void* hookVTable(void** vtableBaseAddress,size_t index,void* hookAddress) {
    void* originalFunc=vtableBaseAddress[index];
    DWORD oldProtectValue;
    VirtualProtect(&vtableBaseAddress[index], sizeof(intptr_t), PAGE_EXECUTE_READWRITE, &oldProtectValue);

    memcpy(&vtableBaseAddress[index], &hookAddress, sizeof(intptr_t));

    VirtualProtect(&vtableBaseAddress[index], sizeof(intptr_t), oldProtectValue, &oldProtectValue);

    return originalFunc;
}

void writeVarToAddress(UINT address,UINT dataSize,void* input){
    void* targetPointer=(void*)address;

    DWORD oldProtectValue;
    VirtualProtect(targetPointer, dataSize, PAGE_EXECUTE_READWRITE, &oldProtectValue);

    memcpy(targetPointer, input, dataSize);

    VirtualProtect(targetPointer, dataSize, oldProtectValue, &oldProtectValue);
}

void writeVarToAddressP(void* targetPointer,UINT dataSize,void* input){
    DWORD oldProtectValue;
    VirtualProtect(targetPointer, dataSize, PAGE_EXECUTE_READWRITE, &oldProtectValue);

    memcpy(targetPointer, input, dataSize);

    VirtualProtect(targetPointer, dataSize, oldProtectValue, &oldProtectValue);
}

//this function is unchecked
void* getClassFunctionAddress(DWORD* classPtr,int index){
    DWORD** classVtable=(DWORD**)*classPtr;
    //std::string msg= "classVtable"+std::to_string((int)classVtable);
    //MessageBoxA(0,msg.c_str(), "test", MB_OK | MB_ICONINFORMATION);

    //void* func=(void*)*(DWORD*)(classVtable+sizeof(classVtable)*index);
    void* func=(void*)classVtable[index];
    //msg= "func"+std::to_string((int)func);
    //MessageBoxA(0, msg.c_str(), "test", MB_OK | MB_ICONINFORMATION);

     return func;
}

__declspec(naked) UINT getThisPtrFromECX(){
    __asm{
        mov     eax, ecx
        retn
    }
}

__declspec(naked) void moveVarToECX(UINT input){
    __asm{
        lea     ecx, [eax]
        retn
    }
}

//============================================hook tool end

int* compilePixelShader(const int* mesh){
    //MessageBoxA(0, "compilePixelShader", "test", MB_OK | MB_ICONINFORMATION);

    //original function:ST3D_CreateDot3MeshVB(ST3D_Mesh const *)
    int* meshOut=CD3MVB(mesh);

    //compile shader from file
    LPCSTR pixelShaderPath="shaders\\dx8\\pixel\\ps.nvv";
    DWORD flag=0;

    LPD3DXBUFFER error;
    LPD3DXBUFFER buffer;

    HRESULT result=D3DXAssembleShaderFromFileA(pixelShaderPath,flag,NULL,&buffer,&error);



    if(result!=0){
        MessageBoxA(0, "error compile pixel shader!", "test", MB_OK | MB_ICONINFORMATION);
    }
    else{
        psCompiled=buffer->GetBufferPointer();
    }

    return meshOut;
}

int64_t createShader(UINT* pShader,UINT* pDeclaration){
    //MessageBoxA(0, "createShader", "test", MB_OK | MB_ICONINFORMATION);

    UINT thisPtr;
    __asm{
        call getThisPtrFromECX
        mov thisPtr,eax
    }

    //old method to create vertex shader
    //run original function to cerate vertex shader
    //int64_t* finalShader=ST3D_DeviceDirectX8_CreateShader((void*)thisPtr,pShader,pDeclaration);
    //MessageBoxA(0, std::to_string((int)finalShader).c_str(), "test", MB_OK | MB_ICONINFORMATION);



    StormDevice=thisPtr;
    //this->m_pD3DDevice
    device=(IDirect3DDevice8*)*(DWORD*)(thisPtr+0x90);
    //MessageBoxA(0, std::to_string((int)device).c_str(), "test", MB_OK | MB_ICONINFORMATION);


    //this is equal to upon
    /*
    int(__thiscall*getPlatormSpec)(DWORD*,DWORD*, DWORD*)=(int(__thiscall*)(DWORD*,DWORD*, DWORD*))(getClassFunctionAddress((DWORD*)thisPtr,48));
    DWORD* deviceFromFunc=new DWORD();
    //moveVarToECX(thisPtr);
    int DXResult=getPlatormSpec((DWORD*)thisPtr,(DWORD*)3,deviceFromFunc);

    //MessageBoxA(0, std::to_string((int)DXResult).c_str(), "test", MB_OK | MB_ICONINFORMATION);
    deviceFromFunc=(DWORD*)*deviceFromFunc;
    MessageBoxA(0, std::to_string((int)*deviceFromFunc).c_str(), "test", MB_OK | MB_ICONINFORMATION);
    device=(IDirect3DDevice8*)deviceFromFunc;
    */




    //create shaders
    HRESULT VSresult=device->CreateVertexShader((DWORD*)pDeclaration,(DWORD*)pShader,(DWORD*)&pShader,0);
    HRESULT PSresult=device->CreatePixelShader((DWORD*)psCompiled,&PSHandle);

    if(VSresult!=0){
        MessageBoxA(0, "error create vertex shader!", "test", MB_OK | MB_ICONINFORMATION);
    }
    if(PSresult!=0){
        MessageBoxA(0, "error create pixel shader!", "test", MB_OK | MB_ICONINFORMATION);
    }

    //this part of code is not correct, but i leave it for reference
/*
    unsigned int handle;
    DWORD* devicePtr=(DWORD*)(*(DWORD*)thisPtr+0x90);
    //MessageBoxA(0, std::to_string((int)devicePtr).c_str(), "test", MB_OK | MB_ICONINFORMATION);


    void* func=getClassFunctionAddress(devicePtr,75);
    //void* func=(void*)((int)devicePtr+0x12C);
    //MessageBoxA(0, std::to_string((int)func).c_str(), "test", MB_OK | MB_ICONINFORMATION);

    int(__stdcall*createVSFunc)(DWORD*, const unsigned int *, const unsigned int *, unsigned int *, unsigned int)=(int(__stdcall*)(DWORD*, const unsigned int *, const unsigned int *, unsigned int *, unsigned int))(func);
    MessageBoxA(0, "get function", "test", MB_OK | MB_ICONINFORMATION);
    int result=createVSFunc(
    devicePtr,
    (const unsigned int *)pDeclaration,
    (const unsigned int *)pShader,
    &handle,
    0);

*/




    //vertex shader constant which do not need update every frame
    //FLOAT ambient[4] = {0.2,0.2,0.2,1};
    //device->SetVertexShaderConstant(20, &ambient, 1);





    //MessageBoxA(0, "end create pixel shader!", "test", MB_OK | MB_ICONINFORMATION);
    //return (UINT)handle;
    //return (int64_t)finalShader;
    return (int64_t)pShader;
}

UINT setPixelShader(int id){
    UINT thisPtr;
    __asm{
        call getThisPtrFromECX
        mov thisPtr,eax
    }

    //original function
    UINT value=getShaderHandle(id);


    //get function for set texture
    //this is NOT correct
    /*
    //this->m_device
    DWORD** StormDeviceTable=(DWORD**)*(DWORD*)(thisPtr+0xCC);
    //this->m_current_device_index
    int curDeviceIndex=(int)*(DWORD*)(thisPtr+0xC0);
    //m_device[m_current_device_index]
    //current device class of storm3d engine
    UINT StormDevice=(UINT)*StormDeviceTable[curDeviceIndex];

    //ST3D_DeviceDirectX8::SetTexture
    void(__stdcall*ST3DSetTexture)(DWORD*,UINT)=(void(__stdcall*)(DWORD*,UINT))getClassFunctionAddress((DWORD*)StormDevice,25);
    */

    /*
    void(__stdcall*ST3DSetTexture)(DWORD*,UINT)=(void(__stdcall*)(DWORD*,UINT))0x624AB0;
    //MessageBoxA(0, std::to_string((int)ST3DSetTexture).c_str(), "test", MB_OK | MB_ICONINFORMATION);
    //get texture pointer
    DWORD** textures=(DWORD**)*(DWORD*)(curTextureSet);

    //set normal map input to r2
    //i think there is something in r1,because game will setTextureStageState to r1, though i do not actually found anything
    UINT SD=*(DWORD*)StormDevice;
    __asm{
        lea     ecx, [SD]
    }
    ST3DSetTexture(textures[2],2);
    */

    //textureStage1 is set in original armada to calculate light color
    device->SetTextureStageState(1,D3DTSS_COLOROP,2 );
    device->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,0 );


    //set constant which need update every frame for vertex shader
    D3DXMATRIX matTemp;
    D3DXMATRIX matWorld;
    D3DXMATRIX matView;

    //7-10
    device->GetTransform(D3DTS_WORLDMATRIX(0), &matWorld);
    D3DXMatrixTranspose(&matTemp, &matWorld);
    device->SetVertexShaderConstant(7, &matTemp, 4);

    //11-14
    device->GetTransform(D3DTS_VIEW, &matView);
    D3DXMatrixTranspose(&matTemp,&(matWorld * matView));
    device->SetVertexShaderConstant(11, &matTemp, 4);

    //15-18
    D3DXMatrixInverse(&matTemp,NULL, &matWorld);
    D3DXMatrixTranspose(&matTemp,&matTemp);
    device->SetVertexShaderConstant(15, &matTemp, 4);

    //19
    Vector3 cameraDir=cameraToNode->front;
    device->SetVertexShaderConstant(19, &cameraDir, 1);
    //std::string text=std::to_string(cameraDir.x)+','+std::to_string(cameraDir.y)+' '+std::to_string(cameraDir.z);
    //MessageBoxA(0, text.c_str(), "test", MB_OK | MB_ICONINFORMATION);


    //
    device->SetPixelShader(PSHandle);

    return value;
}

//call every frame
//TODO: rework drawLight to support real per-pixel multi-light source rendering
int drawLight(int a1, int a2, int a3, DWORD **a4){

    UINT thisPtr;
    __asm{
        call getThisPtrFromECX
        mov thisPtr,eax
    }

}

//call every frame
int dot3MeshVBRender(int a1, int a2, int a3, DWORD **a4){

    UINT thisPtr;
    __asm{
        call getThisPtrFromECX
        mov thisPtr,eax
    }

    //before render
    //we will use this in setPixelShader(id)
    curTextureSet=a4;

    //original FO render
    __asm{

        mov     eax, a4
        push    eax
        mov     eax, a3
        push    eax
        mov     edx, a2
        push    edx
        mov     eax, a1
        push    eax
        mov     ecx, thisPtr
        call D3MVB_R
    }

    //avoid using ecx
    //in default c++ eax,ecx,edx will be use for input transfer,
    //so we manually skip ecx by writing asm
    //return D3MVB_R(a1,a2,a3,a4);


    //after render

    //disable pixel shader to let non-shader mesh render under fixed-pipeline
    //do this work in disablePixelShaderInAlpha() as a hack
    //device->SetPixelShader(NULL);


}

//we disable the last two DrawIndexedPrimitive, they will cause bug in current pipeline
//better solution is rework the ST3D_Dot3_MeshVB::Render function
__declspec(naked) void disablePixelShaderInAlpha(){

    device->SetPixelShader(NULL);

    //this is a hack, better not set render state here
    //fixed pipeline phong shading is not support in most hardware
    //but if any gpu support it, this can make sense
    device->SetRenderState(D3DRS_SHADEMODE ,3);

    __asm{
        pop edi
        pop esi
        pop ebx
        mov esp,ebp
        pop ebp
        retn 0x10
    }

}



//this function seems won't call in real gaming
void MVBcreateShader(){
    MessageBoxA(0, "createShader", "test", MB_OK | MB_ICONINFORMATION);
    return;
}
