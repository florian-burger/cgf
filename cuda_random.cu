#include <curand.h>
#include "cuda_random.h"

static curandGenerator_t rn_gen;
static int NGAUSS=0, NUNIF=0;


extern "C" void init_RNG(int n_gaussnumbers, int n_unifnumbers, int seed){
 
 
  cudaError_t cudaerr;
  //determine sizes for gauss numbers
  
  printf("Initializing GPU random number generator...\n");
  curandCreateGenerator(&rn_gen, CURAND_RNG_PSEUDO_DEFAULT);
  curandSetPseudoRandomGeneratorSeed(rn_gen, seed);
  
  //determine sizes for gaussian numbers
  printf("No. of gauss random numbers: %d\n", n_gaussnumbers );
  NGAUSS = n_gaussnumbers;
   
  //determine sizes for unif. numbers 
  printf("No. of unif. dist. random numbers: %d\n", n_unifnumbers );
  NUNIF = n_unifnumbers;
   

  //allocate fields for random numbers
  printf("Allocating device memory for random numbers...\n");
  CUDA_SAFE_CALL(cudaMalloc((void **)&dev_rndgauss_field, n_gaussnumbers * sizeof(float)) );
  CUDA_SAFE_CALL(cudaMalloc((void **)&dev_rndunif_field, n_unifnumbers * sizeof(float)));


 // CREATE FIRST RANDOM NUMBERS 
 /* update the random field for gauss numbers*/
    curandGenerateNormal(rn_gen, dev_rndgauss_field, n_gaussnumbers, 0.0f, 1.0f);
    
  /* update the random field for unif. dist. numbers*/
    curandGenerateUniform(rn_gen, dev_rndunif_field, n_unifnumbers);
   
   cudaerr = cudaGetLastError();
   if(cudaerr != cudaSuccess){
     printf("%s\n", cudaGetErrorString(cudaerr)); 
   } 

}





extern "C" void update_RNG(){


 /* update the random field for gauss numbers*/
    curandGenerateNormal(rn_gen, dev_rndgauss_field, NGAUSS, 0.0f, 1.0f);
    
  /* update the random field for unif. dist. numbers*/
    curandGenerateUniform(rn_gen, dev_rndunif_field, NUNIF);
    
}



extern "C" void finalize_RNG(){
  cudaFree(dev_rndgauss_field);
  cudaFree(dev_rndunif_field);
  curandDestroyGenerator(rn_gen);
}





