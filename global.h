#ifndef GLOBAL_H
#define GLOBAL_H




#if defined MAIN_PROGRAM
#  define EXTERN 
#else
#  define EXTERN extern
#endif


#ifdef LongLong
# define INT8 long long
#else
# define INT8 unsigned long
#endif


#define PI 3.14159265358979323846


#define maxblockdim 512

typedef struct {
  double re;
  double im;
} complex;



typedef complex su3 [3][3];  /* su(3)-Matrix 3x3 komplexe Einträge */




typedef struct SAPARAM_LIST{
  double Tmin;
  double Tmax;
  double expo;
  int N;
  int checkint;
} SAPARAM_LIST;



typedef struct THERMPARAM_LIST{
  int Nsweep;
  int Nsave;
  double beta;
  int checkint;
  int startcond;
} THERMPARAM_LIST;


typedef complex spinor[12];
typedef complex propmatrix[12][12];
typedef complex fbyf[4][4];




EXTERN int T, L, LX, LY, LZ, VOLUME;
EXTERN spinor ** g_spinor_field;
EXTERN int g_precision;
EXTERN int NMASS;
EXTERN int NKSQUARE;



EXTERN double BETA;
EXTERN double PLAQ;
EXTERN double FUNC;
EXTERN double DADA;
EXTERN double maxDADA;

EXTERN int g_numofgpu;


EXTERN su3 * g_gf;
EXTERN su3 * trafo1, * trafo2;



EXTERN int * nn;
EXTERN int * dev_nn;

EXTERN int * ind;
EXTERN int * lexic2eo;

EXTERN int * eoidx_even;
EXTERN int * eoidx_odd;
EXTERN int * dev_eoidx_even;
EXTERN int * dev_eoidx_odd;


EXTERN int * nn_eo;
EXTERN int * dev_nn_eo;
EXTERN int * nn_oe;
EXTERN int * dev_nn_oe;


/*  the device gauge field */
#ifdef GF_8
EXTERN dev_su3_8 * dev_gf;
EXTERN dev_su3_8 * dev_gf2;
EXTERN dev_su3_8 * h2d_gf;
#else
EXTERN dev_su3_2v * dev_gf;
EXTERN dev_su3_2v * dev_gf2;
EXTERN dev_su3_2v * h2d_gf;
#endif


/*  the device trafo field */
#ifdef GF_8
EXTERN dev_su3_8 * dev_trafo1;
EXTERN dev_su3_8 * dev_trafo2;
EXTERN dev_su3_8 * h2d_trafo;
#else
EXTERN dev_su3_2v * dev_trafo1;
EXTERN dev_su3_2v * dev_trafo2;
EXTERN dev_su3_2v * h2d_trafo;
#endif


/* the device staple field */
#ifdef GF_8
EXTERN dev_su3_8 * dev_staples;
#else
EXTERN dev_su3_2v * dev_staples;
#endif


// used for global summation of parts of the functional
// every threadblock produces F for a sublattice 
// must match the configuation (BLOCK) in <<< , >>>
EXTERN double*  redfield_F;
EXTERN double*  redfield_dAdA;
EXTERN double*  redfield_maxdAdA;
EXTERN double*  redfield_plaq;


EXTERN size_t dev_spinsize;
EXTERN size_t output_size;
EXTERN int* dev_grid;
EXTERN double * dev_output;


EXTERN float * dev_rndunif_field;
EXTERN float * dev_rndgauss_field;




#endif

