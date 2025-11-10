#include "dx9plus.h"
// #define DEBUG

// =========================================================
//  External function/parameter pointers
// =========================================================
int* gRenderer = (int*)0x5AA126BC;         // Delphi Renderer instance pointer
DWORD* dword_5A9EFDA0 = (DWORD*)0x5A9EFDA0;  // param passed to Renderer_create
D3DVERTEXELEMENT9* vertexElementsFO = *(D3DVERTEXELEMENT9**)0x5AA49280;

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

int (__thiscall* TexDemandLoad)(ST3D_Texture* thisPtr,int device_index) = (decltype(TexDemandLoad))0x642610;

// Per-light rendering vanilla FO
// unsigned char (__thiscall* ST3D_Dot3_MeshVB_DrawLight_New_0)(
//     void* self,
//     IDirect3DDevice9* d3dDev9,
//     float* lightingMaterial,
//     ST3D_LightInstance* lightData,
//     int vgiAddr,
//     int numFaces)
//     = (decltype(ST3D_Dot3_MeshVB_DrawLight_New_0))0x5A9F1ABC;

static void* ST3D_Dot3_MeshVB_DrawLight_New_0 = (void*)0x5A9F1ABC;

// =========================================================
//  Helper Functions
// =========================================================
static inline Vector3 vec_sub (const Vector3& a,const Vector3& b) {
    return Vector3{ a.x - b.x, a.y - b.y, a.z - b.z };
}
static inline float vec_len (const Vector3& v) {
    return std::sqrt (v.x * v.x + v.y * v.y + v.z * v.z);
}
static inline Vector3 vec_norm (const Vector3& v) {
    float l = vec_len (v);
    if(l > 1e-8f) return Vector3{ v.x / l, v.y / l, v.z / l };
    return Vector3{ 0.f,0.f,0.f };
}

IDirect3DBaseTexture9* GetD3DTexture9FromStorm3DTexture (ST3D_GraphicsEngine* Storm3D,ST3D_Texture* tex) {
    if(!tex)
        return nullptr;

    int idx = Storm3D->m_current_device_index;

    if(TexDemandLoad (tex,idx))
        return nullptr;

    ST3D_DeviceTexture* devTex = tex->m_device_texture[idx];
    if(!devTex)
        return nullptr;

    void* d3dTex = *((void**)devTex + 1);
    if(!d3dTex)
        return nullptr;

    // DX8 -> DX9
    IDirect3DBaseTexture9* baseTex = nullptr;
    ((IUnknown*)d3dTex)->QueryInterface (IID_IDirect3DBaseTexture9,
        (void**)&baseTex);

    return baseTex;
}


//Shader related
struct SDirLight
{
    D3DXVECTOR4 color;
    D3DXVECTOR4 dir;
};
struct SPointLight
{
    D3DXVECTOR3 pos;
    float falloffStart;
    D3DXVECTOR3 color;
    float falloffRange;
};
// 2 dir + 4 point
SDirLight dirLights[2];
SPointLight pointLights[4];

IDirect3DDevice9* D3DDevice9;
LPVOID vsCompiled;
const DWORD* vsCompiledFX;
IDirect3DVertexDeclaration9* vertexDeclaration;
IDirect3DVertexShader9* vertexShader;

ID3DXEffect* fxShader = nullptr;

#ifdef DEBUG
ID3DXFont* g_pFont = nullptr;
#endif

//original armada set shader constant in PreRender function
//vertex shader + pixel shader
int __fastcall dot3MeshVBRender9Programmable (
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


    D3DDevice9 = nullptr;
    getPlatformSpecific (ST3dDevice,3,(int*)&D3DDevice9);
    // MessageBoxA(0, std::to_string((int)D3DDevice9).c_str(), "D3DDevice9 Value", MB_OK | MB_ICONINFORMATION);

    getShaderHandle9 (Storm3D,shaderID);
    //compile shaders from fx file
    if(fxShader == nullptr) {
        // MessageBoxA (nullptr,"Start Compile FX","Notice",MB_OK | MB_ICONINFORMATION);
        LPD3DXBUFFER errorMsgs = nullptr;
        LPCSTR FXPath = "shaders\\dx9\\pbrLite.fx";
        HRESULT result = D3DXCreateEffectFromFileA (D3DDevice9,FXPath,nullptr,nullptr,0,nullptr,&fxShader,&errorMsgs);

        if(result != 0) {
            const char* msg = (const char*)errorMsgs->GetBufferPointer ();
            MessageBoxA (nullptr,msg,"FX Shader Compile Error",MB_OK | MB_ICONERROR);
        }

        fxShader->SetTechnique ("pbrLite");


        D3DVERTEXELEMENT9 a2VertexElements[] =
        {
            // stream offset   type                  method        usage      usageIndex
            {0,  0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,  0},
            {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,    0},
            {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,   0},
            {0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0},
            {0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL,  0},
            {0, 56, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  1},
            D3DDECL_END () // {0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0}
        };
        D3DDevice9->CreateVertexDeclaration (a2VertexElements,&vertexDeclaration);


    }
#ifdef DEBUG
    if(g_pFont == nullptr) {
        D3DXCreateFontA (D3DDevice9,24,0,FW_NORMAL,1,FALSE,
            DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY,DEFAULT_PITCH | FF_DONTCARE,
            "Arial",&g_pFont);
    }
#endif

    //set inputs
    ST3D_Texture* diffuse = (ST3D_Texture*)(*textures)[0];
    IDirect3DBaseTexture9* diffuseTex9 = GetD3DTexture9FromStorm3DTexture (Storm3D,diffuse);
    fxShader->SetTexture ("gDiffuse",diffuseTex9);

    ST3D_Texture* bump = (ST3D_Texture*)(*textures)[1];
    IDirect3DBaseTexture9* bumpTex9 = GetD3DTexture9FromStorm3DTexture (Storm3D,bump);
    fxShader->SetTexture ("gBump",bumpTex9);

    D3DXMATRIX world,view,proj;
    D3DDevice9->GetTransform (D3DTS_WORLD,&world);
    D3DDevice9->GetTransform (D3DTS_VIEW,&view);
    D3DDevice9->GetTransform (D3DTS_PROJECTION,&proj);

    fxShader->SetMatrix ("gWorld",&world);
    fxShader->SetMatrix ("gView",&view);
    fxShader->SetMatrix ("gProj",&proj);

    std::list<ST3D_LightInstance*> activeLightList = Storm3D->m_active_lights;
    int dirIndex = 0;
    for(ST3D_LightInstance* inst : activeLightList) {
        ST3D_Light* L = inst->m_light;

        if(L->m_light_type == ST3D_LIGHT_DIRECTIONAL) {
            dirLights[dirIndex].color.x = inst->m_colour.red;
            dirLights[dirIndex].color.y = inst->m_colour.green;
            dirLights[dirIndex].color.z = inst->m_colour.blue;

            dirLights[dirIndex].dir.x = inst->m_transform.front.x;
            dirLights[dirIndex].dir.y = inst->m_transform.front.y;
            dirLights[dirIndex].dir.z = inst->m_transform.front.z;
            dirIndex++;
        }
        else if(L->m_light_type == ST3D_LIGHT_POINT) {
            //todo
        }
    }
    D3DXHANDLE hDirLights = fxShader->GetParameterByName (nullptr,"dirLights");
    fxShader->SetValue (hDirLights,dirLights,sizeof (dirLights));
    fxShader->SetInt ("NumDirLights",dirIndex);

    D3DXVECTOR4 envColor;
    envColor.x = lightingMaterial[9];
    envColor.y = lightingMaterial[10];
    envColor.z = lightingMaterial[11];
    fxShader->SetVector ("envColor",&envColor);

    ST3D_Camera* camera = Storm3D->m_camera;
    D3DXVECTOR4 cameraPos;
    cameraPos.x = camera->m_camera_to_world.position.x;
    cameraPos.y = camera->m_camera_to_world.position.y;
    cameraPos.z = camera->m_camera_to_world.position.z;
    fxShader->SetVector ("cameraPos",&cameraPos);

    IDirect3DVertexBuffer9* vbObj = getVertexBufferObject (Storm3D,vgi->vertex_buffer_id);
    IDirect3DIndexBuffer9* ibObj = getIndexBufferObject (Storm3D,vgi->index_buffer_id);


    //Rendering
    UINT passes;
    fxShader->Begin (&passes,0);
    for(UINT p = 0; p < passes; ++p) {
        fxShader->BeginPass (p);

        D3DDevice9->SetStreamSource (0,vbObj,0,68);
        D3DDevice9->SetVertexDeclaration (vertexDeclaration);
        D3DDevice9->SetIndices (ibObj);
        D3DDevice9->DrawIndexedPrimitive (D3DPT_TRIANGLELIST,0,0,vgi->num_vertices,0,numFaces);

        fxShader->EndPass ();
    }
    fxShader->End ();

#ifdef DEBUG
    D3DDevice9->SetRenderState (D3DRS_ZENABLE,FALSE);
    
    std::string wtnChecker = "cameraMatrix=\n";

    auto appendVec = [&](const char* name,const Vector3& v) {
        wtnChecker += name;
        wtnChecker += "(";
        wtnChecker += std::to_string (v.x) + ", ";
        wtnChecker += std::to_string (v.y) + ", ";
        wtnChecker += std::to_string (v.z) + ")";
        wtnChecker += " \n";
        };

    appendVec ("right",camera->m_camera_to_world.right);
    appendVec ("up",camera->m_camera_to_world.up);
    appendVec ("front",camera->m_camera_to_world.front);
    appendVec ("position",camera->m_camera_to_world.position);

    RECT rect = { 20, 200, 500, 1000 };
    g_pFont->DrawTextA (
        nullptr,
        wtnChecker.c_str (),
        -1,
        &rect,
        DT_LEFT | DT_TOP,
        D3DCOLOR_ARGB (255,255,255,255)
    );

    D3DDevice9->SetRenderState (D3DRS_ZENABLE,TRUE);
#endif
}


//call every frame

//This is old pipeline, fixed rendering pipeline + simple vertex shader
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


    D3DDevice9 = nullptr;
    getPlatformSpecific (ST3dDevice,3,(int*)&D3DDevice9);
    // MessageBoxA(0, std::to_string((int)D3DDevice9).c_str(), "D3DDevice9 Value", MB_OK | MB_ICONINFORMATION);

    //compile shaders from fx file
    if(fxShader == nullptr) {
        // MessageBoxA (nullptr,"Start Compile FX","Notice",MB_OK | MB_ICONINFORMATION);
        LPD3DXBUFFER errorMsgs = nullptr;
        LPCSTR FXPath = "shaders\\dx9\\dot3.fx";
        HRESULT result = D3DXCreateEffectFromFileA (D3DDevice9,FXPath,nullptr,nullptr,0,nullptr,&fxShader,&errorMsgs);

        if(result != 0) {
            const char* msg = (const char*)errorMsgs->GetBufferPointer ();
            MessageBoxA (nullptr,msg,"FX Shader Compile Error",MB_OK | MB_ICONERROR);
        }

        fxShader->SetTechnique ("dot3");

        D3DXHANDLE tech = fxShader->GetTechniqueByName ("dot3");
        D3DXHANDLE pass = fxShader->GetPass (tech,0);

        D3DXPASS_DESC desc;
        fxShader->GetPassDesc (pass,&desc);

        vsCompiledFX = desc.pVertexShaderFunction;

        // std::string fxChecker = "getCompiledFX="+*desc.pVertexShaderFunction;
        // MessageBoxA (nullptr,fxChecker.c_str(),"Notice",MB_OK | MB_ICONINFORMATION);
    }


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
    D3DDevice9->SetVertexDeclaration (vertexDeclaration);
    D3DDevice9->SetVertexShader (vertexShader);

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
            // unsigned char resultDraw;
            // __asm {
            //     mov     eax,self            // this
            //     mov     edx,D3DDevice9      // device
            //     mov     ecx,lightingMaterial// material

            //     push    inst            // arg3 (first push)
            //     push    vgi              // arg2
            //     push    numFaces             // arg1

            //     call    ST3D_Dot3_MeshVB_DrawLight_New_0       // callee RET 0x0C (clears 3 pushes)

            //     mov     resultDraw,al           // AL holds return value
            // }

            dot3MeshVBDrawLight9 (self,D3DDevice9,lightingMaterial,inst,vgi,numFaces);
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

// Per-light rendering
void dot3MeshVBDrawLight9 (
    ST3D_Dot3_MeshVB* self,
    IDirect3DDevice9* pD3DDevice9,
    float* lightingMaterial,
    ST3D_LightInstance* li,
    VertexGroup_Info* info,
    unsigned int numFaces) {

    //
    // 1. set light dir to c6
    //
    Vector3 lightDir = { 0,0,0 };
    ST3D_Light* L = li->m_light;

    SPHERE* sphere = reinterpret_cast<SPHERE*>((char*)self->m_mesh + 0xD4);

    if(L->m_light_type == ST3D_LIGHT_DIRECTIONAL) {
        Vector3 n = vec_norm (L->m_light_to_node.front);
        lightDir = Vector3{ -n.x, -n.y, -n.z };
    }
    else if(L->m_light_type == ST3D_LIGHT_POINT) {
        lightDir = vec_sub (L->m_light_to_node.position,sphere->origin);
    }
    else {
        return;
    }

    float c6[4] = { lightDir.x, lightDir.y, lightDir.z, 0.0f };
    pD3DDevice9->SetVertexShaderConstantF (6,c6,1);

    //
    // 2. calculate and set light color to TFACTOR
    //
    ST3D_Colour lightColor;
    {
        //lightingMaterial->diffuse
        const float lm_r = lightingMaterial[9];
        const float lm_g = lightingMaterial[10];
        const float lm_b = lightingMaterial[11];

        lightColor.red = lm_r * li->m_colour.red;
        lightColor.green = lm_g * li->m_colour.green;
        lightColor.blue = lm_b * li->m_colour.blue;
    }

    // 2.1 point light attenuation
    if(L->m_light_type == ST3D_LIGHT_POINT) {
        float d = vec_len (vec_sub (L->m_light_to_node.position,sphere->origin));
        const float start = L->m_falloff_start;
        const float range = L->m_falloff_range;

        //out of range
        if(d >= start + range) {
            return;
        }

        if(d >= start) {
            float att = 1.0f - (d - start) / range;
            lightColor.red *= att;
            lightColor.green *= att;
            lightColor.blue *= att;
        }
    }

    // 2.2 set to D3DRS_TEXTUREFACTOR
    D3DCOLOR tf = D3DCOLOR_COLORVALUE (lightColor.red,lightColor.green,lightColor.blue,1.0f);
    pD3DDevice9->SetRenderState (D3DRS_TEXTUREFACTOR,tf);

    // 3. draw light final
    pD3DDevice9->DrawIndexedPrimitive (D3DPT_TRIANGLELIST,0,0,info->num_vertices,0,numFaces);
}


int64_t __stdcall createShader9 (DWORD* pShader,UINT* pDeclaration) {
    // MessageBoxA(0, "createShader", "test", MB_OK | MB_ICONINFORMATION);

    UINT thisPtr;
    __asm {
        call getThisPtrFromECX
        mov thisPtr,eax
    }

    D3DDevice9->CreateVertexDeclaration (vertexElementsFO,&vertexDeclaration);
    HRESULT VSresult = D3DDevice9->CreateVertexShader ((DWORD*)vsCompiledFX,&vertexShader);
    // this will choose to use old dot3_directional9.nvv
    // HRESULT VSresult = D3DDevice9->CreateVertexShader ((DWORD*)pShader,&vertexShader);

    // HRESULT PSresult = D3DDevice9->CreatePixelShader ((DWORD*)psCompiled,&psHandle);

    if(VSresult != 0) {
        MessageBoxA (0,"error create vertex shader!","test",MB_OK | MB_ICONINFORMATION);
    }
    // if(PSresult != 0) {
    //     MessageBoxA (0,"error create pixel shader!","test",MB_OK | MB_ICONINFORMATION);
    // }
    return (int64_t)pShader;
}


//no longer use because D3DXCompileShaderFromFile do not support vs3.0/ps3.0
int* __stdcall compileHLSLShader9 (const int* mesh) {

    //original function:ST3D_CreateDot3MeshVB(ST3D_Mesh const *)
    int* meshOut = CD3MVB (mesh);

    //compile shader from file
    LPCSTR vertexShaderPath = "shaders\\dx9\\vs.hlsl";
    DWORD flag = 0;

    LPD3DXBUFFER compiledShader = nullptr;
    LPD3DXBUFFER errorMsgs = nullptr;
    LPD3DXCONSTANTTABLE constantTable;

    HRESULT result = D3DXCompileShaderFromFileA (
        vertexShaderPath,
        nullptr,                 // macro definitions
        nullptr,                 // include handler
        "main",                  // VS Entry point
        "vs_2_0",                // Shader model
        flag,
        &compiledShader,
        &errorMsgs,
        &constantTable
    );


    if(result != 0) {
        const char* msg = (const char*)errorMsgs->GetBufferPointer ();
        MessageBoxA (nullptr,msg,"Vertex Shader Compile Error",MB_OK | MB_ICONERROR);
    }
    else {
        vsCompiled = compiledShader->GetBufferPointer ();
    }

    return meshOut;
}


//this is a hack
int64_t __stdcall resetFXShader (DWORD* pShader,UINT* pDeclaration) {
    // MessageBoxA(0, "createShader", "test", MB_OK | MB_ICONINFORMATION);

    UINT thisPtr;
    __asm {
        call getThisPtrFromECX
        mov thisPtr,eax
    }

    if(fxShader!=nullptr){
        fxShader->Release();
    }
    fxShader=nullptr;
    return (int64_t)pShader;
}