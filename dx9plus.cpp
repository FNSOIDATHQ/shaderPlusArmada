#include "dx9plus.h"

//DX library
//from dx9.0c sdk
#include <d3d9.h>
#include <d3dx9.h>

// ---------------------------------------------------------
// Global engine parameters/structures
// ---------------------------------------------------------

int* gRenderer = (int*)0x5AA126BC;         // Delphi Renderer instance pointer
DWORD* dword_5A9EFDA0 = (DWORD*)0x5A9EFDA0;  // param passed to Renderer_create

int** sm_faces_remaining_ptr_ptr = (int**)0x5AA12624; // face count limit

int shaderID = (int)0x72B52C;
int* ST3D_Dot3_MeshVB_VertexDeclaration = (int*)0x5AA4927C;
int* ST3D_Dot3_MeshVB_VertexShader = (int*)0x5AA49278;

float* world_to_node = (float*)0x5AA12790;

struct ST3D_Dot3_MeshVB
{
    void* vtable;
    int m_num_vertex_groups;
    VertexGroup_Info* m_vg_info;
    void* m_mesh;
};

struct ST3D_GraphicsEngine
{
    char pad_0000[0xC0];           // skip everything up to offset 0xC0
    int m_current_device_index;     // 0xC0 (192)
    char pad_00C4[4];               // skip padding C4..C7
    int m_device[2];       // 0xCC (204) ST3D_Device*
};

// =========================================================
//  External function pointers
// =========================================================

int (__stdcall* Renderer_create)(DWORD* param,int enable)
= (decltype(Renderer_create))0x5A9EFDBC;

void (__stdcall* TST3D_Device_SetTexture)(int device,DWORD texture,int stage)
= (decltype(TST3D_Device_SetTexture))0x5A9E227C;

void (__stdcall* TST3D_Device_SetBlendingMode)(int device,int src,int dst,int alphaTest,int enable)
= (decltype(TST3D_Device_SetBlendingMode))0x5A9E2218;

void (__stdcall* TST3D_DeviceDirectX8_SetTextureStageState)(int device,int stage,int type,int value)
= (decltype(TST3D_DeviceDirectX8_SetTextureStageState))0x5A9E39CC;

void (__stdcall* TST3D_Device_GetPlatformSpecific)(int device,int token,int* outDevice9)
= (decltype(TST3D_Device_GetPlatformSpecific))0x5A9E21F4;

void (__stdcall* TST3D_Device_SetSpecularEnable)(int device,int enable)
= (decltype(TST3D_Device_SetSpecularEnable))0x5A9E2244;

void (__stdcall* TST3D_Device_SetDiffuseEnable)(int device,int enable)
= (decltype(TST3D_Device_SetDiffuseEnable))0x5A9E2260;


UINT (__stdcall* getShaderHandle9)(int) = (UINT (__stdcall*)(int))0x62C270;
IDirect3DVertexShader9* GetSH (ST3D_GraphicsEngine* Storm3D,int id) {
    IDirect3DVertexShader9* shader;
    __asm {
        mov ecx,Storm3D
        push id
        call getShaderHandle9
        mov shader,eax
    }
    return shader;
}

void* (__stdcall* GetVertexBufferObject)(int) = (void* (__stdcall*)(int))0x62C1B0;
IDirect3DVertexBuffer9* GetVB (ST3D_GraphicsEngine* Storm3D,int id) {
    IDirect3DVertexBuffer9* buffer;
    __asm {
        mov ecx,Storm3D
        push id
        call GetVertexBufferObject
        mov buffer,eax
    }
    return buffer;
}

void* (__stdcall* GetIndexBufferObject)(int) = (void* (__stdcall*)(int))0x62C210;
IDirect3DIndexBuffer9* GetIB (ST3D_GraphicsEngine* Storm3D,int id) {
    IDirect3DIndexBuffer9* buffer;
    __asm {
        mov ecx,Storm3D
        push id
        call GetIndexBufferObject
        mov buffer,eax
    }
    return buffer;
}

// Per-light rendering
unsigned char (__stdcall* ST3D_Dot3_MeshVB_DrawLight_New_0)(
    void* self,
    IDirect3DDevice9* d3dDev9,
    float* lightingMaterial,
    float* lightData,
    int vgiAddr,
    int numFaces)
    = (decltype(ST3D_Dot3_MeshVB_DrawLight_New_0))0x5A9F1ABC;


//call every frame

//in dx8 version, vgIndex will later be used to carry IDirect3DDevice8
//we will not do same thing here
int __fastcall dot3MeshVBRender9 (
    DWORD** textures,           // [esp+ 8]
    int vgIndex,                // [esp+ C]
    float* lightingMaterial)    // [esp+14]
{
    ST3D_Dot3_MeshVB* self;
    __asm {
        call getThisPtrFromECX
        mov self,eax
    }

    // i dont think this will actually do anything
    if(*gRenderer == 0) {
        *gRenderer = Renderer_create (dword_5A9EFDA0,1);
    }

    // Select correct VertexGroup_Info
    VertexGroup_Info* vgi = &self->m_vg_info[vgIndex];
    int numFaces = vgi->num_triangles;

    // Apply face limiting (Storm3D feature)
    int sm_faces_remaining = **sm_faces_remaining_ptr_ptr;
    if(sm_faces_remaining != -1) {
        if(sm_faces_remaining < numFaces) {
            numFaces = **sm_faces_remaining_ptr_ptr;
        }

        **sm_faces_remaining_ptr_ptr -= numFaces;

        //i am not pretty sure, but guess original code is wrong
        //sm_faces_remaining = *(_DWORD *)off_5AA12624
        sm_faces_remaining = **sm_faces_remaining_ptr_ptr;
    }

    //skip if we do not have poly budget
    if(numFaces <= 0) {
        return sm_faces_remaining;
    }

    // Direct3D device from engine

    // Storm3D->m_device[Storm3D->m_current_device_index]
    ST3D_GraphicsEngine* Storm3D = *(ST3D_GraphicsEngine**)0x5AA11CCC;
    int ST3dDevice = Storm3D->m_device[Storm3D->m_current_device_index];
    IDirect3DDevice9* D3DDevice9 = nullptr;
    TST3D_Device_GetPlatformSpecific (ST3dDevice,3,(int*)&D3DDevice9);

    //
    // Stage 1: light aclculation(dir+color) with normal map
    //
    TST3D_Device_SetTexture (ST3dDevice,(*textures)[1],0);
    TST3D_Device_SetBlendingMode (ST3dDevice,2,1,0,1);

    //Stage=0 Type=D3DTSS_COLOROP(1) value=D3DTOP_DOTPRODUCT3(24)
    TST3D_DeviceDirectX8_SetTextureStageState (ST3dDevice,0,1,24);
    //Stage=0 Type=D3DTSS_ALPHAOP(4) value=D3DTOP_DISABLE(1)
    TST3D_DeviceDirectX8_SetTextureStageState (ST3dDevice,0,4,1);
    //Stage=1 Type=D3DTSS_COLOROP(1) value=D3DTOP_MODULATE(4)
    TST3D_DeviceDirectX8_SetTextureStageState (ST3dDevice,1,1,4);
    //Stage=1 Type=D3DTSS_COLORARG1(2) value=D3DTA_TFACTOR(3)
    //this will later be used in ST3D_Dot3_MeshVB_DrawLight_New_0 for light color
    TST3D_DeviceDirectX8_SetTextureStageState (ST3dDevice,1,2,3);
    //Stage=1 Type=D3DTSS_COLORARG2(3) value=D3DTA_CURRENT(1)
    TST3D_DeviceDirectX8_SetTextureStageState (ST3dDevice,1,3,1);
    //Stage=1 Type=D3DTSS_TEXCOORDINDEX(11) value=UV1(1)
    TST3D_DeviceDirectX8_SetTextureStageState (ST3dDevice,1,11,1);

    //this will NOT will in DX9
    //we can use D3DDevice9->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, *(int*)lodbias_0);
    //but I dont think this is quiet necessary since we will remove all fixed function pipeline later
    //Stage=0 Type=D3DTSS_TEXCOORDINDEX(19) value=UV1(1)
    // TST3D_DeviceDirectX8_SetTextureStageState(ST3dDevice, 0, 19, *(int *)lodbias_0);


    // Get SH/VB/IB
    IDirect3DVertexShader9* shader_handle = GetSH (Storm3D,shaderID);
    IDirect3DVertexBuffer9* vbObj = GetVB (Storm3D,vgi->vertex_buffer_id);
    IDirect3DIndexBuffer9* ibObj = GetIB (Storm3D,vgi->index_buffer_id);

    D3DDevice9->SetStreamSource (0,vbObj,0,68);
    D3DDevice9->SetIndices (ibObj);
    D3DDevice9->SetVertexDeclaration ((IDirect3DVertexDeclaration9*)*ST3D_Dot3_MeshVB_VertexDeclaration);
    D3DDevice9->SetVertexShader (shader_handle);

    //
    // Stage 1.1: per light lighting
    //
    // activeLightList = Storm3D->m_active_lights
    DWORD** activeLightList = *(DWORD***)((char*)Storm3D + 96);
    DWORD* it = *activeLightList;

    while(it != *activeLightList) {
        float* lt = (float*)it[2];       // v10
        float* v11 = *(float**)lt;       // shader buffer pointer

        // transform directional vectors and position
        for(int i = 0; i < 4; ++i) {
            float* in = lt + 4 + 3 * i;
            float* out = v11 + 48 + 3 * i;

            out[0] = world_to_node[0] * in[0] + world_to_node[12] * in[1] + world_to_node[24] * in[2];
            out[1] = world_to_node[4] * in[0] + world_to_node[16] * in[1] + world_to_node[28] * in[2];
            out[2] = world_to_node[8] * in[0] + world_to_node[20] * in[1] + world_to_node[32] * in[2];

            if(i == 3) {
                out[0] += world_to_node[36];
                out[1] += world_to_node[40];
                out[2] += world_to_node[44];
            }
        }

        // call per-light dot3 rendering
        if(ST3D_Dot3_MeshVB_DrawLight_New_0 (self,D3DDevice9,lightingMaterial,lt,(int)vgi,numFaces)) {
            TST3D_Device_SetBlendingMode (ST3dDevice,2,2,0,1); // ONE, ONE
        }

        it = (DWORD*)it[0];  // move to next
    }

    //
    // Stage 2: Base texture pass
    //
    TST3D_Device_SetBlendingMode (ST3dDevice,9,1,0,1);
    //guess tex[0]=diffuse tex[1]=normal
    TST3D_Device_SetTexture (ST3dDevice,(*textures)[0],0);
    //Stage=0 Type=D3DTSS_COLOROP(1) value=D3DTOP_SELECTARG1(2)
    TST3D_DeviceDirectX8_SetTextureStageState (ST3dDevice,0,1,2);
    //Stage=1 Type=D3DTSS_COLOROP(1) value=D3DTOP_DISABLE(1)
    TST3D_DeviceDirectX8_SetTextureStageState (ST3dDevice,1,1,1);

    D3DDevice9->DrawIndexedPrimitive (D3DPT_TRIANGLELIST,0,0,vgi->num_vertices,0,numFaces);

    //
    // Optional alpha pass
    //
    if(*(int*)((char*)self->m_mesh + 300) & 4) {
        TST3D_Device_SetBlendingMode (ST3dDevice,5,6,0,1);

        TST3D_Device_SetSpecularEnable (ST3dDevice,0);
        TST3D_Device_SetDiffuseEnable (ST3dDevice,0);

        D3DDevice9->DrawIndexedPrimitive (D3DPT_TRIANGLELIST,0,0,vgi->num_vertices,0,numFaces);
    }
    else {
        //Stage=0 Type=D3DTSS_COLOROP(1) value=D3DTOP_MODULATE(4)
        TST3D_DeviceDirectX8_SetTextureStageState (ST3dDevice,0,1,4);
    }


    // Cleanup to avoid crash on dx8 part(guess)
    D3DDevice9->SetVertexShader (nullptr);
    D3DDevice9->SetVertexDeclaration (nullptr);
    D3DDevice9->SetIndices (nullptr);
    return D3DDevice9->SetStreamSource (0,nullptr,0,0);
}
