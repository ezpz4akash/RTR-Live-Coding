// CUDA Kernel
__global__ void sineWaveKernel(float4* pos, unsigned int width, unsigned int height, float time)
{
    // variable declaration
    unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int j = blockIdx.y * blockDim.y + threadIdx.y;

    float u = (float) (i)/ (float) (width);
    float v = (float) (j)/ (float) (height);

    u = u*2.0f - 1.0f;
    v = v*2.0f - 1.0f;
    float frequency = 4.0f;
    float w = sinf(u * frequency + time) * cosf(v * frequency + time) * 0.2;

    pos[j * width + i] = make_float4(u, w, v, 1.0f);
}

void sineWaveKernelStart(float4* pos, unsigned int width, unsigned int height, float time)
{
        //run cuda Kernel
    dim3 dimBlock = dim3(8, 8, 1);
    
    dim3 dimGrid = dim3((int)ceil((int)width / (int)dimBlock.x), (int)ceil((int)height / (int)dimBlock.y), 1);

    sineWaveKernel <<< dimGrid, dimBlock >>> (pos, width, height, time);
}