#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi/mpi.h>
//#include "fuzzy.h"
#include "fuzzy-loader.h"
#include "fuzzy-core.h"

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
    int i,j,k;

    int numInputsMFS;
    int numOutputsMFS;
    int numRules;

    float **inputs, **outputs;
    int *inputs_sz, *outputs_sz, *rules, *local_rules, rules_sz, rules_sz_ln, local_rules_sz;
    double *in, *out;

    double* fuzzyValues;

    char pack_iosz[sizeof(int)*3];
  	int pos = 0;

  	//rules scatterv structure
  	int* sendcounts;
  	int* displs;
 
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
	    rules_sz_ln = 1 + numInputsMFS + numOutputsMFS;
	    local_rules_sz = local_n*rules_sz_ln;

	    MPI_Bcast(inputs_sz, numInputsMFS, MPI_INT, 0, comm);
	    MPI_Bcast(outputs_sz, numOutputsMFS, MPI_INT, 0, comm);

	    //Broadcasting MFS
	    for(i = 0; i < numInputsMFS; i++)
	        MPI_Bcast(inputs[i], inputs_sz[i], MPI_FLOAT, 0, comm);

	    for(i = 0; i < numOutputsMFS; i++)
	        MPI_Bcast(outputs[i], outputs_sz[i], MPI_FLOAT, 0, comm);

	    fclose(fp);

	    //Broadcasting Rules
	    //Mounting scatterv structure
	    sendcounts = malloc(sizeof(int)*p);
	    displs = malloc(sizeof(int)*p);

	    sendcounts[0] = local_rules_sz;
	    displs[0] = 0;
	    for(i = 1; i < p; i++) {
	    	sendcounts[i] = ((int)(i < (numRules%p)) + (numRules/p))*rules_sz_ln;
	    	displs[i] = displs[i-1] + sendcounts[i-1];
	    }

	    local_rules = malloc(sizeof(int)*local_rules_sz);
	    MPI_Scatterv(rules, sendcounts, displs, MPI_INT, local_rules, local_rules_sz, MPI_INT, 0, comm);

	    free(sendcounts);
	    free(displs);

	} else {

		MPI_Bcast(pack_iosz, sizeof(int)*3, MPI_PACKED, 0, comm);

		MPI_Unpack(pack_iosz, sizeof(int)*3, &pos, &numInputsMFS, 1, MPI_INT, comm);  
		MPI_Unpack(pack_iosz, sizeof(int)*3, &pos, &numOutputsMFS, 1, MPI_INT, comm);  
		MPI_Unpack(pack_iosz, sizeof(int)*3, &pos, &numRules, 1, MPI_INT, comm);

		local_n = (int)(my_rank < (numRules%p)) + (numRules/p);

		rules_sz_ln = 1 + numInputsMFS + numOutputsMFS;
		local_rules_sz = local_n*rules_sz_ln;

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

	    local_rules = malloc(sizeof(int)*local_rules_sz);
	    MPI_Scatterv(rules, sendcounts, displs, MPI_INT, local_rules, local_rules_sz, MPI_INT, 0, comm);

	}

	srand(time(NULL));

	
	/*----------------------------------------------------------------*/

	/*Read inputs and broadcast them----------------------------------*/

	//TODO

	in = malloc(sizeof(double)*numInputsMFS);
	out = malloc(sizeof(double)*numOutputsMFS);
	for(i = 0; i < numInputsMFS; i++){
    	in[i] = (rand()%2001 - 1)/2000.0;
    }

    for(i = 0; i < numOutputsMFS; i++){
    	out[i] = (rand()%1001)/1000.0;
    }

	/*----------------------------------------------------------------*/

	/*Computing-------------------------------------------------------*/

	//fuzzifing

	fuzzyValues = malloc(sizeof(double)*local_n);
	int jump;
	float* cur_MF;
	double* cur_value_v;
	double cur_value;
	int cur_MF_sz;
	cur_value_v = malloc(sizeof(double)*numInputsMFS);
	for(i = 0; i < 1; i++){
		jump = 0;
		for(j = 1; j <= numInputsMFS; j++){
			cur_MF_sz = (int)(inputs[j-1][local_rules[i*rules_sz_ln+j]]);
			cur_MF = malloc(sizeof(float)*cur_MF_sz);
			
			for(k = 0; k < local_rules[i*rules_sz_ln+j]; k++)
				jump += (int)(inputs[j-1][k]);
			for(k = 0; k < cur_MF_sz; k++){
				cur_MF[k] = inputs[j-1][k+jump+1];
			
			}

			cur_value_v[j] = fuzzify(in[j-1], cur_MF, cur_MF_sz);
			//cur_value_v[j] = fuzzify(-1, cur_MF, cur_MF_sz);
			free(cur_MF);
		}
		for(j = 0; j < numInputsMFS; j += 2){
			if(local_rules[i*rules_sz_ln+numInputsMFS] == 1)//andOp
				fuzzyValues[i] = andOp(cur_value_v[j], cur_value_v[j+1]);
			else if(local_rules[i*rules_sz_ln+numInputsMFS] == 2)//andOp
				fuzzyValues[i] = orOp(cur_value_v[j], cur_value_v[j+1]);
		}

		if(!my_rank){
			printf("%f\n", fuzzyValues[i]);
		}

	}

	//TODO

	/*----------------------------------------------------------------*/

	/*Reduce values---------------------------------------------------*/

	//TODO

	/*----------------------------------------------------------------*/

	#ifdef DEBUG
	printf("my_rank = %i NumInputsMFS = %i\n", my_rank, numInputsMFS);
	printf("my_rank = %i NumOutputsMFS = %i\n", my_rank, numOutputsMFS);
	printf("my_rank = %i NumRules = %i\n", my_rank, numRules);
	printf("my_rank = %i Rules = %i\n", my_rank, rules_sz_ln);
	/*if(my_rank == 1)
		for(i = 0; i < local_rules_sz; i++)
			printf("%i\n", local_rules[i]);*/
	#endif



    MPI_Finalize();
    return 0;
}