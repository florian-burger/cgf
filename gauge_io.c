#include "lime.h" 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cudaglobal.h"
#include "global.h"
#include "complex.h"
#include "spinor_io.h"
#include "gauge_io.h"


#define MAXBUF 2048





extern LimeReader* limeCreateReader(FILE *fp);
extern void limeDestroyReader(LimeReader *r);
extern int limeReaderNextRecord(LimeReader *r);
extern char *limeReaderType(LimeReader *r);
extern LimeWriter* limeCreateWriter(FILE *fp);

extern int LX,LY,LZ,T,VOLUME;
extern su3 * gaugefield;
extern su3 * gtrafofield;
extern double FUNC;
extern double DADA;


extern void byte_swap_assign_double(void * out_ptr, void * in_ptr, int nmemb);
extern void byte_swap_assign_float(void * out_ptr, void * in_ptr, int nmemb);
int getXmlEntry(char* searchstring, char* tagname, char* content);


void transpose_gf(su3* gf){
  int k;
  int l,m;
  complex chelp;
  su3 helpmatrix;
  int su3numofbytes = sizeof(su3);
  for(k=0; k<4*VOLUME; k++){
    memcpy(&helpmatrix,&(gf[k]),su3numofbytes);
    for(l=0; l<3;l++){ /* transposition of one su(3) matrix */
      for(m=l+1;m<3;m++){
        chelp=helpmatrix[l][m];
        helpmatrix[l][m] = helpmatrix[m][l];
        helpmatrix[m][l] = chelp;
      } 
    } 
    memcpy(&(gf[k]),&helpmatrix,su3numofbytes);
  }
}




void transpose_trafo(su3* trafo){
  int k;
  int l,m;
  complex chelp;
  su3 helpmatrix;
  int su3numofbytes = sizeof(su3);
  for(k=0; k<VOLUME; k++){
    memcpy(&helpmatrix,&(trafo[k]),su3numofbytes);
    for(l=0; l<3;l++){ /* transposition of one su(3) matrix */
      for(m=l+1;m<3;m++){
        chelp=helpmatrix[l][m];
        helpmatrix[l][m] = helpmatrix[m][l];
        helpmatrix[m][l] = chelp;
      } 
    } 
    memcpy(&(trafo[k]),&helpmatrix,su3numofbytes);
  }
}




void swap_directions_gf(su3* gf){
  int k;
  su3 helpmatrix[4];
  int su3numofbytes = sizeof(su3);
  for(k=0;k<VOLUME; k++){
    memcpy(&helpmatrix[0],&(gf[4*k]),4*su3numofbytes);
    
    memcpy(&(gf[4*k+0]),&(helpmatrix[1]),su3numofbytes);
    
    memcpy(&(gf[4*k+1]),&(helpmatrix[2]),su3numofbytes);
   
    memcpy(&(gf[4*k+2]),&(helpmatrix[3]),su3numofbytes);
    
    memcpy(&(gf[4*k+3]),&(helpmatrix[0]),su3numofbytes);
  }
}




void showsu3(su3 M){
 printf ("(%f,%f)\t(%f,%f)\t(%f,%f)\n", M[0][0].re,M[0][0].im,M[0][1].re,M[0][1].im,M[0][2].re,M[0][2].im);
 printf ("(%f,%f)\t(%f,%f)\t(%f,%f)\n", M[1][0].re,M[1][0].im,M[1][1].re,M[1][1].im,M[1][2].re,M[1][2].im);
 printf ("(%f,%f)\t(%f,%f)\t(%f,%f)\n", M[2][0].re,M[2][0].im,M[2][1].re,M[2][1].im,M[2][2].re,M[2][2].im);
}






int get_unformatted_entry(char* searchstring,char* tagname, char* content){
  char * substr;
  char tokens[] = " \n\t";
  
  substr = strtok (searchstring,tokens);
  while (substr != NULL)
  {
    if(strcmp(substr,tagname)==0){
      substr = strtok (NULL, tokens); /* get next substr == '='! */
      substr = strtok (NULL, tokens); /* get next substr == 'value'! */
      strcpy(content,substr);
      break;
    }
    substr = strtok (NULL, tokens);
  }
return 0;
}



int read_gf_header_ildg(char* filename){
  FILE * ifs;
  int status=0 , i=0, k=0;
  char character;
  n_uint64_t bytes = sizeof(float);
  n_uint64_t singlebyte = 1;
  char * header_type;
  LimeReader * limereader;
  float fhelp;
  double dhelp;
  int su3numofdoubles = (sizeof(su3)/sizeof(double));
  int su3numofbytes = sizeof(su3);
    
  int x,y,z,t,mu,nu;  
    
  float * farray = (float*) malloc(su3numofdoubles*sizeof(float));
  double * darray = (double*) malloc(su3numofbytes);
  
  char xmlentry[512];
  char xmlcontent[512];
  
  int readformat = 0;
  int precision = 0; /* 32 = single, 64 = double*/
  int dimension;
  int readprecision = 0;
  int retval;

  if((ifs = fopen(filename, "r")) == (FILE*)NULL) {
      fprintf(stderr, "Error opening file %s\n", filename);
    return(-1);
  }
  printf("Reading ildg-header out of file: '%s'\n",filename);

  limereader = limeCreateReader( ifs );
  if( limereader == (LimeReader *)NULL ) {
      fprintf(stderr, "Unable to open LimeReader\n");
    return(-1);
  }
  while( (status = limeReaderNextRecord(limereader)) != LIME_EOF ) {
    if(status != LIME_SUCCESS) {
      fprintf(stderr, "limeReaderNextRecord returned error with status = %d!\n", status);
      status = LIME_EOF;
      break;
    }
    header_type = limeReaderType(limereader);
    
    /*printf("The selected file contains a record of type: %s\n",header_type);    */
    if(!strcmp("ildg-format",header_type)){
      bytes = limeReaderBytes(limereader);
      strcpy(xmlentry,"");
      strcpy(xmlcontent,"");
      limeReaderReadData(xmlentry,&bytes,limereader); /* hier ganzer Eintrag in xmlentry*/
      
      getXmlEntry(xmlentry,"field", xmlcontent);
      printf("The field is of type: %s\n",xmlcontent);
      
      getXmlEntry(xmlentry,"precision", xmlcontent);
      sscanf(xmlcontent,"%d",&precision);
      readprecision = 1; /* precision aus xml - entry gelesen*/
      printf("The precision is: %d\n",precision);

      
      getXmlEntry(xmlentry,"lx", xmlcontent);
      sscanf(xmlcontent,"%d",&dimension);
      printf("LX: %d\n",dimension);
      LX = dimension;
      
      getXmlEntry(xmlentry,"ly", xmlcontent);
      sscanf(xmlcontent,"%d",&dimension);
      printf("LY: %d\n",dimension);
      LY = dimension;
      
      getXmlEntry(xmlentry,"lz", xmlcontent);
      sscanf(xmlcontent,"%d",&dimension);
      printf("LZ: %d\n",dimension);
      LZ = dimension;
      
      getXmlEntry(xmlentry,"lt", xmlcontent);
      sscanf(xmlcontent,"%d",&dimension);
      printf("T: %d\n",dimension);
      T = dimension;
          
      VOLUME = LX*LY*LZ*T;
      printf("VOLUME: %d\n",VOLUME);
      
      readformat = 1;
    }
    if(!strcmp("xlf-info",header_type)){
      bytes = limeReaderBytes(limereader);
      strcpy(xmlentry,"");
      strcpy(xmlcontent,"");
      limeReaderReadData(xmlentry,&bytes,limereader); /* hier ganzer Eintrag in xmlentry*/
      
      get_unformatted_entry(xmlentry,"plaquette", xmlcontent); 
      PLAQ = atof(xmlcontent);
      printf("plaquette: %e\n",PLAQ);
    }
    if(!strcmp("ildg-binary-data",header_type)) {
      break;
    }
  }
  printf("\n\n");
return(0);
}


int read_gf_ildg(su3 * gf, char* filename){
  FILE * ifs;
  int status=0 , k=0;
  int i=0;
  char character;
  n_uint64_t bytes = (n_uint64_t) sizeof(float);
  n_uint64_t singlebyte = 1;
  char * header_type;
  LimeReader * limereader;
  float fhelp;
  double dhelp;
  int su3numofdoubles = (sizeof(su3)/sizeof(double));
  int su3numofbytes = sizeof(su3);
    
  int x,y,z,t,mu,nu;  
    
  float * farray = (float*) malloc(su3numofdoubles*sizeof(float));
  double * darray = (double*) malloc(su3numofbytes);
  
  char xmlentry[512];
  char xmlcontent[512];
  char helpstring[512];
  
  int readformat = 0;
  int precision = 0; /* 32 = single, 64 = double*/
  int dimension;
  int readprecision = 0;
  int retval;
  double plaq;

  if((ifs = fopen(filename, "r")) == (FILE*)NULL) {
      fprintf(stderr, "Error opening file %s\n", filename);
    return(-1);
  }
  printf("Reading ildg-format:\n");
  printf("Reading file '%s' in ildg-format\n",filename);

  limereader = limeCreateReader( ifs );
  if( limereader == (LimeReader *)NULL ) {
      fprintf(stderr, "Unable to open LimeReader\n");
    return(-1);
  }
  while( (status = limeReaderNextRecord(limereader)) != LIME_EOF ) {
    if(status != LIME_SUCCESS) {
      fprintf(stderr, "limeReaderNextRecord returned error with status = %d!\n", status);
      status = LIME_EOF;
      break;
    }
    header_type = limeReaderType(limereader);
    
    /*printf("The selected file contains a record of type: %s\n",header_type);    */
    if(!strcmp("ildg-format",header_type)){
      bytes = limeReaderBytes(limereader);
      strcpy(xmlentry,"");
      strcpy(xmlcontent,"");
      limeReaderReadData(xmlentry,&bytes,limereader); /* hier ganzer Eintrag in xmlentry*/
      
      getXmlEntry(xmlentry,"field", xmlcontent);
      printf("The field is of type: %s\n",xmlcontent);
      
      getXmlEntry(xmlentry,"precision", xmlcontent);
      sscanf(xmlcontent,"%d",&precision);
      readprecision = 1; /* precision aus xml - entry gelesen*/
      printf("The precision is: %d\n",precision);

      
      getXmlEntry(xmlentry,"lx", xmlcontent);
      sscanf(xmlcontent,"%d",&dimension);
      printf("LX: %d\n",dimension);
      if(LX != dimension){
        fprintf(stderr, "Error in read_gf_ildg: wrong LX dimension in gauge file. Aborting...\n");
        exit(120);
      }
      
      
      getXmlEntry(xmlentry,"ly", xmlcontent);
      sscanf(xmlcontent,"%d",&dimension);
      printf("LY: %d\n",dimension);
      if(LY != dimension){
        fprintf(stderr, "Error in read_gf_ildg: wrong LY dimension in gauge file. Aborting...\n");
        exit(120);
      }
      
      getXmlEntry(xmlentry,"lz", xmlcontent);
      sscanf(xmlcontent,"%d",&dimension);
      printf("LZ: %d\n",dimension);
      if(LZ != dimension){
        fprintf(stderr, "Error in read_gf_ildg: wrong LZ dimension in gauge file. Aborting...\n");
        exit(120);
      }
      
      getXmlEntry(xmlentry,"lt", xmlcontent);
      sscanf(xmlcontent,"%d",&dimension);
      printf("T: %d\n",dimension);
      if(T != dimension){
        fprintf(stderr, "Error in read_gf_ildg: wrong T dimension in gauge file. Aborting...\n");
        exit(120);
      }
          
      VOLUME = LX*LY*LZ*T;
      printf("VOLUME: %d\n",VOLUME);
      
      readformat = 1;
    }    
    if(!strcmp("xlf-info",header_type)){
      bytes = limeReaderBytes(limereader);
      strcpy(xmlentry,"");
      strcpy(xmlcontent,"");
      limeReaderReadData(xmlentry,&bytes,limereader); /* hier ganzer Eintrag in xmlentry*/
      
      strcpy(helpstring,xmlentry);
      
      get_unformatted_entry(xmlentry,"plaquette", xmlcontent);  
      PLAQ = atof(xmlcontent);
      printf("plaquette: %e\n",PLAQ);
      
      
      get_unformatted_entry(helpstring,"beta", xmlcontent);  
      BETA = atof(xmlcontent);
      printf("beta: %e\n",BETA);
    }

    if(!strcmp("ildg-binary-data",header_type)) {
      break;
    }
  }
  if(status == LIME_EOF) {
    limeDestroyReader(limereader);
    fclose(ifs);
    return(-1);
  }
  

  if(readformat == 0){
    fprintf(stderr, "Unable to read Xml-Format (ildg-format) out of LimeReader\n");
    return(-1);    
  } /* is lattice size already initialized? */
  bytes = limeReaderBytes(limereader);
  
  if(bytes == (n_uint64_t)LX*LY*LZ*T*4*(18*sizeof(float)) && ( (precision == 32) || (readprecision == 0) )) { /* file contains floats  3*3*2 floats *4 wg 4 Richtungen*/

  
  bytes = (n_uint64_t)sizeof(su3)/2; /* halbe Gr��e, wegen floats */
  
  i=0;
     for(i = 0; i < 4*VOLUME; i++){  
             for(mu =0; mu<4; mu++){
               limeReaderReadData(&farray[0], &bytes, limereader);
	       for(k=0; k < su3numofdoubles;k++){ /* convert to double and make little-endian */ 
#ifndef WORDS_BIGENDIAN   
                   fhelp = farray[k];          
                   byte_swap_assign_float(&(farray[k]), &fhelp , 1);
#else

#endif
                   darray[k] = (double) farray[k];
                }
                   memcpy(&(gf[i]) ,&darray[0], su3numofbytes); 
             }
     }
   }
   
  
  else{
  if(bytes == (n_uint64_t)LX*LY*LZ*T*4*(18*sizeof(double)) && ( (precision == 64) || (readprecision == 0) )) { /* file contains doubles  3*3*2 doubles *4 wegen 4 Richtungen */
  bytes = (n_uint64_t) sizeof(su3);  
  /*printf("number of bytes = %d\nreadprecision = %d\nprecision = %d\n",bytes,readprecision,precision);
 */
     for(i = 0; i < 4*VOLUME; i++){  

#ifndef WORDS_BIGENDIAN     
               limeReaderReadData(&darray[0], &bytes, limereader);
               byte_swap_assign_double(&(gf[i]),&(darray[0]), su3numofdoubles);
#else
               limeReaderReadData(&(gf[i]), &bytes, limereader);
#endif

             }

   }
   else{ /* precision xml-entry doesn't match size of data-block */
     fprintf(stderr, "Error in %s: Xml-Entry 'precision' does not match size of data block. Possible file corruption! Aborting...\n",filename);
     exit(500);
   }
 }
   
   
  if(status < 0 && status != LIME_EOR) {
    fprintf(stderr, "LIME read error occured with status = %d while reading file %s!\n Aborting...\n", status, filename);
    exit(500);
  }

  limeDestroyReader(limereader);
  fclose(ifs);
  free(farray);
  free(darray);
  printf("\n\n");
  return(0);
}








int write_gf_ildg(su3 * gf, char * filename, const int prec){/*writes gauge-field gf to file 'filename' with
                                                                   prec = 32 (single) prec = 64 (double)*/
  FILE * ofs;
  char * header_type = NULL;
  LimeWriter * limewriter = NULL;
  n_uint64_t bytes;
  int status = 0;  
  int ME_flag=1, MB_flag=1;
  int counter,i,j;
  int k;
  char message[MAXBUF];
  LimeRecordHeader * limeheader = NULL;
  
  int su3numofdoubles = (sizeof(su3)/sizeof(double));
  int su3numofbytes = sizeof(su3);
  
  
  float farray [su3numofdoubles];
  double darray[su3numofdoubles];
  
  float  farray2[su3numofdoubles];
  double  darray2[su3numofdoubles];
  

  
  if((ofs = fopen(filename, "w")) == (FILE*)NULL) {
      fprintf(stderr, "Error opening file %s\n", filename);
    return(-1);
  }
   printf("Writing ildg-format:\n");
 
  printf("Writing to file '%s' in ildg-format\n",filename);
  limewriter = limeCreateWriter( ofs );
  if(limewriter == (LimeWriter*)NULL) {
    fprintf(stderr, "LIME error in file %s for writing!\n Aborting...\n", filename);
    exit(500);
  }
  

/* ildg-format */  

if(prec == 64) {
  sprintf(message, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<ildgFormat xmlns=\"http://www.lqcd.org/ildg\"\n            xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n            xsi:schemaLocation=\"http://www.lqcd.org/ildg filefmt.xsd\">\n  <version> 1.0 </version>\n  <field> su3gauge </field>\n  <precision> 64 </precision>\n  <lx> %d </lx>\n  <ly> %d </ly>\n  <lz> %d </lz>\n  <lt> %d </lt>\n</ildgFormat>",LX,LY,LZ,T);

}
else if(prec == 32) {
  sprintf(message, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<ildgFormat xmlns=\"http://www.lqcd.org/ildg\"\n            xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n            xsi:schemaLocation=\"http://www.lqcd.org/ildg filefmt.xsd\">\n  <version> 1.0 </version>\n  <field> su3gauge </field>\n  <precision> 32 </precision>\n  <lx> %d </lx>\n  <ly> %d </ly>\n  <lz> %d </lz>\n  <lt> %d </lt>\n</ildgFormat>",LX,LY,LZ,T);
} 
else{
    fprintf(stderr, "Wrong precision! (must be 32 or 64) Aborting...\n", status);
    exit(500);
}

  bytes = (n_uint64_t) strlen( message );
  limeheader = limeCreateHeader(MB_flag, ME_flag, "ildg-format", bytes);
  status = limeWriteRecordHeader( limeheader, limewriter);
  if(status < 0 ) {
    fprintf(stderr, "LIME write header error %d\n", status);
    exit(500);
  }
  limeDestroyHeader( limeheader );
  limeWriteRecordData(message, &bytes, limewriter);


/*  xlf- info */

  sprintf(message, " plaquette = %.6f\ntrajectory nr = \nbeta = %.6f, kappa = , mu = , c2_rec = \ntime = 1216028646\nhmcversion = 4.0.1\nmubar = \nepsilonbar = \ndate = " ,PLAQ,BETA);


  bytes = (n_uint64_t)strlen( message );
  limeheader = limeCreateHeader(MB_flag, ME_flag, "xlf-info", bytes);
  status = limeWriteRecordHeader( limeheader, limewriter);
  if(status < 0 ) {
    fprintf(stderr, "LIME write header error %d\n", status);
    exit(500);
  }
  limeDestroyHeader( limeheader );
  limeWriteRecordData(message, &bytes, limewriter);


  if (prec == 64){
    bytes = (n_uint64_t)4*T*LX*LY*LZ*su3numofbytes;
  }
  else if(prec == 32){
    bytes = (n_uint64_t)4*T*LX*LY*LZ*su3numofbytes/2;
  }
  
  limeheader = limeCreateHeader(MB_flag, ME_flag, "ildg-binary-data", bytes);
  status = limeWriteRecordHeader( limeheader, limewriter);
  if(status < 0 ) {
    fprintf(stderr, "LIME write header (scidac-binary-data) error %d\n", status);
    exit(500);
  }
  limeDestroyHeader( limeheader );
  
  
 
  if(prec == 64){ 
    counter = 0;
    bytes =  (n_uint64_t)su3numofbytes;  /* number of bytes in spin[x] */
    
    for( k = 0; k < 4*VOLUME; k++) {

              memcpy(&(darray[0]),&(gf[k]),su3numofbytes);
#ifndef WORDS_BIGENDIAN 
              /* convert to big-endian (as necessary for linux)*/
              byte_swap_assign_double(&(darray2[0]),&(darray[0]),su3numofdoubles);
              status = limeWriteRecordData((void*)darray2, &bytes, limewriter);
#else
              status = limeWriteRecordData((void*)darray, &bytes, limewriter); 
#endif                      
          }
  }
  else{
    if(prec == 32){
      bytes =  (n_uint64_t)su3numofbytes/2;  /* number of bytes in spin[x] */
      
      
      for( k = 0; k < 4*VOLUME; k++) {
              memcpy(&(darray[0]),&(gf[k]), su3numofbytes);
              for(i=0; i<su3numofdoubles; i++){         /* convert to single */
                farray[i] = (float)(darray[i]); 
              }
#ifndef WORDS_BIGENDIAN 
              /* convert to big-endian (as necessary for linux)  */
              byte_swap_assign_float(&(farray2[0]),&(farray[0]),su3numofdoubles);
              status = limeWriteRecordData((void*)farray2, &bytes, limewriter);
#else
              status = limeWriteRecordData((void*)farray, &bytes, limewriter);
#endif              
            } 
    }
    else{
      fprintf(stderr, "Error in write_gf_ildg: precision neither 64 nor 32! Aborting...\n");
      exit(500);
    }
  }

  limeDestroyWriter(limewriter);
  fflush(ofs);
  fclose(ofs);
  printf("\n\n");
return(0);
}






