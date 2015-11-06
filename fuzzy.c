#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>
//#include "fuzzy.h"
#include "fuzzy-loader.h"

int main(int argc, char* argv[])
{
    if(argc < 2) {
        printf("Usage: %s <fis path>\n", argv[0]);
        exit(1);
    }

    int my_rank, p;
	int local_n;
	MPI_Comm comm;

	MPI_Init(&argc, &argv);

	comm = MPI_COMM_WORLD;

	MPI_Comm_size(comm, &p);
	MPI_Comm_rank(comm, &my_rank);

    char* path = argv[1];
    FILE* fp;
    int foundline = 0;
    char linebuffer[MAX_BUFFER];
    int i;

    int numInputsMFS;
    int numOutputsMFS;
    int numRules;

    float **inputs, **outputs;
    int *inputs_sz, *outputs_sz, *rules, rules_sz;

    char pack_iosz[sizeof(int)*3];
  	int pos = 0;
 
  	/*Initializing config------------------------------------------------*/
    if(my_rank == 0) {
	    fp = fopen(path, "r");

	    numInputsMFS = get_numInputs(fp);
	    numOutputsMFS = get_numOutputs(fp);
	    numRules = get_numRules(fp);

	    local_n = (int)(my_rank < (numRules%p)) + (numRules/p);

	    MPI_Pack(&numInputsMFS, 1, MPI_INT, pack_iosz, sizeof(int)*3, &pos, comm);
		MPI_Pack(&numOutputsMFS, 1, MPI_INT, pack_iosz, sizeof(int)*3, &pos, comm);
		MPI_Pack(&numRules, 1, MPI_INT, pack_iosz, sizeof(int)*3, &pos, comm);

		MPI_Bcast(pack_iosz, sizeof(int)*3, MPI_PACKED, 0, comm);
	    
	    inputs = malloc(sizeof(float*)*numInputsMFS);
	    inputs_sz = malloc(sizeof(int)*numInputsMFS);

	    for(i = 0; i < numInputsMFS; i++)
	        inputs[i] = load_input(fp, &inputs_sz[i]);

	    outputs = malloc(sizeof(float*)*numOutputsMFS);
	    outputs_sz = malloc(sizeof(int)*numOutputsMFS);
	    for(i = 0; i < numOutputsMFS; i++)
	        outputs[i] = load_output(fp, &outputs_sz[i]); 

	    rules = load_rules(fp, numInputsMFS, numOutputsMFS, numRules, &rules_sz);


	    MPI_Bcast(inputs_sz, numInputsMFS, MPI_INT, 0, comm);
	    MPI_Bcast(outputs_sz, numOutputsMFS, MPI_INT, 0, comm);

	    for(i = 0; i < numInputsMFS; i++)
	        MPI_Bcast(inputs[i], inputs_sz[i], MPI_FLOAT, 0, comm);

	    for(i = 0; i < numOutputsMFS; i++)
	        MPI_Bcast(outputs[i], outputs_sz[i], MPI_FLOAT, 0, comm);

	    fclose(fp);


	} else {

		MPI_Bcast(pack_iosz, sizeof(int)*3, MPI_PACKED, 0, comm);

		MPI_Unpack(pack_iosz, sizeof(int)*3, &pos, &numInputsMFS, 1, MPI_INT, comm);  
		MPI_Unpack(pack_iosz, sizeof(int)*3, &pos, &numOutputsMFS, 1, MPI_INT, comm);  
		MPI_Unpack(pack_iosz, sizeof(int)*3, &pos, &numRules, 1, MPI_INT, comm);

		local_n = (int)(my_rank < (numRules%p)) + (numRules/p);

		inputs = malloc(sizeof(float*)*numInputsMFS);
		outputs = malloc(sizeof(float*)*numOutputsMFS);
		
		inputs_sz = malloc(sizeof(int)*numInputsMFS);
		outputs_sz = malloc(sizeof(int)*numOutputsMFS);

		MPI_Bcast(inputs_sz, numInputsMFS, MPI_INT, 0, comm);
	    MPI_Bcast(outputs_sz, numOutputsMFS, MPI_INT, 0, comm);

	    for(i = 0; i < numInputsMFS; i++){
	    	inputs[i] = malloc(sizeof(float)*inputs_sz[i]);
	        MPI_Bcast(inputs[i], inputs_sz[i], MPI_FLOAT, 0, comm);
	    }

	    for(i = 0; i < numOutputsMFS; i++){
	    	outputs[i] = malloc(sizeof(float)*outputs_sz[i]);
	        MPI_Bcast(outputs[i], outputs_sz[i], MPI_FLOAT, 0, comm);
	    }


	}
	/*----------------------------------------------------------------*/

	/*Read inputs and broadcast them----------------------------------*/

	//TODO

	/*----------------------------------------------------------------*/

	/*Computing-------------------------------------------------------*/

	//TODO

	/*----------------------------------------------------------------*/

	/*Reduce values---------------------------------------------------*/

	//TODO

	/*----------------------------------------------------------------*/

	#ifdef DEBUG
	printf("my_rank = %i NumInputsMFS = %i\n", my_rank, numInputsMFS);
	printf("my_rank = %i NumOutputsMFS = %i\n", my_rank, numOutputsMFS);
	printf("my_rank = %i NumRules = %i\n", my_rank, numRules);
	printf("my_rank = %i Rules = %i\n", my_rank, local_n);
	#endif



    MPI_Finalize();
    return 0;
}