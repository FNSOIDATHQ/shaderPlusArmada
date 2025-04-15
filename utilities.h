#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <windows.h>
#include <stdint.h>

    //for easier coding
    struct Vector3 {
        float x,y,z;
    };
    struct Matrix {
        Vector3 right,up,front,position;
    };
    struct VertexGroup_Info {
        int vertex_buffer_id;
        UINT8* vertex_buffer_data;
        int index_buffer_id;
        UINT16* index_buffer_data;
        unsigned int num_vertices;
        unsigned int num_triangles;
    };

    //FO function
extern int (__stdcall* D3MVB_R)(int,int,int,DWORD**);
extern int (__stdcall* DR)(int,int,int,DWORD**);

//Armada function
extern int* (*CD3MVB)(const int*);
extern int64_t* (__thiscall* ST3D_DeviceDirectX8_CreateShader)(void*,UINT*,UINT*);
extern UINT (__stdcall* getShaderHandle)(int);

#endif // __UTILITIES_H__

