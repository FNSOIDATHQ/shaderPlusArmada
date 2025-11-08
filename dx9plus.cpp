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

struct ST3D_Dot3_MeshVB
{
    void* vtable;
    int m_num_vertex_groups;
    VertexGroup_Info* m_vg_info;
    void* m_mesh;
};

#pragma pack(push, 1)

struct Matrix34
{
    Vector3 right;
    Vector3 up;
    Vector3 front;
    Vector3 position;
};

struct SPHERE {
    Vector3 origin;
    float radius;
};

enum ST3D_Light_Type : int32_t
{
    ST3D_LIGHT_DIRECTIONAL = 1,
    ST3D_LIGHT_POINT = 2,
    ST3D_LIGHT_SPOT = 3,
};

struct ST3D_Colour
{
    float red;
    float green;
    float blue;
};

struct ST3D_Light
{
    char padding_to_C0[0xC0];

    Matrix34 m_light_to_node;
    ST3D_Light_Type m_light_type;
    ST3D_Colour m_colour;
    float m_falloff_start;
    float m_falloff_range;
    Matrix34 m_light_to_camera;
};

struct ST3D_LightInstance
{
    ST3D_Light* m_light;
    ST3D_Colour m_colour;
    Matrix34 m_transform;
    int32_t m_autoDelete;
};

#pragma pack(pop)

struct ST3D_GraphicsEngine
{
    char pad_0000[0x60];           // skip everything up to offset 0x5C
    std::list<ST3D_LightInstance*> m_active_lights;
    char pad_0068[0x58];
    int m_current_device_index;     // 0xC0 (192)
    char pad_00C4[0x8];
    void* m_device[2];       // 0xCC (204) ST3D_Device*
};
// =========================================================
//  External function pointers
// =========================================================

int (__stdcall* Renderer_create)(DWORD* param,int enable)
= (decltype(Renderer_create))0x5A9EFDBC;

//vtable functions
void (__thiscall* setTexture)(void* thisPtr,DWORD texture,int stage);
void (__thiscall* setBlendingMode)(void* thisPtr,int src,int dst,int alphaTest,int enable);
int (__thiscall* getPlatformSpecific)(void* thisPtr,int token,int* outDevice9);
void (__thiscall* setSpecularEnable)(void* thisPtr,int enable);
void (__thiscall* setDiffuseEnable)(void* thisPtr,int enable);

unsigned int (__thiscall* inRange)(ST3D_Light*,void*);

void (__thiscall* TST3D_DeviceDirectX8_SetTextureStageState)(void* thisPtr,int stage,int type,int value)
= (decltype(TST3D_DeviceDirectX8_SetTextureStageState))0x625100;

void (__thiscall* getShaderHandle9)(ST3D_GraphicsEngine* Storm3D,int id) = (decltype(getShaderHandle9))0x62C270;
IDirect3DVertexBuffer9* (__thiscall* getVertexBufferObject)(ST3D_GraphicsEngine* Storm3D,int id) = (decltype(getVertexBufferObject))0x62C1B0;
IDirect3DIndexBuffer9* (__thiscall* getIndexBufferObject)(ST3D_GraphicsEngine* Storm3D,int id) = (decltype(getIndexBufferObject))0x62C210;

// Per-light rendering
// unsigned char (__thiscall* ST3D_Dot3_MeshVB_DrawLight_New_0)(
//     void* self,
//     IDirect3DDevice9* d3dDev9,
//     float* lightingMaterial,
//     ST3D_LightInstance* lightData,
//     int vgiAddr,
//     int numFaces)
//     = (decltype(ST3D_Dot3_MeshVB_DrawLight_New_0))0x5A9F1ABC;

static void* ST3D_Dot3_MeshVB_DrawLight_New_0 = (void*)0x5A9F1ABC;


//call every frame

//in dx8 version, vgIndex will later be used to carry IDirect3DDevice8
//we will not do same thing here
int __fastcall dot3MeshVBRender9 (
    int padding1,
    int padding2,
    int vgIndex,           // [ebp+ 8]
    float* lightingMaterial,                // [ebp+ C]
    int padding3,
    DWORD** textures)    // [ebp+14]
{
    ST3D_Dot3_MeshVB* self;
    __asm {
        call getThisPtrFromECX
        mov self,eax
    }

    // i dont think this will actually do anything
    // if(*gRenderer == 0) {
    //     *gRenderer = Renderer_create (dword_5A9EFDA0,1);
    // }

    // Select correct VertexGroup_Info
    VertexGroup_Info* vgi = &self->m_vg_info[vgIndex];
    int numFaces = vgi->num_triangles;

    // Apply face limiting (Storm3D feature)
    int* sm_faces_remaining_ptr = (int*)0x72C3F4;
    int sm_faces_remaining = *sm_faces_remaining_ptr;
    if(sm_faces_remaining != -1) {
        if(sm_faces_remaining < numFaces) {
            numFaces = *sm_faces_remaining_ptr;
        }

        *sm_faces_remaining_ptr -= numFaces;

        //i am not pretty sure, but guess original code is wrong
        //sm_faces_remaining = *(_DWORD *)off_5AA12624
        sm_faces_remaining = *sm_faces_remaining_ptr;
    }

    //skip if we do not have poly budget
    if(numFaces <= 0) {
        return sm_faces_remaining;
    }

    // Direct3D device from engine
    ST3D_GraphicsEngine* Storm3D = *(ST3D_GraphicsEngine**)0x7AD508;
    int shaderID = *(int*)0x72B52C;
    void* ST3dDevice = Storm3D->m_device[Storm3D->m_current_device_index];
    // MessageBoxA(0, "before get vtable functions", "Checker", MB_OK | MB_ICONINFORMATION);

    //get vtable functions
    getPlatformSpecific = (int (__thiscall*)(void*,int,int*))(getClassFunctionAddress ((DWORD*)ST3dDevice,48));
    setTexture = (void (__thiscall*)(void*,DWORD,int))(getClassFunctionAddress ((DWORD*)ST3dDevice,25));
    setBlendingMode = (void (__thiscall*)(void*,int,int,int,int))(getClassFunctionAddress ((DWORD*)ST3dDevice,36));
    setSpecularEnable = (void (__thiscall*)(void*,int))(getClassFunctionAddress ((DWORD*)ST3dDevice,40));
    setDiffuseEnable = (void (__thiscall*)(void*,int))(getClassFunctionAddress ((DWORD*)ST3dDevice,41));


    IDirect3DDevice9* D3DDevice9 = nullptr;
    getPlatformSpecific (ST3dDevice,3,(int*)&D3DDevice9);
    // MessageBoxA(0, std::to_string((int)D3DDevice9).c_str(), "D3DDevice9 Value", MB_OK | MB_ICONINFORMATION);

    //
    // Stage 1: light aclculation(dir+color) with normal map
    //

    setTexture (ST3dDevice,(*textures)[1],0);
    setBlendingMode (ST3dDevice,2,1,0,1);

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
    getShaderHandle9 (Storm3D,shaderID);
    IDirect3DVertexBuffer9* vbObj = getVertexBufferObject (Storm3D,vgi->vertex_buffer_id);
    IDirect3DIndexBuffer9* ibObj = getIndexBufferObject (Storm3D,vgi->index_buffer_id);

    D3DDevice9->SetStreamSource (0,vbObj,0,68);
    D3DDevice9->SetIndices (ibObj);

    //DX9 has new shader logic, so we can not just use shader handle to set veretx shader
    int* ST3D_Dot3_MeshVB_VertexDeclaration = (int*)0x5AA4927C;
    D3DDevice9->SetVertexDeclaration ((IDirect3DVertexDeclaration9*)*ST3D_Dot3_MeshVB_VertexDeclaration);
    int* ST3D_Dot3_MeshVB_VertexShader = (int*)0x5AA49278;
    D3DDevice9->SetVertexShader ((IDirect3DVertexShader9*)*ST3D_Dot3_MeshVB_VertexShader);

    // MessageBoxA(0, "before Stage 1.1", "Checker", MB_OK | MB_ICONINFORMATION);
    //
    // Stage 1.1: per light lighting
    //

    Matrix34& world_to_node = *(Matrix34*)(0x7AD6A0);
    // std::string wtnChecker = "world_to_node=\n";

    // auto appendVec = [&](const char* name,const Vector3& v) {
    //     wtnChecker += name;
    //     wtnChecker += "(";
    //     wtnChecker += std::to_string (v.x) + ", ";
    //     wtnChecker += std::to_string (v.y) + ", ";
    //     wtnChecker += std::to_string (v.z) + ")";
    //     wtnChecker += " \n";
    //     };

    // appendVec ("right",world_to_node.right);
    // appendVec ("up",world_to_node.up);
    // appendVec ("front",world_to_node.front);
    // appendVec ("position",world_to_node.position);

    // MessageBoxA (0,wtnChecker.c_str(),"Checker",MB_OK | MB_ICONINFORMATION);

    // activeLightList = Storm3D->m_active_lights
    std::list<ST3D_LightInstance*> activeLightList = Storm3D->m_active_lights;

    for(ST3D_LightInstance* inst : activeLightList) {

        Matrix34& dst = inst->m_light->m_light_to_node;   // p_x
        Matrix34& src = inst->m_transform;                // Value->m_transform

        //matrix33
        dst.right.x = world_to_node.right.x * src.right.x
            + world_to_node.front.x * src.right.z
            + world_to_node.up.x * src.right.y;

        dst.right.y = world_to_node.right.y * src.right.x
            + world_to_node.front.y * src.right.z
            + world_to_node.up.y * src.right.y;

        dst.right.z = world_to_node.right.z * src.right.x
            + world_to_node.front.z * src.right.z
            + world_to_node.up.z * src.right.y;

        dst.up.x = world_to_node.right.x * src.up.x
            + world_to_node.front.x * src.up.z
            + world_to_node.up.x * src.up.y;

        dst.up.y = world_to_node.right.y * src.up.x
            + world_to_node.front.y * src.up.z
            + world_to_node.up.y * src.up.y;

        dst.up.z = world_to_node.right.z * src.up.x
            + world_to_node.front.z * src.up.z
            + world_to_node.up.z * src.up.y;

        dst.front.x = world_to_node.right.x * src.front.x
            + world_to_node.front.x * src.front.z
            + world_to_node.up.x * src.front.y;

        dst.front.y = world_to_node.right.y * src.front.x
            + world_to_node.front.y * src.front.z
            + world_to_node.up.y * src.front.y;

        dst.front.z = world_to_node.right.z * src.front.x
            + world_to_node.front.z * src.front.z
            + world_to_node.up.z * src.front.y;

        // position
        dst.position.x = world_to_node.right.x * src.position.x
            + world_to_node.front.x * src.position.z
            + world_to_node.up.x * src.position.y
            + world_to_node.position.x;

        dst.position.y = world_to_node.right.y * src.position.x
            + world_to_node.front.y * src.position.z
            + world_to_node.up.y * src.position.y
            + world_to_node.position.y;

        dst.position.z = world_to_node.right.z * src.position.x
            + world_to_node.front.z * src.position.z
            + world_to_node.up.z * src.position.y
            + world_to_node.position.z;

        inRange = (unsigned int (__thiscall*)(ST3D_Light*,void*))(getClassFunctionAddress ((DWORD*)inst->m_light,18));
        void* spherePtr = (void*)((char*)self->m_mesh + 0xD4);
        unsigned int resultRange = inRange (inst->m_light,spherePtr);
        if(resultRange) {
            //this is ugly! I will replace drawlight to my own function later
            unsigned char resultDraw;
            __asm {
                mov     eax,self            // this
                mov     edx,D3DDevice9      // device
                mov     ecx,lightingMaterial// material

                push    inst            // arg3 (first push)
                push    vgi              // arg2
                push    numFaces             // arg1

                call    ST3D_Dot3_MeshVB_DrawLight_New_0       // callee RET 0x0C (clears 3 pushes)

                mov     resultDraw,al           // AL holds return value
            }
            setBlendingMode (ST3dDevice,2,2,0,1); // ONE, ONE
        }
    }

    //
    // Stage 2: Base texture pass
    //
    setBlendingMode (ST3dDevice,9,1,0,1);
    //guess tex[0]=diffuse tex[1]=normal
    setTexture (ST3dDevice,(*textures)[0],0);
    //Stage=0 Type=D3DTSS_COLOROP(1) value=D3DTOP_SELECTARG1(2)
    TST3D_DeviceDirectX8_SetTextureStageState (ST3dDevice,0,1,2);
    //Stage=1 Type=D3DTSS_COLOROP(1) value=D3DTOP_DISABLE(1)
    TST3D_DeviceDirectX8_SetTextureStageState (ST3dDevice,1,1,1);

    D3DDevice9->DrawIndexedPrimitive (D3DPT_TRIANGLELIST,0,0,vgi->num_vertices,0,numFaces);

    //
    // Optional alpha pass
    //
    if(*(int*)((char*)self->m_mesh + 300) & 4) {
        setBlendingMode (ST3dDevice,5,6,0,1);

        setSpecularEnable (ST3dDevice,0);
        setDiffuseEnable (ST3dDevice,0);

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

    // return sm_faces_remaining;
}
