#ifndef _SPINOR_IO_H
#define _SPINOR_IO_H

#include "lime.h"


void byte_swap_assign_double(void * out_ptr, void * in_ptr, int nmemb);
void byte_swap_assign_float(void * out_ptr, void * in_ptr, int nmemb);
int getXmlEntry(char* searchstring, char* tagname, char* content);
int get_unformatted_entry(char* searchstring,char* tagname, char* content);
void showsu3(su3 M);

int write_ildg_format_xml(char *filename, LimeWriter * limewriter, const int prec);
int read_gf_header_ildg(char* filename);
int write_gf_ildg(su3 * gf, char * filename, const int prec);
int read_gf_ildg(su3 * gf , char* filename);

void transpose_gf(su3* gf);
void transpose_trafo(su3* trafo);

void swap_directions_gf(su3* gf);
#endif









