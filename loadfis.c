#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#define MAX_BUFFER 100

void next_line(FILE* fp);
float* load_input(FILE* fp, int* sz);
float* load_output(FILE* fp, int* sz);
int* load_rules(FILE *fp,int numInputs, int numOutputs, int numRules, int *total);
int get_numInputs(FILE* fp);
int get_numOutputs(FILE* fp);
int get_numRules(FILE* fp);

int main(int argc, char* argv[])
{
    if(argc < 2) {
        printf("Usage: %s <fis path>\n", argv[0]);
        exit(1);
    }

    char* path = argv[1];
    FILE* fp;
    int foundline = 0;
    char linebuffer[MAX_BUFFER];
    int i;

    fp = fopen(path, "r");

    int numInputs;
    int numOutputs;
    int numRules;

    float **inputs, **outputs;
    int *inputs_sz, *outputs_sz, *rules, rules_sz;

    numInputs = get_numInputs(fp);
    numOutputs = get_numOutputs(fp);
    numRules = get_numRules(fp);

    int ip1_sz;
    
    inputs = malloc(sizeof(float*)*numInputs);
    inputs_sz = malloc(sizeof(int)*numInputs);
    for(i = 0; i < numInputs; i++)
        inputs[i] = load_input(fp, &inputs_sz[i]);

    outputs = malloc(sizeof(float*)*numOutputs);
    outputs_sz = malloc(sizeof(int)*numOutputs);
    for(i = 0; i < numOutputs; i++)
        outputs[i] = load_output(fp, &outputs_sz[i]); 

    rules = load_rules(fp, numInputs, numOutputs, numRules, &rules_sz);

#ifdef DEBUG
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
#endif

    fclose(fp);



    return 0;
}

void next_line(FILE* fp)
{
    fscanf(fp, "%*s\n");
}

int get_numInputs(FILE* fp)
{
    int foundline  = 0;
    int r;

    while(!foundline){
        foundline = fscanf(fp, "NumInputs=%i\n", &r);
        if(!foundline) next_line(fp);
    }

    return r;
}
int get_numOutputs(FILE* fp)
{
    int foundline  = 0;
    int r;

    while(!foundline){
        foundline = fscanf(fp, "NumOutputs=%i\n", &r);
        if(!foundline) next_line(fp);
    }

    return r;
}
int get_numRules(FILE* fp)
{
    int foundline  = 0;
    int r;

    while(!foundline){
        foundline = fscanf(fp, "NumRules=%i\n", &r);
        if(!foundline) next_line(fp);
    }

    return r;
}

float* load_input(FILE* fp, int* sz)
{
    int status = 0;

    float a,b,c,d;
    int foundline = 0;
    int tmp;
    char type[10];

    int size = 0;
    int offsetR = 0;
    int offsetL = 0;

    //Looking for input zone
    while(!foundline){    
        foundline = fscanf(fp, "[Input%i]\n", &tmp);
        if(!foundline) next_line(fp);
    }

    /*Skiping 3 lines*/
    next_line(fp);
    next_line(fp);
    next_line(fp);
    /*---------------*/
    fscanf(fp, "NumMFs=%i\n", &tmp);


    float* stackR = malloc(sizeof(float));
    float* stackL = malloc(sizeof(float)*(tmp+1));
    float* fullStack;

    stackL[offsetL++] = tmp;
    for(tmp; tmp>0; tmp--){
        foundline = fscanf(fp, "MF%*i=\'%*c\':\'%[a-z]\',[%f %f %f %f]\n", type, &a, &b, &c, &d);
        if(foundline != 5){ /* trimf case, read only 3 floats*/
            next_line(fp);
            size+= 3;
            stackR = realloc(stackR, (sizeof(float))*size);
            stackL[offsetL++] = 3;
            stackR[offsetR++] = a;
            stackR[offsetR++] = b;
            stackR[offsetR++] = c;
            continue; 
        }
        size += 4;
        stackR = realloc(stackR, (sizeof(float))*size);
        stackL[offsetL++] = 4;
        stackR[offsetR++] = a;
        stackR[offsetR++] = b;
        stackR[offsetR++] = c;
        stackR[offsetR++] = d;
    }
    fullStack = malloc(sizeof(float)*(offsetL+offsetR));
    int i;
    for(i = 0; i < offsetL+offsetR; i++){
            fullStack[i] = i < offsetL ? stackL[i] : stackR[i - offsetL];
    }

    *sz = (offsetL+offsetR);

    free(stackL);
    free(stackR);

    return fullStack;
}

float* load_output(FILE* fp, int* sz)
{
    int status = 0;

    float a,b,c,d;
    int foundline = 0;
    int tmp;
    char type[10];

    int size = 0;
    int offsetR = 0;
    int offsetL = 0;

    //looking for Output zone
    while(!foundline){    
        foundline = fscanf(fp, "[Output%i]\n", &tmp);
        if(!foundline) next_line(fp);
    }

    /*Skiping 3 lines*/
    next_line(fp);
    next_line(fp);
    next_line(fp);
    /*---------------*/
    fscanf(fp, "NumMFs=%i\n", &tmp);


    float* stackR = malloc(sizeof(float));
    float* stackL = malloc(sizeof(float)*(tmp+1));
    float* fullStack;

    stackL[offsetL++] = tmp;
    for(tmp; tmp>0; tmp--){
        foundline = fscanf(fp, "MF%*i=\'%*c\':\'%[a-z]\',[%f %f %f %f]\n", type, &a, &b, &c, &d);
        if(foundline != 5){ /* trimf case, read only 3 floats*/
            next_line(fp);
            size+= 3;
            stackR = realloc(stackR, (sizeof(float))*size);
            stackL[offsetL++] = 3;
            stackR[offsetR++] = a;
            stackR[offsetR++] = b;
            stackR[offsetR++] = c;
            continue; 
        }
        size += 4;
        stackR = realloc(stackR, (sizeof(float))*size);
        stackL[offsetL++] = 4;
        stackR[offsetR++] = a;
        stackR[offsetR++] = b;
        stackR[offsetR++] = c;
        stackR[offsetR++] = d;
    }
    fullStack = malloc(sizeof(float)*(offsetL+offsetR));
    int i;
    for(i = 0; i < offsetL+offsetR; i++){
            fullStack[i] = i < offsetL ? stackL[i] : stackR[i - offsetL];
    }

    *sz = (offsetL+offsetR);

    free(stackL);
    free(stackR);

    return fullStack;
}

int* load_rules(FILE *fp,int numInputs, int numOutputs, int numRules, int *total)
{
    int *r;
    int foundline = 0;
    int i,j,k;
    *total = (numInputs+numOutputs+1)*numRules;
    char aux[40];

    r = malloc(sizeof(int)*(*total));

    //looking for the last zone, Rules
    while(!foundline){    
        foundline = fscanf(fp, "%s\n", aux);
        if(!foundline) next_line(fp);
    }


    k = 0;
    for(i = 0; i < numRules; i++){
        for(j = 0; j < numInputs; j++)
            fscanf(fp, "%i ", &r[k++]);

        fscanf(fp, "%c ", aux); //Scaping , char

        for(j = 0; j < numOutputs; j++)
            fscanf(fp, "%i ", &r[k++]);

        fscanf(fp, "(%*i) : %i\n", &r[k++]);
    }

    return r;

}
