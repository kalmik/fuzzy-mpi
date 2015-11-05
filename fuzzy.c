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

    int numInputs;
    int numOutputs;
    int numRules;

    float **inputs, **outputs;
    int *inputs_sz, *outputs_sz, *rules, rules_sz;

    char pack_iosz[sizeof(int)*3];
  	int pos = 0;
 

    if(my_rank == 0) {
	    fp = fopen(path, "r");

	    numInputs = get_numInputs(fp);
	    numOutputs = get_numOutputs(fp);
	    numRules = get_numRules(fp);

	    MPI_Pack(&numInputs, 1, MPI_INT, pack_iosz, sizeof(int)*3, &pos, comm);
		MPI_Pack(&numOutputs, 1, MPI_INT, pack_iosz, sizeof(int)*3, &pos, comm);
		MPI_Pack(&numRules, 1, MPI_INT, pack_iosz, sizeof(int)*3, &pos, comm);

		MPI_Bcast(pack_iosz, sizeof(int)*3, MPI_PACKED, 0, comm);
	    
	    inputs = malloc(sizeof(float*)*numInputs);
	    inputs_sz = malloc(sizeof(int)*numInputs);

	    for(i = 0; i < numInputs; i++)
	        inputs[i] = load_input(fp, &inputs_sz[i]);

	    outputs = malloc(sizeof(float*)*numOutputs);
	    outputs_sz = malloc(sizeof(int)*numOutputs);
	    for(i = 0; i < numOutputs; i++)
	        outputs[i] = load_output(fp, &outputs_sz[i]); 

	    rules = load_rules(fp, numInputs, numOutputs, numRules, &rules_sz);

		/*#ifdef DEBUG
	    printf("NumInputs = %i\n", numInputs);
	    printf("NumOutputs = %i\n", numOutputs);
	    printf("NumRules = %i\n", numRules);
	    printf("input1 \n");
	    for(i = 0; i < inputs_sz[1]; i++)
	        printf("%.1f ", inputs[1][i]);
	    printf("\n");
	    printf("output\n");
	    for(i = 0; i < outputs_sz[0]; i++)
	        printf("%.1f ", outputs[0][i]);
	    printf("\n");
	    printf("rules qtd = %i\n", rules_sz);
	    for(i = 0; i < rules_sz; i++)
	        printf("%i ", rules[i]);
	    printf("\n");
		#endif*/

	    fclose(fp);
	}
	else {
		MPI_Bcast(pack_iosz, sizeof(int)*3, MPI_PACKED, 0, comm);

		MPI_Unpack(pack_iosz, sizeof(int)*3, &pos, &numInputs, 1, MPI_INT, comm);  
		MPI_Unpack(pack_iosz, sizeof(int)*3, &pos, &numOutputs, 1, MPI_INT, comm);  
		MPI_Unpack(pack_iosz, sizeof(int)*3, &pos, &numRules, 1, MPI_INT, comm);

		inputs = malloc(sizeof(float*)*numInputs);
		outputs = malloc(sizeof(float*)*numOutputs);
		rules = load_rules(fp, numInputs, numOutputs, numRules, &rules_sz);


	}

	#ifdef DEBUG
	printf("my_rank = %i NumInputs = %i\n", my_rank, numInputs);
	printf("my_rank = %i NumOutputs = %i\n", my_rank, numOutputs);
	printf("my_rank = %i NumRules = %i\n", my_rank, numRules);
	#endif


    MPI_Finalize();
    return 0;
}