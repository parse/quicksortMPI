########################################################################
# Makefile for MPItutorial
#
########################################################################

CC         =  mpicc
CCFLAGS    =  -O3
LIBS       =  -lmpi

all:
	@echo "Usage: make qsort"

qsort:          main.c
	$(CC) $(CCFLAGS) -o qsort main.c $(LIBS)

clean:
	rm -f *.o *~ qsort