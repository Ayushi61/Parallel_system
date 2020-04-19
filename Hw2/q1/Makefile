make:
	#gcc -o my_mpi my_mpi.c -lm my_mpi.h -pthread
	#gcc -o my_mpi_old my_mpi_old.c -lm my_mpi.h -pthread
	#gcc -o my_main my_main.c -lm my_mpi.c -pthread
	gcc -o mpi_sendrcv mpi_sendrcv.c -lm my_mpi.c -pthread
clean:
	#rm my_mpi
	#rm my_mpi_old
	#rm my_main
	rm mpi_sendrcv
run:
	#./simple_my_prun.sh ./my_mpi
	#./simple_imy_prun.sh ./my_mpi_old
	./simple_my_prun.sh ./mpi_sendrcv
	#./simple_my_prun.sh ./my_main

