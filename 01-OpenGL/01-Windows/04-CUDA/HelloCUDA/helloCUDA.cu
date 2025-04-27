#include <stdio.h>
#include <cuda.h>

const int iNumberOfArrayElements = 5;

float* hostInput1 = NULL;
float* hostInput2 = NULL;
float* hostOutput = NULL;

float* deviceInput1 = NULL;
float* deviceInput2 = NULL;
float* deviceOutput = NULL;

//CUDA Kernel
__global__ void vecAddGPU(float *in1, float *in2, float *out, int len){
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    if(i < len){
        out[i] = in1[i] + in2[i];
    }
}

int main(void){
    void cleanup(void);

    int size = iNumberOfArrayElements * sizeof(float);
    cudaError_t result = cudaSuccess;

    hostInput1 = (float*)malloc(size);
    if(hostInput1 == NULL){
        printf("Host Memory allocation is failed for hostInput1 array.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    hostInput2 = (float*)malloc(size);
    if(hostInput2 == NULL){
        printf("Host Memory allocation is failed for hostInput2 array.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    hostOutput = (float*)malloc(size);
    if(hostOutput == NULL){
        printf("Host Memory allocation is failed for hostOutput array.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    hostInput1[0] = 101.0;
    hostInput1[1] = 102.0;
    hostInput1[2] = 103.0;
    hostInput1[3] = 104.0;
    hostInput1[4] = 105.0;

    hostInput2[0] = 201.0;
    hostInput2[1] = 202.0;
    hostInput2[2] = 203.0;
    hostInput2[3] = 204.0;
    hostInput2[4] = 205.0;

    // Device Memory Allocation
    result = cudaMalloc((void**)&deviceInput1, size);
    if(result != cudaSuccess){
        printf("Device Memory Allocation Is Failed For deviceInput1 Array\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = cudaMalloc((void**)&deviceInput2, size);
    if(result != cudaSuccess){
        printf("Device Memory Allocation Is Failed For deviceInput2 Array\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = cudaMalloc((void**)&deviceOutput, size);
    if(result != cudaSuccess){
        printf("Device Memory Allocation Is Failed For deviceOutput Array\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Copy data from host arrays into device arrays
    result = cudaMemcpy(deviceInput1, hostInput1, size, cudaMemcpyHostToDevice);
    if(result != cudaSuccess){
        printf("Host To Device Data Copy Is Failed For deviceInput1 Array\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = cudaMemcpy(deviceInput2, hostInput2, size, cudaMemcpyHostToDevice);
    if(result != cudaSuccess){
        printf("Host To Device Data Copy Is Failed For deviceInput2 Array\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    dim3 dimGrid = dim3(iNumberOfArrayElements, 1, 1);
    dim3 dimBlock = dim3(1, 1, 1);

    /* 
        When you launch a CUDA kernel using <<<>>>, you are launching one grid for that kernel execution.
        <<<numBlocks, threadsPerBlock>>>
            - Defines a single grid execution
            - The grid consists of multiple blocks, each containing multiple threads.

        1D : 
            int global_id = blockIdx.x * blockDim.x + threadIdx.x; (for 1D)

        2D : 
            - As matrix : 
                int global_id_x = blockIdx.x * blockDim.x + threadIdx.x;
                int global_id_y = blockIdx.y * blockDim.y + threadIdx.y;
            - As linear array : 
                int global_id = global_id_y * (gridDim.x * blockDim.x) + global_id_x;

        3D : 
            - As Cube : 
                int global_id_x = blockIdx.x * blockDim.x + threadIdx.x;
                int global_id_y = blockIdx.y * blockDim.y + threadIdx.y;
                int global_id_z = blockIdx.z * blockDim.z + threadIdx.z;
            
            - As linear array : 
                int global_id = global_id_z * (gridDim.y * blockDim.y * gridDim.x * blockDim.x) +
                global_id_y * (gridDim.x * blockDim.x) +
                global_id_x;
    
    */
    vecAddGPU<<<dimGrid, dimBlock>>> (deviceInput1, deviceInput2, deviceOutput, iNumberOfArrayElements);

    result = cudaMemcpy(hostOutput, deviceOutput, size, cudaMemcpyDeviceToHost);
    if(result != cudaSuccess){
        printf("Device To Host Data Copy Is Failed For hostOutput Array\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < iNumberOfArrayElements; i++){
        printf("%f + %f = %f\n", hostInput1[i], hostInput2[i], hostOutput[i]);
    }

    cleanup();
}

void cleanup(void){

    /* 
    
    
    
    
    */
    if(deviceOutput){
        cudaFree(deviceOutput);
        deviceOutput = NULL;
    }

    if(deviceInput2){
        cudaFree(deviceInput2);
        deviceInput2 = NULL;
    }

    if(deviceInput1){
        cudaFree(deviceInput1);
        deviceInput1 = NULL;
    }

    if(hostOutput){
        cudaFree(hostOutput);
        hostOutput = NULL;
    }

    if(hostInput2){
        cudaFree(hostInput2);
        hostInput2 = NULL;
    }

    if(hostInput1){
        cudaFree(hostInput1);
        hostInput1 = NULL;
    }
}

/* 
    ***** CUDA INFORMATION : *****

    Total Number of CUDA supporting GPU Device/Devices on this system : 1

    ===============================
    ***** CUDA DRIVER AND RUNTIME INFORMATION ******
    CUDA Driver Version : 12.8
    CUDA Runtime Version : 12.8
    ===============================
    ***** GPU DEVICE GENERAL INFORMATION ******
    GPU Device Number : 0
    GPU Device Name : NVIDIA GeForce GTX 1650
    GPU Device Computer Capability : 7.5
    GPU Device Type : Discrete
    ===============================
    ***** GPU DEVICE MEMORY INFORMATION ******
    GPU Device Total Memory : 4 GB = 4096 MB = 4294639616 Bytes
    GPU device Constant Memory : 65536 Bytes
    GPU device Shared Memory Per SMProcessor : 49152
    ===============================
    ***** GPU DEVICE MULTIPROCESSOR INFORMATION ******
    GPU device Number Of SMProcessors : 14
    GPU Device Number Of Registers Per SMProcessor : 65536
    GPU Number Of CUDA Cores : 896
    ===============================
    ***** GPU DEVICE THREAD INFORMATION ******
    GPU Device Maximum Number Of Threads Per SMProcessor : 1024
    GPU Device Maximum Number Of Threads Per Block : 1024
    GPU Device Threads In Warp : 32
    GPU Device Maximum Thread Dimensions : (1024, 1024, 64)
    GPU Device Maximum Grid Dimensions : (2147483647, 65535, 65535)
    ===============================
    ***** GPU DEVICE DRIVER INFORMATION ******
    GPU Device has ECC support : Disabled
    GPU Device CUDA Driver Mode (TCC or WDDM) : WDDM (Windows Display Driver Model)
    ===============================
*/