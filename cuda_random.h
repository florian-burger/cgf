#ifndef CUDA_RANDOM_H
#define CUDA_RANDOM_H

extern "C" void init_RNG(int n_gaussnumbers, int n_unifnumbers, int seed);
extern "C" void update_RNG();
extern "C" void finalize_RNG();

#endif

