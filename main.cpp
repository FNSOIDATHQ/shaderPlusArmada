#include "main.h"

//dx version control
int dxVersion;

//FO function
int (__stdcall* D3MVB_R)(int,int,int,DWORD**);
int (__stdcall* DR)(int,int,int,DWORD**);

//Armada function
int* (*CD3MVB)(const int*);
int64_t* (__thiscall* ST3D_DeviceDirectX8_CreateShader)(void*,UINT*,UINT*);
UINT (__stdcall* getShaderHandle)(int);

BOOL APIENTRY DllMain (HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
    //only hook when dll is attaching
    if(fdwReason == DLL_PROCESS_ATTACH) {
        // return activate();
    }
    else {
        //do nothing
    }

    return TRUE; // succesful
}

bool __stdcall activate () {
    // MessageBoxA(0, "load shader+", "test", MB_OK | MB_ICONINFORMATION);

    //load hook tools
    loadTools ();

    //load minHook
    int (*init)();
    int (*createHook)(LPVOID,LPVOID,LPVOID*);
    int (*enableHook)(LPVOID);

    HINSTANCE minHook = LoadLibrary (".\\dll\\MinHook.x86.dll");
    if(minHook == NULL) {
        MessageBoxA (0,"can not load minHook!","test",MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }

    if(
        hookJMP == NULL || hookVTable == NULL || writeVarToAddress == NULL || writeVarToAddressP == NULL || getClassFunctionAddress == NULL || getThisPtrFromECX == NULL || moveVarToECX == NULL
        ) {
        MessageBoxA (0,"can not  load functions in hook tools!","test",MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }

    //load minHook, we use its hookTrampoline function, I haven't imp my own version
    init = (int (*)())GetProcAddress (minHook,"MH_Initialize");
    if(init == NULL) {
        MessageBoxA (0,"can not  load MH_Initialize!","test",MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }
    createHook = (int (*)(LPVOID,LPVOID,LPVOID*))GetProcAddress (minHook,"MH_CreateHook");
    if(createHook == NULL) {
        MessageBoxA (0,"can not  load MH_CreateHook!","test",MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }
    enableHook = (int (*)(LPVOID))GetProcAddress (minHook,"MH_EnableHook");
    if(enableHook == NULL) {
        MessageBoxA (0,"can not  load MH_EnableHook!","test",MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }

    init ();

    //read dx version
    int argNum;
    wchar_t** args;
    args = CommandLineToArgvW (GetCommandLineW (),&argNum);
    std::vector<std::wstring> argsV;
    if(args) {
        argsV.assign (args,args + argNum);
        LocalFree (args);
    }
    int index = (argsV.size () > 1) ? 1 : 0;
    //MessageBoxW (0,argsV[index].c_str(),L"test",MB_OK | MB_ICONINFORMATION);
    if(argsV[index] == L"/d3d9" || argsV[index] == L"-d3d9") {
        MessageBoxW (0,L"Now in DX9 Mode",L"Notice",MB_OK | MB_ICONINFORMATION);
        dxVersion = 9;
    }
    else {
        dxVersion = 8;
    }

    //hook code
    if(dxVersion == 8) {
        //======compile shader
        char newVertexShaderPath[] = "shaders\\dx8\\vertex\\vs.nvv";
        writeVarToAddressP ((void*)0x72B580,sizeof (newVertexShaderPath),(void*)newVertexShaderPath);
        createHook ((LPVOID)0x626E50,(LPVOID)compilePixelShader,reinterpret_cast<LPVOID*>(&CD3MVB));
        enableHook ((LPVOID)0x626E50);


        //======create shader
                //this will cause a crash, possiblely because FO using a different call method in its work
                //createHook((LPVOID)0x6226B0, (LPVOID)createPixelShader, reinterpret_cast<LPVOID*>(&ST3D_DeviceDirectX8_CreateShader));
                //enableHook((LPVOID)0x6226B0);

        ST3D_DeviceDirectX8_CreateShader = reinterpret_cast<int64_t * (__thiscall*)(void*,UINT*,UINT*)>(hookVTable ((void**)0x6BC6AC,18,(void*)createShader));

        //======set shader
                //set shader in getShaderHandle
        createHook ((LPVOID)0x62C270,(LPVOID)setPixelShader,reinterpret_cast<LPVOID*>(&getShaderHandle));
        enableHook ((LPVOID)0x62C270);

        //rework drawLight
        //createHook((LPVOID)0x5A9E610C, (LPVOID)drawLight, reinterpret_cast<LPVOID*>(&DR));
        //enableHook((LPVOID)0x5A9E610C);

//======disable shader when enter fixed function pipeline
        //at the end of dot3MeshVB::Render
        //hookTrampoline((void*)6452640,(void*)dot3MeshVBRender);
        createHook ((LPVOID)0x5A9E6320,(LPVOID)dot3MeshVBRender,reinterpret_cast<LPVOID*>(&D3MVB_R));
        enableHook ((LPVOID)0x5A9E6320);

        //at the last if of dot3MeshVB::Render
        //this is a hack, most of time do not hook function at middle of it
        uint8_t jmpInstruction[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };

        const uint32_t relativeAddr = (uint32_t)disablePixelShaderInAlpha - ((uint32_t)0x5A9E67D1 + 5);
        memcpy (jmpInstruction + 1,&relativeAddr,4);

        writeVarToAddress (0x5A9E67D1,sizeof (jmpInstruction),(void*)jmpInstruction);
    }
    else if(dxVersion == 9) {
        //this is basically same as fo's dx9 conversion, just add pixel shader
        //writeNopsToAddress (0x51AA31,0xAB);


        //writeNopsToAddress (0x5AA019D0,0xF);
        //void(*FODX9)()=(void(*)())0x5A9F254C;

        //FODX9();


        // char d3dtest_checkedArguments = 1;
        // writeVarToAddressP ((void*)0x5AA102E0,sizeof (d3dtest_checkedArguments),(void*)d3dtest_checkedArguments);
        // char d3dtest_isD3D9 = 1;
        // writeVarToAddressP ((void*)0x5AA102E4,sizeof (d3dtest_isD3D9),(void*)d3dtest_isD3D9);

        //all of commented code upon is not worked or won't be done for now
        
        //no longer use because D3DXCompileShaderFromFile do not support vs3.0/ps3.0
        // createHook ((LPVOID)0x626E50,(LPVOID)compileHLSLShader9,reinterpret_cast<LPVOID*>(&CD3MVB));
        // enableHook ((LPVOID)0x626E50);

        // hookVTable ((void**)0x6BC6AC,18,(void*)createShader9);

        hookJMP((void*)0x5A9F1CD4,dot3MeshVBRender9Programmable);

        hookVTable ((void**)0x6BC6AC,18,(void*)resetFXShader);
    }

    FreeLibrary (minHook);
    //you shouldn't free it!
    //FreeLibrary (hookTool);

    return TRUE;
}
