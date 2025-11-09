#ifndef __DX9PLUS_H__
#define __DX9PLUS_H__

#include "loadHookTool.h"
//DX library
//from dx9.0c sdk
#include <d3d9.h>
#include <d3dx9.h>

// ---------------------------------------------------------
// Global engine structures
// ---------------------------------------------------------

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

#ifdef __cplusplus
extern "C"
{
#endif

    DLL_EXPORT int __fastcall dot3MeshVBRender9 (int,int,int,float*,int,DWORD**);
    DLL_EXPORT void dot3MeshVBDrawLight9(ST3D_Dot3_MeshVB*,IDirect3DDevice9*,float*,ST3D_LightInstance*,VertexGroup_Info*,unsigned int);

#ifdef __cplusplus
}
#endif

#endif // __DX9PLUS_H__
