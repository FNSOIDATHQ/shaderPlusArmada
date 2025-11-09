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

enum ST3D_TextureAddress : int32_t
{
    ST3D_TextureAddress_Wrap   = 1,
    ST3D_TextureAddress_Mirror = 2,
    ST3D_TextureAddress_Clamp  = 3,
    ST3D_TextureAddress_Border = 4,
};

enum ST3D_PixelFormat : int32_t
{
    ST_PIXELFORMAT_INDEX8    = 0,
    ST_PIXELFORMAT_RGB332    = 1,
    ST_PIXELFORMAT_RGB565    = 2,
    ST_PIXELFORMAT_RGB555    = 3,
    ST_PIXELFORMAT_RGB888    = 4,
    ST_PIXELFORMAT_XRGB8888  = 5,
    ST_PIXELFORMAT_ARGB4444  = 6,
    ST_PIXELFORMAT_ARGB1555  = 7,
    ST_PIXELFORMAT_ARGB8888  = 8,
    ST_PIXELFORMAT_LAST      = 9,
    ST_PIXELFORMAT_UNDEFINED = 0xFF
};

struct ST3D_Colour
{
    float red;
    float green;
    float blue;
};

struct ST3D_DeviceTexture
{
    void* placeholder0;      // 0x00
    void* d3dTexturePtr;     // 0x04
};

struct ST3D_Texture
{
    void*               vtable;               // +0x00
    char                _padding00[0x18 - 4]; // +0x04 ~ +0x17

    unsigned int        m_flags;              // +0x18
    int                 m_width;              // +0x1C
    int                 m_height;             // +0x20
    unsigned char*      m_pixels;             // +0x24
    unsigned int        m_stride;             // +0x28
    unsigned char       m_colour_key_enabled; // +0x2C
    unsigned char       _padding2D[3];        // +0x2D~2F

    ST3D_Colour         m_colour_key;         // +0x30

    int                 m_num_mipmap_levels;  // +0x3C

    // 实际关键点：每个设备都有一个 ST3D_DeviceTexture
    ST3D_DeviceTexture* m_device_texture[2];  // +0x40 (2 个指针)

    unsigned int        m_largest_mipmap_to_use; // +0x48
    ST3D_PixelFormat    m_format_override;       // +0x4C
    ST3D_TextureAddress m_address_u;             // +0x50
    ST3D_TextureAddress m_address_v;             // +0x54
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

    DLL_EXPORT int __fastcall dot3MeshVBRender9Programmable (int,int,int,float*,int,DWORD**);
    DLL_EXPORT int __fastcall dot3MeshVBRender9 (int,int,int,float*,int,DWORD**);
    DLL_EXPORT void dot3MeshVBDrawLight9(ST3D_Dot3_MeshVB*,IDirect3DDevice9*,float*,ST3D_LightInstance*,VertexGroup_Info*,unsigned int);
    DLL_EXPORT int64_t __stdcall createShader9 (DWORD*,UINT*);
    DLL_EXPORT int* __stdcall compileHLSLShader9 (const int*);

#ifdef __cplusplus
}
#endif

#endif // __DX9PLUS_H__
