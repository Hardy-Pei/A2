all: run

run: compile
	mpirun -n 21 ./a.out
	

compile:
	export OMP_NUM_THREADS=8
	mpicc -fopenmp wsn.c baseStation.c nodes.c encryption.c IP.c
# 	mpicc -fopenmp -o run wsn.o baseStation.o nodes.o encryption.o
	


