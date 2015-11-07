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
    /*Iterators-------------------------------------------------*/
	int i,j,k,l;

	int numInputs;
	int numOutputs;
	int numRules;

	float **inputs, **outputs;
	int *inputs_sz, *outputs_sz, *rules, *local_rules, rules_sz, rules_sz_ln, local_rules_sz;
	double *in, *out;

	double *fuzzyValues, *outValues;

	char pack_iosz[sizeof(int)*3];
	int pos = 0;

	int jump;
	float* cur_MF;
	double* cur_value_v;
	double cur_value;
	int cur_MF_sz;

  	//rules scatterv structure
	int* sendcounts;
	int* displs;

  	/*Initializing config------------------------------------------------*/
	if(my_rank == 0) {
		fp = fopen(path, "r");

		numInputs = get_numInputs(fp);
		numOutputs = get_numOutputs(fp);
		numRules = get_numRules(fp);

		local_n = (int)(my_rank < (numRules%p)) + (numRules/p);

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
		rules_sz_ln = 1 + numInputs + numOutputs;
		local_rules_sz = local_n*rules_sz_ln;

		MPI_Bcast(inputs_sz, numInputs, MPI_INT, 0, comm);
		MPI_Bcast(outputs_sz, numOutputs, MPI_INT, 0, comm);

	    //Broadcasting MFS
		for(i = 0; i < numInputs; i++)
			MPI_Bcast(inputs[i], inputs_sz[i], MPI_FLOAT, 0, comm);

		for(i = 0; i < numOutputs; i++)
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

		MPI_Unpack(pack_iosz, sizeof(int)*3, &pos, &numInputs, 1, MPI_INT, comm);  
		MPI_Unpack(pack_iosz, sizeof(int)*3, &pos, &numOutputs, 1, MPI_INT, comm);  
		MPI_Unpack(pack_iosz, sizeof(int)*3, &pos, &numRules, 1, MPI_INT, comm);

		local_n = (int)(my_rank < (numRules%p)) + (numRules/p);

		rules_sz_ln = 1 + numInputs + numOutputs;
		local_rules_sz = local_n*rules_sz_ln;

		inputs = malloc(sizeof(float*)*numInputs);
		outputs = malloc(sizeof(float*)*numOutputs);
		
		inputs_sz = malloc(sizeof(int)*numInputs);
		outputs_sz = malloc(sizeof(int)*numOutputs);

		MPI_Bcast(inputs_sz, numInputs, MPI_INT, 0, comm);
		MPI_Bcast(outputs_sz, numOutputs, MPI_INT, 0, comm);

		for(i = 0; i < numInputs; i++){
			inputs[i] = malloc(sizeof(float)*inputs_sz[i]);
			MPI_Bcast(inputs[i], inputs_sz[i], MPI_FLOAT, 0, comm);
		}

		for(i = 0; i < numOutputs; i++){
			outputs[i] = malloc(sizeof(float)*outputs_sz[i]);
			MPI_Bcast(outputs[i], outputs_sz[i], MPI_FLOAT, 0, comm);
		}

		local_rules = malloc(sizeof(int)*local_rules_sz);
		MPI_Scatterv(rules, sendcounts, displs, MPI_INT, local_rules, local_rules_sz, MPI_INT, 0, comm);

	}

	srand(time(NULL));

	
	/*----------------------------------------------------------------*/

	double start, finish, elapsed, local_elapsed = 0;

	/*Read inputs and broadcast them----------------------------------*/

	in = malloc(sizeof(double)*numInputs);
	
	fuzzyValues = malloc(sizeof(double)*local_n);
	outValues = malloc(sizeof(double)*numOutputs*2);
	cur_value_v = malloc(sizeof(double)*numInputs);

	if(my_rank == 0){
		out = malloc(sizeof(double)*numOutputs);
		#ifdef GENERATE_DATA
		for(i = 0; i < numInputs; i++){
			in[i] = (rand()%2001 - 1)/2000.0;
		}
	    #endif
	    #ifndef GENERATE_DATA
		if(argc < 2 + numInputs) {
			printf("Usage: %s <fis path> <args>\n", argv[0]);
			exit(1);
		}
		for(i = 0; i < numInputs; i++){
			in[i] = atof(argv[2+i]);
		}
	    #endif
	}

	MPI_Barrier(comm);
	start = MPI_Wtime();

	MPI_Bcast(in, numInputs, MPI_DOUBLE, 0, comm);

	/*----------------------------------------------------------------*/
	/*Computing-------------------------------------------------------*/

	//fuzzifing

	for(i = 0; i < local_n; i++){
		
		for(j = 0; j < numInputs; j++){

			cur_MF_sz = (int)(inputs[j][local_rules[i*rules_sz_ln+j]]);
			cur_MF = malloc(sizeof(float)*cur_MF_sz);

			jump = 0;
			for(k = 0; k < local_rules[i*rules_sz_ln+j]; k++)
				jump += (int)(inputs[j][k]);

			for(k = 0; k < cur_MF_sz; k++){
				cur_MF[k] = inputs[j][k+jump+1];
			}

			cur_value_v[j] = fuzzify(in[j], cur_MF, cur_MF_sz);
			free(cur_MF);
		}

		//Implications

		fuzzyValues[i] = cur_value_v[0];
		for(j = 1; j < numInputs; j ++){

			if(local_rules[i*rules_sz_ln+rules_sz_ln-1] == 1)//andOp
				fuzzyValues[i] = andOp(cur_value_v[j], fuzzyValues[i]);

			else if(local_rules[i*rules_sz_ln+rules_sz_ln-1] == 2)//andOp
				fuzzyValues[i] = orOp(cur_value_v[j], fuzzyValues[i]);
		}

		//Aggregations

		l = 0;
		for(j = numInputs; j < numOutputs+numInputs; j++){

			cur_MF_sz = (int)(outputs[j - numInputs][local_rules[i*rules_sz_ln+j]]);
			cur_MF = malloc(sizeof(float)*cur_MF_sz);

			jump = 0;	
			for(k = 0; k < local_rules[i*rules_sz_ln+j]; k++)
				jump += (int)(outputs[j - numInputs][k]);

			for(k = 0; k < cur_MF_sz; k++){
				cur_MF[k] = outputs[j - numInputs][k+jump+1];
			}

			defuzzify(fuzzyValues[i], cur_MF, cur_MF_sz, &outValues[l], &outValues[l+1]);
			l += 2;
			free(cur_MF);
		}

	}

	/*----------------------------------------------------------------*/

	/*Reduce values and print------------------------------------------*/

	MPI_Reduce(outValues, out, numOutputs*2, MPI_DOUBLE, MPI_SUM, 0, comm);

	finish = MPI_Wtime();

	local_elapsed = finish - start;

	MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

	if(my_rank == 0){
		for (i = 0; i < numOutputs; ++i)
		{
			printf("Output%i = %f\n", i, out[i]/out[i+1]);
		}
		printf("elapsed = %f\n", elapsed);
	}

	/*DEBUGING--------------------------------------------------------*/

	#ifdef DEBUG
	printf("my_rank = %i Num. Inputs = %i\n", my_rank, numInputs);
	printf("my_rank = %i Num. Outputs = %i\n", my_rank, numOutputs);
	printf("my_rank = %i Num. Total Rules = %i\n", my_rank, numRules);
	printf("my_rank = %i My Total Rules = %i\n", my_rank, local_n);
	// for (i = 0; i < inputs_sz[0]; ++i)
	// {
	// 	printf("%0.1f\n", inputs[0][i]);
	// }
	#endif



	MPI_Finalize();
	return 0;
}