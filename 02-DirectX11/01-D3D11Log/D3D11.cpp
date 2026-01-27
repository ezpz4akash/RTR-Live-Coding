// headers
#include <d3d11.h> // ~ <gl/gl.h>
#include <math.h>
#include <stdio.h>

// libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// main
int main(void)
{
    void printDXInfo(void);
    printDXInfo();
    return (0);
}

void printDXInfo(void)
{
    // local variable declarations
    IDXGIFactory *pIDXGIFactory = NULL;
    IDXGIAdapter *pIDXGIAdapter = NULL; // ~ physical graphics device

    DXGI_ADAPTER_DESC dxgiAdapterDesc;
    HRESULT hr;
    char str[255];

    // code

    // coInitialize() and coCreateInstance() is internally called by
    // CreateDXGIFactory() method
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pIDXGIFactory);
    if (FAILED(hr))
    {
        printf("CreateDXGIFactory() failed..\n");
        goto CLEANUP;
    }

    // enumerate all compatible adapters
    if (pIDXGIFactory->EnumAdapters(0, &pIDXGIAdapter) == DXGI_ERROR_NOT_FOUND)
    {
        printf("Cannot find DXGIAdapter..\n");
        goto CLEANUP;
    }

    // fetch adapter descre
    ZeroMemory((void *)&dxgiAdapterDesc, sizeof(DXGI_ADAPTER_DESC));
    hr = pIDXGIAdapter->GetDesc(&dxgiAdapterDesc);
    if (FAILED(hr))
    {
        printf("GetDesc() failed..\n");
        goto CLEANUP;
    }

    // convert wide char string to ansi string
    WideCharToMultiByte(CP_ACP, // ANSI CODE PAGE
                        0,
                        dxgiAdapterDesc.Description, // source string
                        255,                         // size of input string
                        str,                         // destination buffer
                        255,                         // size of destination buffer
                        NULL, NULL);

    printf("GPU      : %s\n", str);
    printf("GPU VRAM : %I64d bytes\n",
           (__int64)dxgiAdapterDesc.DedicatedVideoMemory);
    printf("GPU VRAM : %d GB\n", (int)ceil(dxgiAdapterDesc.DedicatedVideoMemory /
                                           1024.0 / 1024.0 / 1024.0));

CLEANUP:
    if (pIDXGIAdapter)
    {
        pIDXGIAdapter->Release();
        pIDXGIAdapter = NULL;
    }

    if (pIDXGIFactory)
    {
        pIDXGIFactory->Release();
        pIDXGIFactory = NULL;
    }
}