/* 
    Build Command : 
    cl /EHsc /I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.8\include" VecAdd.cpp /link "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.8\lib\x64\OpenCL.lib" /MACHINE:x64 /SUBSYSTEM:CONSOLE
*/
#include <stdio.h>
#include <math.h>
#include <CL/opencl.h>

#include "helper_timer.h"
#include <cuda.h>

const int iNumberOfArrayElements = 11444777;

cl_platform_id   oclPlatformID = NULL;
cl_device_id     oclDeviceID = NULL;

cl_context       oclContext = NULL;
cl_command_queue oclCommandQueue = NULL;

cl_program       oclProgram = NULL;
cl_kernel        oclKernel = NULL;

float* hostInput1 = NULL;
float* hostInput2 = NULL;
float* hostOutput = NULL;
float* gold       = NULL;

cl_mem deviceInput1 = NULL;
cl_mem deviceInput2 = NULL;
cl_mem deviceOutput = NULL;

float timeOnCPU = 0.0f;
float timeOnGPU = 0.0f;

const char* oclSourceCode[] = 
{
    "__kernel void vecAddGPU(__global float *in1, __global float *in2, __global float *out, int len)"
    "{"
    "int i = get_global_id(0);"
    "if(i < len)"
    "{"
    "out[i] = in1[i] + in2[i];"
    "}"
    "}"
};

int main(void){
    void cleanup(void);
    void vecAddCPU(const float* arr1, const float* arr2, float* out, int len);
    void fillFloatArrayWithRandomNumbers(float*, int);
    size_t roundGlobalSizeToNearestMultipleOfLocalSize(int local_size, unsigned int global_size);

    int size = iNumberOfArrayElements * sizeof(float);
    cl_int result;

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

    gold = (float*)malloc(size);
    if(gold == NULL){
        printf("gold Memory allocation is failed for hostOutput array.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    fillFloatArrayWithRandomNumbers(hostInput1, iNumberOfArrayElements);
    fillFloatArrayWithRandomNumbers(hostInput2, iNumberOfArrayElements);

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

    oclKernel = clCreateKernel(oclProgram, "vecAddGPU", &result);
    if(result != CL_SUCCESS){
        printf("clCreateKernel() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    deviceInput1 = clCreateBuffer(oclContext, CL_MEM_READ_ONLY, size, NULL, &result);
    if(result != CL_SUCCESS){
        printf("clCreateBuffer() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    deviceInput2 = clCreateBuffer(oclContext, CL_MEM_READ_ONLY, size, NULL, &result);
    if(result != CL_SUCCESS){
        printf("clCreateBuffer() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    deviceOutput= clCreateBuffer(oclContext, CL_MEM_WRITE_ONLY, size, NULL, &result);
    if(result != CL_SUCCESS){
        printf("clCreateBuffer() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clSetKernelArg(oclKernel, 0, sizeof(cl_mem), (void*)&deviceInput1);
    if(result != CL_SUCCESS){
        printf("clSetKernelArg() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clSetKernelArg(oclKernel, 1, sizeof(cl_mem), (void*)&deviceInput2);
    if(result != CL_SUCCESS){
        printf("clSetKernelArg() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clSetKernelArg(oclKernel, 2, sizeof(cl_mem), (void*)&deviceOutput);
    if(result != CL_SUCCESS){
        printf("clSetKernelArg() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clSetKernelArg(oclKernel, 3, sizeof(cl_int), (void*)&iNumberOfArrayElements);
    if(result != CL_SUCCESS){
        printf("clSetKernelArg() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    result = clEnqueueWriteBuffer(oclCommandQueue, deviceInput1, CL_FALSE, 0, size, hostInput1, 0, NULL, NULL);
    if(result != CL_SUCCESS){
        printf("clEnqueueWriteBuffer() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }
    result = clEnqueueWriteBuffer(oclCommandQueue, deviceInput2, CL_FALSE, 0, size, hostInput2, 0, NULL, NULL);
    if(result != CL_SUCCESS){
        printf("clEnqueueWriteBuffer() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Kernel config
    size_t localWorkSize = 256;
    size_t globalWorkSize;
    globalWorkSize = roundGlobalSizeToNearestMultipleOfLocalSize(localWorkSize, iNumberOfArrayElements);

    StopWatchInterface* timer = NULL;
    sdkCreateTimer(&timer);

    sdkStartTimer(&timer);

    result = clEnqueueNDRangeKernel(oclCommandQueue, oclKernel, 1, NULL, &globalWorkSize, &localWorkSize, 0, NULL, NULL);
    if(result != CL_SUCCESS){
        printf("clEnqueueNDRangeKernel() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }
    clFinish(oclCommandQueue);

    sdkStopTimer(&timer);

    timeOnGPU = sdkGetTimerValue(&timer);
    sdkDeleteTimer(&timer);

    result = clEnqueueReadBuffer(oclCommandQueue, deviceOutput, CL_TRUE, 0, size, hostOutput, 0, NULL, NULL);
    if(result != CL_SUCCESS){
        printf("clEnqueueReadBuffer() failed : %d\n", result);
        cleanup();
        exit(EXIT_FAILURE);
    }

    vecAddCPU(hostInput1, hostInput2, gold, iNumberOfArrayElements);
    // Comparison
    const float epsilon = 0.000001f;
    int breakValue = -1;
    bool bAccuracy = true;

    for(int i = 0; i < iNumberOfArrayElements; i++){
        float val1 = gold[i];
        float val2 = hostOutput[i];
        if(fabs(val1 - val2) > epsilon){
            bAccuracy = false;
            breakValue = i;
            break;
        }
    }

    char str[128];
    if(bAccuracy == false){
        sprintf(str, "Comparison of CPU and GPU Vector Addition is not within accuracy of 0.000001f at array index %d.\n", breakValue);
    }
    else{
        sprintf(str, "Comparison of CPU and GPU Vector Addition is within accuracy of 0.000001f\n", breakValue);
    }

    // Output
    printf("==========================\n");
    printf("Array1 begins from 0th index %.6f to %dth index %.6f\n", hostInput1[0], iNumberOfArrayElements - 1, hostInput1[iNumberOfArrayElements - 1]);
    printf("Array2 begins from 0th index %.6f to %dth index %.6f\n", hostInput2[0], iNumberOfArrayElements - 1, hostInput2[iNumberOfArrayElements - 1]);
    printf("==========================\n\n");

    printf("OpenCL Kernel Global Work Size = %zu and Local Work Size = %zu\n\n", globalWorkSize, localWorkSize);
    printf("==========================\n");

    printf("Output begins from 0th index %.6f to %dth index %.6f\n", hostOutput[0], iNumberOfArrayElements - 1, hostOutput[iNumberOfArrayElements - 1]);
    printf("==========================\n");
    // Time taken
    printf("Time taken for Vector Addition on CPU = %.6f\n", timeOnCPU);
    printf("Time taken for Vector Addition on GPU = %.6f\n", timeOnGPU);
    printf("==========================\n");
    printf("%s", str);
    printf("==========================\n");

    cleanup();

    return (0);
}

void fillFloatArrayWithRandomNumbers(float* p, int len){
    const float fscale = 1.0f / (float)RAND_MAX;
    for(int i = 0; i < len; i++){
        p[i] = fscale * rand();
    }
}

void vecAddCPU(const float* arr1, const float* arr2, float* out, int len){ 
    StopWatchInterface* timer = NULL;
    sdkCreateTimer(&timer);
    sdkStartTimer(&timer);

    for(int i = 0; i < len; i++){
        out[i] = arr1[i] + arr2[i];
    }

    sdkStopTimer(&timer);
    timeOnCPU = sdkGetTimerValue(&timer);
    sdkDeleteTimer(&timer);
    timer = NULL;
}

size_t roundGlobalSizeToNearestMultipleOfLocalSize(int local_size, unsigned int global_size){
    unsigned int r = global_size % local_size;
    if(r == 0){
        return (global_size);
    }
    else{
        return (global_size + local_size - r); // 11,444,992
    }
}

void cleanup(void){
    if(deviceOutput){
        clReleaseMemObject(deviceOutput);
        deviceOutput = NULL;
    }

    if(deviceInput2){
        clReleaseMemObject(deviceInput2);
        deviceInput2 = NULL;
    }

    if(deviceInput1){
        clReleaseMemObject(deviceInput1);
        deviceInput1 = NULL;
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

    if(hostOutput){
        free(hostOutput);
        hostOutput = NULL;
    }

    if(hostInput2){
        free(hostInput2);
        hostInput2 = NULL;
    }

    if(hostInput1){
        free(hostInput1);
        hostInput1 = NULL;
    }

    if(gold){
        free(gold);
        gold = NULL;
    }
}

