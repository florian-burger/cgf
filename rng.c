#include <stdio.h>
#include <stdlib.h>


void SetSeed(long x){
  srand48(x);
}


double Random(){
  return(drand48());
}