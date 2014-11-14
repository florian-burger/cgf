CUDACC = nvcc
CC = gcc-4.2
LEX=flex
#limedir = ~/codepackages/c-lime/build/lib
limedir = /users/stud/burger/codepackages/lime-1.3.2_pool/lib/

headers =  global.h 
objects =   complex.o gauge_io.o spinor_io.o rng.o su3manip.o observables.o cudagaugefix.o read_input.o 

flags =  
gpuflags = -arch=sm_20 -Xptxas -v
        #-arch=sm_20 -ftz=false -prec-div=false -prec-sqrt=false
        #-arch=sm_13
        #
        #-gencode=arch=compute_20,code=sm_20
        #-gencode=arch=compute_20,code=compute_20
        
includes = -I /users/stud/burger/codepackages/lime-1.3.2_pool/include/  
libs =   -lcuda -lcudart -lcublas -lcurand -L $(limedir) -llime
lime = $(limedir)lime_header.o $(limedir)lime_reader.o $(limedir)lime_utils.o $(limedir)lime_writer.o $(limedir)lime_fseeko.o
linkobjects =

COMPILE = ${CUDACC}   -c $(includes) -O3 -o $@ ${flags}
CUDACOMPILE = ${CUDACC}  -c   $(includes) -o $@ ${gpuflags}
##--use_fast_math 

LINK = ${CUDACC} -o $@ $(libs) ${flags}


cudagaugefix: $(objects) 
	$(LINK) $(linkobjects) $(objects)
rng.o: rng.c
	$(COMPILE) $<	
cudagaugefix.o: cudagaugefix.cu overrelax.cu simulated_annealing.cu cuda_random.cu heatbath_thermalization.cu dev_su3.cu cudaglobal.h
	$(CUDACOMPILE) $<
gauge_io.o: gauge_io.c
	$(COMPILE) $<
complex.o: complex.c
	$(COMPILE) $<
spinor_io.o: spinor_io.c
	$(COMPILE) $<
su3manip.o: su3manip.c
	$(COMPILE) $<
observables.o: observables.c
	$(COMPILE) $<
read_input.o: read_input.c
	$(COMPILE) $<
read_input.c: read_input.l
	$(LEX) -o $@ $^
clean:
	rm *.o
