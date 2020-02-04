make:
	gcc -o my_mpi my_mpi.c -lm my_mpi.h -pthread
	#gcc -o my_mpi_old my_mpi_old.c -lm my_mpi.h -pthread
clean:
	rm my_mpi
	#rm my_mpi_old

run:
	./simple_my_prun.sh ./my_mpi
	#./simple_my_prun.sh ./my_mpi_old

