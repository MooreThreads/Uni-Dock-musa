#pragma once

#include <musa_runtime.h>
#if defined(__MUSA__) || defined(__CUDA_ARCH__)
#include <murand_kernel.h>
#endif

// The engine keeps one algorithm/layout implementation while binding its
// narrow runtime and Philox surface to MUSA 5.2.0 in native-MUSA builds.
#define cudaError_t musaError_t
#define cudaStream_t musaStream_t
#define cudaEvent_t musaEvent_t
#define cudaMemcpyKind musaMemcpyKind
#define cudaSuccess musaSuccess
#define cudaMemcpyHostToDevice musaMemcpyHostToDevice
#define cudaMemcpyDeviceToHost musaMemcpyDeviceToHost
#define cudaMemcpyDeviceToDevice musaMemcpyDeviceToDevice
#define cudaGetErrorName musaGetErrorName
#define cudaGetDeviceCount musaGetDeviceCount
#define cudaSetDevice musaSetDevice
#define cudaMemGetInfo musaMemGetInfo
#define cudaDeviceSynchronize musaDeviceSynchronize
#define cudaMalloc musaMalloc
#define cudaFree musaFree
#define cudaMallocHost musaMallocHost
#define cudaFreeHost musaFreeHost
#define cudaMemcpy musaMemcpy
#define cudaMemcpyAsync musaMemcpyAsync
#define cudaMemset musaMemset
#define cudaMemsetAsync musaMemsetAsync
#define cudaMemcpyToSymbol musaMemcpyToSymbol
#define cudaStreamCreate musaStreamCreate
#define cudaStreamDestroy musaStreamDestroy
#define cudaStreamSynchronize musaStreamSynchronize
#define cudaEventCreate musaEventCreate
#define cudaEventRecord musaEventRecord
#define cudaEventSynchronize musaEventSynchronize
#define cudaEventElapsedTime musaEventElapsedTime
#define cudaEventDestroy musaEventDestroy
#define curandStatePhilox4_32_10_t murandStatePhilox4_32_10_t
#define curand_init murand_init
#define curand murand
#define curand_uniform murand_uniform
#define curand_uniform4 murand_uniform4
