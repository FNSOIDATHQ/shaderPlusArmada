#include "dx9plus.h"

//cpp std library
#include <string>
#include <vector>

#include "loadHookTool.h"

//DX library
//from dx9.0c sdk
#include <d3d9.h>
#include <d3dx9.h>

//call every frame
int dot3MeshVBRender9 (int a1,int a2,int a3,DWORD** a4) {

    UINT thisPtr;
    __asm {
        call getThisPtrFromECX
        mov thisPtr,eax
    }

    // void* m_world_to_node; // esi
    // int v5; // edx
    // int result; // eax
    // int v7; // ebx
    // float* v8; // edi
    // unsigned __int8 (__stdcall * v9)(int); // [esp+Ch] [ebp-24h]
    // int ST3dDevice; // [esp+10h] [ebp-20h]
    // DWORD* i; // [esp+14h] [ebp-1Ch]
    // DWORD* v12; // [esp+18h] [ebp-18h]
    // int v13; // [esp+1Ch] [ebp-14h]
    // int v15; // [esp+24h] [ebp-Ch]
    // IDirect3DVertexBuffer9* VertexBufferObject; // [esp+28h] [ebp-8h]
    // IDirect3DDevice9 D3DDevice9; // [esp+2Ch] [ebp-4h] BYREF

    // m_world_to_node = (void*)0x5AA12790;
    // if(!*(DWORD*)0x5AA126BC) {
    //     LOBYTE (v5) = 1;
    //     *(DWORD*)0x5AA126BC = Renderer_create (dword_5A9EFDA0,v5);
    // }
    // ST3dDevice = *(DWORD*)(*0x5AA11CCC + 4 * *(DWORD*)(*0x5AA11CCC + 192) + 204);
    // TST3D_Device_GetPlatformSpecific (ST3dDevice,3,&D3DDevice9);
    // v12 = (DWORD*)(24 * a1 + *(DWORD*)(thisPtr + 8));
    // v13 = v12[5];
    // result = **(DWORD**)0x5AA12624;
    // if(result != -1) {
    //     if(result < v13)
    //         v13 = **(DWORD**)0x5AA12624;
    //     result = *(DWORD*)0x5AA12624;
    //     **(DWORD**)0x5AA12624 -= v13;
    // }
    // if(v13) {
    //     TST3D_Device_SetTexture (ST3dDevice,(*a4)[1],0);
    //     TST3D_Device_SetBlendingMode (ST3dDevice,2,1,0,1);
    //     TST3D_DeviceDirectX8_SetTextureStageState (24);
    //     TST3D_DeviceDirectX8_SetTextureStageState (1);
    //     TST3D_DeviceDirectX8_SetTextureStageState (4);
    //     TST3D_DeviceDirectX8_SetTextureStageState (3);
    //     TST3D_DeviceDirectX8_SetTextureStageState (1);
    //     TST3D_DeviceDirectX8_SetTextureStageState (1);
    //     TST3D_DeviceDirectX8_SetTextureStageState (*(DWORD*)lodbias_0);
    //     (*(void (__stdcall**)(DWORD))getShaderHandle)(**(DWORD**)0x5AA123E8);

    //     VertexBufferObject = ((IDirect3DVertexBuffer9 * (__stdcall*)(DWORD)) * (DWORD*)0x5AA12FA4)(*v12);
    //     v15 = (*(int (__stdcall**)(DWORD))0x5AA12774)(v12[2]);

    //     D3DDevice9->SetStreamSource(0,VertexBufferObject,0,68);

    //     (*(void (__stdcall**)(int,int))(*(DWORD*)D3DDevice9 + 416))(D3DDevice9,v15);
    //     (*(void (__stdcall**)(int,int))(*(DWORD*)D3DDevice9 + 348))(D3DDevice9,ST3D_Dot3_MeshVB_VertexDeclaration);
    //     (*(void (__stdcall**)(int,int))(*(DWORD*)D3DDevice9 + 368))(D3DDevice9,ST3D_Dot3_MeshVB_VertexShader);
    //     for(i = **(DWORD***)(*0x5AA11CCC + 96); *(DWORD**)(*0x5AA11CCC + 96) != i; i = (DWORD*)*i) {
    //         v7 = i[2];
    //         v8 = *(float**)v7;
    //         v8[48] = **(float**)m_world_to_node * *(float*)(v7 + 16)
    //             + *(float*)(*(DWORD*)m_world_to_node + 12) * *(float*)(v7 + 20)
    //             + *(float*)(*(DWORD*)m_world_to_node + 24) * *(float*)(v7 + 24);
    //         v8[49] = *(float*)(*(DWORD*)m_world_to_node + 4) * *(float*)(v7 + 16)
    //             + *(float*)(*(DWORD*)m_world_to_node + 16) * *(float*)(v7 + 20)
    //             + *(float*)(*(DWORD*)m_world_to_node + 28) * *(float*)(v7 + 24);
    //         v8[50] = *(float*)(*(DWORD*)m_world_to_node + 8) * *(float*)(v7 + 16)
    //             + *(float*)(*(DWORD*)m_world_to_node + 20) * *(float*)(v7 + 20)
    //             + *(float*)(*(DWORD*)m_world_to_node + 32) * *(float*)(v7 + 24);
    //         v8[51] = **(float**)m_world_to_node * *(float*)(v7 + 28)
    //             + *(float*)(*(DWORD*)m_world_to_node + 12) * *(float*)(v7 + 32)
    //             + *(float*)(*(DWORD*)m_world_to_node + 24) * *(float*)(v7 + 36);
    //         v8[52] = *(float*)(*(DWORD*)m_world_to_node + 4) * *(float*)(v7 + 28)
    //             + *(float*)(*(DWORD*)m_world_to_node + 16) * *(float*)(v7 + 32)
    //             + *(float*)(*(DWORD*)m_world_to_node + 28) * *(float*)(v7 + 36);
    //         v8[53] = *(float*)(*(DWORD*)m_world_to_node + 8) * *(float*)(v7 + 28)
    //             + *(float*)(*(DWORD*)m_world_to_node + 20) * *(float*)(v7 + 32)
    //             + *(float*)(*(DWORD*)m_world_to_node + 32) * *(float*)(v7 + 36);
    //         v8[54] = **(float**)m_world_to_node * *(float*)(v7 + 40)
    //             + *(float*)(*(DWORD*)m_world_to_node + 12) * *(float*)(v7 + 44)
    //             + *(float*)(*(DWORD*)m_world_to_node + 24) * *(float*)(v7 + 48);
    //         v8[55] = *(float*)(*(DWORD*)m_world_to_node + 4) * *(float*)(v7 + 40)
    //             + *(float*)(*(DWORD*)m_world_to_node + 16) * *(float*)(v7 + 44)
    //             + *(float*)(*(DWORD*)m_world_to_node + 28) * *(float*)(v7 + 48);
    //         v8[56] = *(float*)(*(DWORD*)m_world_to_node + 8) * *(float*)(v7 + 40)
    //             + *(float*)(*(DWORD*)m_world_to_node + 20) * *(float*)(v7 + 44)
    //             + *(float*)(*(DWORD*)m_world_to_node + 32) * *(float*)(v7 + 48);
    //         v8[57] = **(float**)m_world_to_node * *(float*)(v7 + 52)
    //             + *(float*)(*(DWORD*)m_world_to_node + 12) * *(float*)(v7 + 56)
    //             + *(float*)(*(DWORD*)m_world_to_node + 24) * *(float*)(v7 + 60)
    //             + *(float*)(*(DWORD*)m_world_to_node + 36);
    //         v8[58] = *(float*)(*(DWORD*)m_world_to_node + 4) * *(float*)(v7 + 52)
    //             + *(float*)(*(DWORD*)m_world_to_node + 16) * *(float*)(v7 + 56)
    //             + *(float*)(*(DWORD*)m_world_to_node + 28) * *(float*)(v7 + 60)
    //             + *(float*)(*(DWORD*)m_world_to_node + 40);
    //         v8[59] = *(float*)(*(DWORD*)m_world_to_node + 8) * *(float*)(v7 + 52)
    //             + *(float*)(*(DWORD*)m_world_to_node + 20) * *(float*)(v7 + 56)
    //             + *(float*)(*(DWORD*)m_world_to_node + 32) * *(float*)(v7 + 60)
    //             + *(float*)(*(DWORD*)m_world_to_node + 44);
    //         v9 = *(unsigned __int8 (__stdcall**)(int))(*(DWORD*)v8 + 72);
    //         movToECX (v8);
    //         if(v9 (*(DWORD*)(thisPtr + 12) + 212) && (unsigned __int8)ST3D_Dot3_MeshVB_DrawLight_New_0 (v13,v12,v7))
    //             TST3D_Device_SetBlendingMode (ST3dDevice,2,2,0,1);
    //     }
    //     TST3D_Device_SetBlendingMode (ST3dDevice,9,1,0,1);
    //     TST3D_Device_SetTexture (ST3dDevice,**a4,0);
    //     TST3D_DeviceDirectX8_SetTextureStageState (2);
    //     TST3D_DeviceDirectX8_SetTextureStageState (1);
    //     (*(void (__stdcall**)(int,int,DWORD,DWORD,DWORD,DWORD,int))(*(DWORD*)D3DDevice9 + 328))(
    //         D3DDevice9,
    //         4,
    //         0,
    //         0,
    //         v12[4],
    //         0,
    //         v13);
    //     if((*(DWORD*)(*(DWORD*)(thisPtr + 12) + 300) & 4) != 0) {
    //         TST3D_Device_SetBlendingMode (ST3dDevice,5,6,0,1);
    //         TST3D_Device_SetSpecularEnable (ST3dDevice,0);
    //         TST3D_Device_SetDiffuseEnable (ST3dDevice,0);
    //         (*(void (__stdcall**)(int,int,DWORD,DWORD,DWORD,DWORD,int))(*(DWORD*)D3DDevice9 + 328))(
    //             D3DDevice9,
    //             4,
    //             0,
    //             0,
    //             v12[4],
    //             0,
    //             v13);
    //     }
    //     else {
    //         TST3D_DeviceDirectX8_SetTextureStageState (4);
    //     }
    //     (*(void (__stdcall**)(int,DWORD))(*(DWORD*)D3DDevice9 + 368))(D3DDevice9,0);
    //     (*(void (__stdcall**)(int,DWORD))(*(DWORD*)D3DDevice9 + 348))(D3DDevice9,0);
    //     (*(void (__stdcall**)(int,DWORD))(*(DWORD*)D3DDevice9 + 416))(D3DDevice9,0);
    //     return (*(int (__stdcall**)(int,DWORD,DWORD,DWORD,DWORD))(*(DWORD*)D3DDevice9 + 400))(D3DDevice9,0,0,0,0);
    // }
    // return result;

    return 1;
}
