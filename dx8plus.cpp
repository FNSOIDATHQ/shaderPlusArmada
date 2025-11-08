#include "dx8plus.h"

//DX library
//from dx9.0c sdk
#include <d3d8.h>
#include <d3dx8.h>

//Armada pointer
//int* storm3D=(int*)0x7AD508;
Matrix* cameraToNode = (Matrix*)0x7AD5E0;
DWORD** curTextureSet;
UINT stormDevice;

//DX pointer
//this will be update every time shader create is called
//that is where the first time we need this pointer
IDirect3DDevice8* device;

//pixel shader
//i do not know why but shader in armada is managed by a hash map
//in real game there is probably less than 10 shader instances(maybe 1 actually)
//so i only use a variable to save pixel shader
LPVOID psCompiled;
DWORD psHandle;


int* __stdcall compilePixelShader (const int* mesh) {
    //MessageBoxA(0, "compilePixelShader", "test", MB_OK | MB_ICONINFORMATION);

    //original function:ST3D_CreateDot3MeshVB(ST3D_Mesh const *)
    int* meshOut = CD3MVB (mesh);

    //compile shader from file
    LPCSTR pixelShaderPath = "shaders\\dx8\\pixel\\ps.nvv";
    DWORD flag = 0;

    LPD3DXBUFFER error;
    LPD3DXBUFFER buffer;

    HRESULT result = D3DXAssembleShaderFromFileA (pixelShaderPath,flag,NULL,&buffer,&error);



    if(result != 0) {
        MessageBoxA (0,"error compile pixel shader!","test",MB_OK | MB_ICONINFORMATION);
    }
    else {
        psCompiled = buffer->GetBufferPointer ();
    }

    return meshOut;
}

int64_t __stdcall createShader (UINT* pShader,UINT* pDeclaration) {
    //MessageBoxA(0, "createShader", "test", MB_OK | MB_ICONINFORMATION);

    UINT thisPtr;
    __asm {
        call getThisPtrFromECX
        mov thisPtr,eax
    }

    //old method to create vertex shader
    //run original function to cerate vertex shader
    //int64_t* finalShader=ST3D_DeviceDirectX8_CreateShader((void*)thisPtr,pShader,pDeclaration);
    //MessageBoxA(0, std::to_string((int)finalShader).c_str(), "test", MB_OK | MB_ICONINFORMATION);



    stormDevice = thisPtr;
    //this->m_pD3DDevice
    device = (IDirect3DDevice8*)*(DWORD*)(thisPtr + 0x90);
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
    HRESULT VSresult = device->CreateVertexShader ((DWORD*)pDeclaration,(DWORD*)pShader,(DWORD*)&pShader,0);
    HRESULT PSresult = device->CreatePixelShader ((DWORD*)psCompiled,&psHandle);

    if(VSresult != 0) {
        MessageBoxA (0,"error create vertex shader!","test",MB_OK | MB_ICONINFORMATION);
    }
    if(PSresult != 0) {
        MessageBoxA (0,"error create pixel shader!","test",MB_OK | MB_ICONINFORMATION);
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

UINT __stdcall setPixelShader (int id) {
    UINT thisPtr;
    __asm {
        call getThisPtrFromECX
        mov thisPtr,eax
    }

    //original function
    UINT value = getShaderHandle (id);

    //MessageBoxA(0, "set pixel shader!", "test", MB_OK | MB_ICONINFORMATION);
    //get function for set texture
    //this is NOT correct
    /*
    //this->m_device
    DWORD** StormDeviceTable=(DWORD**)*(DWORD*)(thisPtr+0xCC);
    //this->m_current_device_index
    int curDeviceIndex=(int)*(DWORD*)(thisPtr+0xC0);
    //m_device[m_current_device_index]
    //current device class of storm3d engine
    UINT stormDevice=(UINT)*StormDeviceTable[curDeviceIndex];

    //ST3D_DeviceDirectX8::SetTexture
    void(__stdcall*ST3DSetTexture)(DWORD*,UINT)=(void(__stdcall*)(DWORD*,UINT))getClassFunctionAddress((DWORD*)stormDevice,25);
    */

    /*
    void(__stdcall*ST3DSetTexture)(DWORD*,UINT)=(void(__stdcall*)(DWORD*,UINT))0x624AB0;
    //MessageBoxA(0, std::to_string((int)ST3DSetTexture).c_str(), "test", MB_OK | MB_ICONINFORMATION);
    //get texture pointer
    DWORD** textures=(DWORD**)*(DWORD*)(curTextureSet);

    //set normal map input to r2
    //i think there is something in r1,because game will setTextureStageState to r1, though i do not actually found anything
    UINT SD=*(DWORD*)stormDevice;
    __asm{
        lea     ecx, [SD]
    }
    ST3DSetTexture(textures[2],2);
    */

    //textureStage1 is set in original armada to calculate light color
    device->SetTextureStageState (1,D3DTSS_COLOROP,2);
    device->SetTextureStageState (1,D3DTSS_TEXCOORDINDEX,0);


    //set constant which need update every frame for vertex shader
    D3DXMATRIX matTemp;
    D3DXMATRIX matWorld;
    D3DXMATRIX matView;

    //7-10
    device->GetTransform (D3DTS_WORLDMATRIX (0),&matWorld);
    D3DXMatrixTranspose (&matTemp,&matWorld);
    device->SetVertexShaderConstant (7,&matTemp,4);

    //11-14
    device->GetTransform (D3DTS_VIEW,&matView);
    D3DXMatrixTranspose (&matTemp,&(matWorld * matView));
    device->SetVertexShaderConstant (11,&matTemp,4);

    //15-18
    D3DXMatrixInverse (&matTemp,NULL,&matWorld);
    D3DXMatrixTranspose (&matTemp,&matTemp);
    device->SetVertexShaderConstant (15,&matTemp,4);

    //19
    Vector3 cameraDir = cameraToNode->front;
    device->SetVertexShaderConstant (19,&cameraDir,1);
    //std::string text=std::to_string(cameraDir.x)+','+std::to_string(cameraDir.y)+' '+std::to_string(cameraDir.z);
    //MessageBoxA(0, text.c_str(), "test", MB_OK | MB_ICONINFORMATION);


    //
    device->SetPixelShader (psHandle);

    return value;
}

//call every frame
//TODO: rework drawLight to support real per-pixel multi-light source rendering
int __stdcall drawLight (int a1,int a2,int a3,DWORD** a4) {

    UINT thisPtr;
    __asm {
        call getThisPtrFromECX
        mov thisPtr,eax
    }

}

//call every frame
int __stdcall dot3MeshVBRender (int a1,int a2,int a3,DWORD** a4) {

    UINT thisPtr;
    __asm {
        call getThisPtrFromECX
        mov thisPtr,eax
    }

    //before render
    //we will use this in setPixelShader(id)
    curTextureSet = a4;

    //original FO render
    __asm {

        mov     eax,a4
        push    eax
        mov     eax,a3
        push    eax
        mov     edx,a2
        push    edx
        mov     eax,a1
        push    eax
        mov     ecx,thisPtr
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
__declspec(naked) void disablePixelShaderInAlpha () {

    device->SetPixelShader (NULL);

    //this is a hack, better not set render state here
    //fixed pipeline phong shading is not support in most hardware
    //but if any gpu support it, this can make sense
    device->SetRenderState (D3DRS_SHADEMODE,3);

    __asm {
        pop edi
        pop esi
        pop ebx
        mov esp,ebp
        pop ebp
        retn 0x10
    }

}

//this function seems won't call in real gaming
void __stdcall MVBcreateShader () {
    MessageBoxA (0,"createShader","test",MB_OK | MB_ICONINFORMATION);
    return;
}
