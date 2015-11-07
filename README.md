# fuzzy-mpi
Implementation of fuzzy logic for distributed memory. (MPI Based).

##How to use

###instaling MPI

```
sudo apt-get isntall openmpi1.5-bin openmpi1.5-doc libopenmpi1.5-dev
```

### buiding

```
mpicc fuzzy.c fuzzy-loader.c fuzzy-core.c -o fuzzy

```

#### options

-DGENERATE_DATA  -- random inputs

-DDEBUG -- debug mode

### runing

```
mpiexec -n <number of core> ./fuzzy study-cases/testes.fis

```

###fuzzy-loader API

Configure your fuzzy sistem based on .fis file from MATLAB&reg;

```C
//Jumping pointer line in file
void next_line(
	FILE* fp //(in) file pointer
);
//packing input and output structure from file
float* pack_io(
	FILE* fp, //(in) file pointer
	int* sz   //(out) structure size
);

float* load_input(
	FILE* fp, //(in) file pointer
	int* sz   //(out) structure size
);

float* load_output(
	FILE* fp, //(in) file pointer
	int* sz   //(out) structure size
);

//loag rule structure from file
int* load_rules(
	FILE *fp,       //(in) file pointer
	int numInputs,  //(in) number of inputs
	int numOutputs, //(in) number of outputs
	int numRules,   //(in) number of rules
	int *total      //(out) total data read
);

//return number of inputs
int get_numInputs(FILE* fp);

//return number of outputs
int get_numOutputs(FILE* fp);

//return number of rules
int get_numRules(FILE* fp);
```

### fuzzy-core API

Utils function to computing fuzzy data

```C
float max(float _a, float _b);

float min(float _a, float _b);

float orOp(float _a, float _b);

float andOp(float _a, float _b);

float trimf(float _value, float _points[]);

float trapmf(float _value, float _points[]);

float fuzzify(float _value, float* _points, int size);

void defuzzify( double _value, float* _rules, int _size, double* uX, double* u);
```
