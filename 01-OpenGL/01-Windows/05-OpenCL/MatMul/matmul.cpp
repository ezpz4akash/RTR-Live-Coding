/* 
    Build Command : 
    cl /EHsc /I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.8\include" matmul.cpp /link "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.8\lib\x64\OpenCL.lib" /MACHINE:x64 /SUBSYSTEM:CONSOLE
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <CL/opencl.h>
#include "helper_timer.h"

#define BLOCK_WIDTH 256

// global variables
cl_platform_id oclPlatformID;
cl_device_id oclDeviceID;

cl_context oclContext;
cl_command_queue oclCommandQueue;

cl_program oclProgram;
cl_kernel oclKernel;

int* hostA = NULL;
int* hostB = NULL;
int* hostC = NULL;
int* gold  = NULL;

cl_mem deviceA = NULL;
cl_mem deviceB = NULL;
cl_mem deviceC = NULL;

float timeOnCPU = 0.0f;
float timeOnGPU = 0.0f;

const char* oclSourceCode[] = 
{
    "__kernel void matMulGPU(__global int *A, __global int *B, __global int *C, int numARows, int numAColumns, int numBColumns, int numCColumns)"
    "{"
    "int row = get_global_id(0);"
    "int column = get_global_id(1);"
    "if((row < numARows) && (column < numBColumns))"
    "{"
    "int value = 0;"
    "for(int k = 0; k < numAColumns; k++)"
    "{"
    "int a = A[row * numAColumns + k];"
    "int b = B[k * numBColumns + column];"
    "value += (a * b);"
    "}"
    "C[row * numCColumns + column] = value;"
    "}"
    "}"
};

int main(void){
    void InitA(int *data, int, int);
    void InitB(int *data, int, int);
    void matMulCPU(int*, int*, int*, int, int, int, int);
    void cleanup(void);

    int numARows = BLOCK_WIDTH;
    int numAColumns = BLOCK_WIDTH;
    int numBRows = BLOCK_WIDTH;
    int numBColumns = BLOCK_WIDTH;

    int numCRows = numARows;
    int numCColumns = numBColumns;

    int numGoldRows = numARows;
    int numGoldColumns = numBColumns;

    int sizeA = numARows * numAColumns * sizeof(int);
    int sizeB = numBRows * numBColumns * sizeof(int);
    int sizeC = numCRows * numCColumns * sizeof(int);
    int sizeGold = numGoldRows * numGoldColumns * sizeof(int);

    cl_int result;

    hostA = (int*)malloc(sizeA);
    if(hostA == NULL){
        printf("Host Memory allocation is failed for hostA Matrix.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    hostB = (int*)malloc(sizeB);
    if(hostB == NULL){
        printf("Host Memory allocation is failed for hostB Matrix.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    hostC = (int*)malloc(sizeC);
    if(hostC == NULL){
        printf("Host Memory allocation is failed for hostC Matrix.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    gold = (int*)malloc(sizeGold);
    if(gold == NULL){
        printf("Host Memory allocation is failed for gold Matrix.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Printing matrix dimensions and sizes
    printf("The Dimensions of Matrix 'hostA' Are : %d x %d\n", numARows, numAColumns);
    printf("The Dimensions of Matrix 'hostB' Are : %d x %d\n", numBRows, numBColumns);
    printf("The Dimensions of Matrix 'hostC' Are : %d x %d\n", numCRows, numCColumns);
    printf("The Dimensions of Matrix 'gold' Are : %d x %d\n", numGoldRows, numGoldColumns);

    printf("Size of matrix hostA = %d\n", sizeA);
    printf("Size of matrix hostB = %d\n", sizeB);
    printf("Size of matrix hostC = %d\n", sizeC);
    printf("Size of matrix gold = %d\n", sizeGold);

    // Fill source matrices
    InitA(hostA, numARows, numAColumns);
    InitB(hostB, numBRows, numBColumns);

    result = clGetPlatformIDs(1, &oclPlatformID, NULL);
    if(result != CL_SUCCESS){
        printf("clGetPlatformIDs() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clGetDeviceIDs(oclPlatformID, CL_DEVICE_TYPE_GPU, 1, &oclDeviceID, NULL);
    if(result != CL_SUCCESS){
        printf("clGetDeviceIDs() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    oclContext = clCreateContext(NULL, 1, &oclDeviceID, NULL, NULL, &result);
    if(result != CL_SUCCESS){
        printf("clCreateContext() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    oclCommandQueue = clCreateCommandQueueWithProperties(oclContext, oclDeviceID, 0, &result);
    if(result != CL_SUCCESS){
        printf("clCreateCommandQueue() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    oclProgram = clCreateProgramWithSource(oclContext, 1, (const char**)&oclSourceCode, NULL, &result);
    if(result != CL_SUCCESS){
        printf("clCreateProgramWithSource() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clBuildProgram(oclProgram, 0, NULL, NULL, NULL, NULL);
    if(result != CL_SUCCESS){
        size_t len;
        char buffer[2048];
        clGetProgramBuildInfo(oclProgram, oclDeviceID, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("Program Build Log : %s\n", buffer);
        printf("clBuildProgram() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    oclKernel = clCreateKernel(oclProgram, "matMulGPU", &result);
    if(result != CL_SUCCESS){
        printf("clCreateKernel() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    deviceA = clCreateBuffer(oclContext, CL_MEM_READ_ONLY, sizeA, NULL, &result);
    if(result != CL_SUCCESS){
        printf("clCreateBuffer() failed for 1st input matrix : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    deviceB = clCreateBuffer(oclContext, CL_MEM_READ_ONLY, sizeB, NULL, &result);
    if(result != CL_SUCCESS){
        printf("clCreateBuffer() failed for 2nd input matrix  : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    deviceC= clCreateBuffer(oclContext, CL_MEM_WRITE_ONLY, sizeC, NULL, &result);
    if(result != CL_SUCCESS){
        printf("clCreateBuffer() failed for 3rd input matrix  : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clSetKernelArg(oclKernel, 0, sizeof(cl_mem), (void*)&deviceA);
    if(result != CL_SUCCESS){
        printf("clSetKernelArg() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clSetKernelArg(oclKernel, 1, sizeof(cl_mem), (void*)&deviceB);
    if(result != CL_SUCCESS){
        printf("clSetKernelArg() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clSetKernelArg(oclKernel, 2, sizeof(cl_mem), (void*)&deviceC);
    if(result != CL_SUCCESS){
        printf("clSetKernelArg() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clSetKernelArg(oclKernel, 3, sizeof(cl_int), (void*)&numARows);
    if(result != CL_SUCCESS){
        printf("clSetKernelArg() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clSetKernelArg(oclKernel, 4, sizeof(cl_int), (void*)&numAColumns);
    if(result != CL_SUCCESS){
        printf("clSetKernelArg() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clSetKernelArg(oclKernel, 5, sizeof(cl_int), (void*)&numBColumns);
    if(result != CL_SUCCESS){
        printf("clSetKernelArg() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clSetKernelArg(oclKernel, 6, sizeof(cl_int), (void*)&numCColumns);
    if(result != CL_SUCCESS){
        printf("clSetKernelArg() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clEnqueueWriteBuffer(oclCommandQueue, deviceA, CL_FALSE, 0, sizeA, hostA, 0, NULL, NULL);
    if(result != CL_SUCCESS){
        printf("clEnqueueWriteBuffer() failed for 1st input matrix: %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }
    result = clEnqueueWriteBuffer(oclCommandQueue, deviceB, CL_FALSE, 0, sizeB, hostB, 0, NULL, NULL);
    if(result != CL_SUCCESS){
        printf("clEnqueueWriteBuffer() failed for 2nd input matrix: %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    //kernel config
    size_t globalWorkSize[2];
    globalWorkSize[0] = BLOCK_WIDTH;
    globalWorkSize[1] = BLOCK_WIDTH;

    StopWatchInterface* timer = NULL;
    sdkCreateTimer(&timer);

    sdkStartTimer(&timer);

    result = clEnqueueNDRangeKernel(oclCommandQueue, oclKernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);
    if(result != CL_SUCCESS){
        printf("clEnqueueNDRangeKernel() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }
    clFinish(oclCommandQueue);

    sdkStopTimer(&timer);

    timeOnGPU = sdkGetTimerValue(&timer);
    sdkDeleteTimer(&timer);

    result = clEnqueueReadBuffer(oclCommandQueue, deviceC, CL_TRUE, 0, sizeC, hostC, 0, NULL, NULL);
    if(result != CL_SUCCESS){
        printf("clEnqueueReadBuffer() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    matMulCPU(hostA, hostB, gold, numARows, numAColumns, numBColumns, numCColumns);

    // Cmoparison
    int breakValue = -1;
    bool bAccuracy = true;

    for(int i = 0; i < numCRows * numCColumns; i++){
        int val1 = gold[i];
        int val2 = hostC[i];
        if(val1 != val2){
            bAccuracy = false;
            breakValue = i;
            break;
        }
    }

    char str[128];
    if(bAccuracy == false){
        sprintf(str, "Comparison of CPU and GPU Matrix Multiplication is not accurate at index %d.\n", breakValue);
    }
    else{
        sprintf(str, "Comparison of CPU and GPU Matrix Multiplication is Accurate\n");
    }

    // Time taken
    printf("Time taken for Matrix Multiplication on CPU = %.6f\n", timeOnCPU);
    printf("Time taken for Matrix Multiplication on GPU = %.6f\n", timeOnGPU);
    printf("==========================\n");
    printf("%s", str);
    printf("==========================\n");
}

void InitA(int *data, int row, int col){
    int num = 1;
    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
            *(data+ i * col + j) = num;
            num++;
        }
    }
}

void InitB(int *data, int row, int col){
    int num = BLOCK_WIDTH;
    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
            *(data+ i * col + j) = num;
            num--;
        }
    }
}

void matMulCPU(int *A, int *B, int *C, int numARows, int numAColumns, int numBColumns, int numCColumns){
    StopWatchInterface* timer = NULL;
    sdkCreateTimer(&timer);
    sdkStartTimer(&timer);

    for(int i = 0; i < numARows; i++){
        for(int j = 0; j < numBColumns; j++){
            int value = 0;
            for(int k = 0; k < numAColumns; k++){
                int a = A[i * numAColumns + k];
                int b = B[k * numBColumns + j];
                value += a * b;
            }
            C[i * numCColumns + j] = value;
        }
    }

    sdkStopTimer(&timer);
    timeOnCPU = sdkGetTimerValue(&timer);
    sdkDeleteTimer(&timer);
    timer = NULL;
}

void cleanup(void){
    if(deviceC){
        clReleaseMemObject(deviceC);
        deviceC = NULL;
    }
    if(deviceB){
        clReleaseMemObject(deviceB);
        deviceB = NULL;
    }
    if(deviceA){
        clReleaseMemObject(deviceA);
        deviceA = NULL;
    }

    if(oclKernel){
        clReleaseKernel(oclKernel);
        oclKernel = NULL;
    }

    if(oclProgram){
        clReleaseProgram(oclProgram);
        oclProgram = NULL;
    }

    if(oclCommandQueue){
        clReleaseCommandQueue(oclCommandQueue);
        oclCommandQueue = NULL;
    }

    if(oclContext){
        clReleaseContext(oclContext);
        oclContext = NULL;
    }

    if(gold){
        free(gold);
        gold = NULL;
    }

    if(hostC){
        free(hostC);
        hostC = NULL;
    }
    if(hostB){
        free(hostB);
        hostB = NULL;
    }
    if(hostA){
        free(hostA);
        hostA = NULL;
    }
}