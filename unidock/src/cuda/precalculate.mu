// Native-MUSA translation unit wrapper.  The production implementation stays
// in precalculate.cu so CUDA and MUSA builds share the exact engine source and
// its compiler-fix boundary.
#undef __noinline__
#include "precalculate.cu"
