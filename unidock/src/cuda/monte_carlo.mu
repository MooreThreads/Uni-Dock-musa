// Native-MUSA translation unit wrapper.  The production implementation stays
// in monte_carlo.cu so CUDA and MUSA builds share the exact engine source and
// its compiler-fix boundary.
#undef __noinline__
#include "monte_carlo.cu"
