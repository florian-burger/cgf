#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "global.h"
#include <fftw3.h>
#include "fft.h"

#define pi 3.141592653589793116


/* calculates 1/2i (U - U^+) - Trace( 1/2i (U - U^+) )*ID  */
/* U IS CHANGED!!!*/
void make_traceless(su3* U){

  su3 temp;
  int Usize = sizeof(su3);
  int i, c1;
  complex trace;
  
  for(i=0; i<4*VOLUME; i++){
    
    //copy to local
    memcpy(&(temp), &(U[i]), Usize);
     
     trace.re = (temp[0][0].re + temp[1][1].re + temp[2][2].re)/3.0;
     trace.im = (temp[0][0].im + temp[1][1].im + temp[2][2].im)/3.0;
  
     for(c1=0; c1<3; c1++){
       temp[c1][c1].re -= trace.re;
       temp[c1][c1].im -= trace.im;
     }
     
     //copy back
    memcpy(&(U[i]), &(temp), Usize);
     
  }
}



/* calculated the 4d Fourier transfor of an SU(3) Field*/
void U_fft(su3 * U ,int finvers){/*mit fftw3.lib  finvers = 0 (false) -> normale fft
                                                           finvers != 0 (true) -> inverse fft 
                                              berechnet laut fftw-doku folgendes:
                                               f(k) = sum f(x) e^(-i k x) (FFTW_FORWARD)            
                                               f(x) = sum f(k) e^(+i k x) (FFTW_BACKWARD)             
                                             */
                                             
  int latdims[4];
  latdims[0]=T; latdims[1]=LX; latdims[2]=LY; latdims[3]=LZ;
  
  int mu,c1,c2,x0,x1,x2,x3;
  
  int colorpos;
  int Ufieldpos;
  
  
  fftw_complex *in, *out;
  fftw_plan p;

  in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * VOLUME);
  out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * VOLUME);
  if (finvers == 0){/*false*/
    p = fftw_plan_dft(4, latdims,in, out, FFTW_FORWARD, FFTW_ESTIMATE); /*4d fft planen */
  }
  else{
    p = fftw_plan_dft(4, latdims,in, out, FFTW_BACKWARD, FFTW_ESTIMATE);/*4d fft planen*/
  }

for(mu=0;mu<4;mu++){

  for (c1=0; c1<3; c1++){/*Color1 */
    for (c2=0; c2<3; c2++){/*Color2 */
      printf("Doing FFT of color component (%d, %d)\n",c1,c2);
      /*Feld initialisieren */
      
      for(x0=0; x0<T;x0++){
	for(x1=0; x1<LX;x1++){
	  for(x2=0; x2<LY;x2++){
	    for(x3=0; x3<LZ;x3++){
	      Ufieldpos = x1 + LX*(x2 + LY*(x3 + LZ*x0));
	      in[Ufieldpos][0] = U[4*Ufieldpos+mu][c1][c2].re; /*re*/
	      in[Ufieldpos][1] = U[4*Ufieldpos+mu][c1][c2].im; /*im*/
	    }
	  }
	}
      }
      
      /*fftw starten */
      fftw_execute(p);
      
      
      /*Feld zurückschreiben */
      /* falls finvers -> Normieren auf Volumen */
      double normfactor = 1.0;
      
      if (finvers){
	normfactor = 1.0/(double)VOLUME;
      }
      
      for(x0=0; x0<T;x0++){
	for(x1=0; x1<LX;x1++){
	  for(x2=0; x2<LY;x2++){
	    for(x3=0; x3<LZ;x3++){
	      Ufieldpos = x1 + LX*(x2 + LY*(x3 + LZ*x0));      
	      
	      U[4*Ufieldpos+mu][c1][c2].re = out[Ufieldpos][0]*normfactor ; /*re*/
	      U[4*Ufieldpos+mu][c1][c2].im = out[Ufieldpos][1]*normfactor ; /*im*/	    
	    }
	  }
	}
      }
    }
  }


}//mu

  fftw_destroy_plan(p);
  fftw_free(in); fftw_free(out);

return;
}




    
 /* c.f. Andre Sternbeck Dissertation*/
int zylindercut(int x,int y, int z, int t, double limit){
  double term1, term2;
  double temp;
  
  /* prevent for doubler region > L/2*/
  /*
  if(t>=T/2) return 0;  
  if(x>=LX/2) return 0;
  if(y>=LY/2) return 0;
  if(z>=LZ/2) return 0;
  */
//   if(t>=14) return 0;
//   if(x>=9) return 0;
//   if(y>=9) return 0;
//   if(z>=9) return 0;
   
  temp = ((double) x) / ((double) LX);
  term1 = temp*temp;
  temp = ((double) y) / ((double) LY);
  term1 += temp*temp;
  temp = ((double) z) / ((double) LZ);
  term1 += temp*temp;
  temp = ((double) t) / ((double) T);
  term1 += temp*temp;  
  /* Diagonale: 0.5(1,1,1,1)*/
  temp = 0.5* ((double) x) / ((double) LX);
  term2 = temp;
  temp = 0.5* ((double) y) / ((double) LY);
  term2 += temp;
  temp = 0.5* ((double) z) / ((double) LZ);
  term2 += temp;
  temp = 0.5* ((double) t) / ((double) T);
  term2 += temp;  
  term2 = term2*term2;

  if((term1-term2) <= (limit/( (double)LX * (double)LX ) ) ){
    return(1);
  }
  else{
    return(0);
  }
}


/* specify here which cut is to be applied !!!!!*/
int cut(int x, int y, int z, int t){
  return(zylindercut(x,y,z,t, 1.1));

}
     
  

/* 

  formula (7) of 1108.1735 
  we have the index matching of
  
       0         .....    L-1
  -L/2 + 1       .....    L/2

*/
int init_lattice_momenta(double * sinT, double* sinL){

  int i;
  
  for(i=0;i<LX;i++){
     sinL[i] = 2.0*sin( pi * (i+1-LX/2) / LX );
  }
  
  for(i=0;i<L;i++){
     sinT[i] = 2.0*sin( pi * (i+1-T/2) / T );
  }
  
}




int gluon_propagator(su3* A, char* outfilename, double zylcut_limit){

  double * sinL;
  double * sinT;
  
  sinT = malloc(T*sizeof(double));
  sinL = malloc(LX*sizeof(double));

  /* calculate momenta q_mu*/
  init_lattice_momenta(sinT, sinL);
  
  
  int pos1, pos2;
  int k0,k1,k2,k3, x0,x1,x2,x3,c;
  
  // A(pos1=k) A(pos2=-k) 
  //
   for(k0=0; k0<T/2; k0++){
      for(k1=0; k1<LX/2; k1++){
        for(k2=0; k2<LY/2; k2++){
          for(k3=0; k3<LZ/2; k3++){
            k0  = T/2 + x0;
	    mk0 = T/2 - x0;
	    
          }
        }
      }
    }
  
  
  free(sinL);
  free(sinT);

}





