#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdarg.h>
#include "TuringMachine.h"

#define NUM_OF_TAPES 2

/*
    Read a string from file. If fullLine is 0 then 
    the next string will be read. Otherwise, it will
    be read the rest of the line (or the whole line
    if the cursor is at the begining of the line)
*/
char* fReadString(FILE *file, size_t fullLine)
{
	char *s = NULL, c, *tmp;
	int crt_index = 0, buff_size = 0;

	// Skip whitespaces until EOF
	while ((c = fgetc(file)) == ' ' ||
		c == '\n' || c == '\r' || 
		c == EOF)
	{
		if (c == EOF)
			return NULL;
	}

	while (((c != ' ' || fullLine) && c != '\n' && c != '\r')
		&& c != EOF)
	{
		// Alloc memory for s if needed
		if (!s)
		{
			buff_size = 2;
			s = (char*)malloc(buff_size * sizeof(char));
			if (!s)
				return NULL;
		}

		// Increase buffer size if needed
		if (crt_index >= buff_size - 1)
		{
			buff_size *= 2;
			tmp = (char*)realloc(s, buff_size);
			if (!tmp) 
			{
				s[crt_index] = '\0';
				return s;
			}
			s = tmp;
		}

		// Put the character into s
		s[crt_index++] = c;
		c = fgetc(file);
	}

	// Put the terminator of the string
	if (s)
		s[crt_index] = '\0';

	return s;
}

/*
    Read N state names form file and 
    store in an array K
*/
char** fReadStates(FILE *file, int N)
{
	int i, j;
	char **K;
	
	K = (char**)malloc(N * sizeof(char*));
	if (!K)
		return NULL;

	// Read states set
	for (i = 0; i < N; i++)
	{
		K[i] = fReadString(file, 0);
		if (!K[i])
		{
			for (j = 0; j < i; j++)
				free(K[j]);
			return NULL;
		}
	}

	return K;
}

/*
    Read final state array and return an array of 
    pointers to state form K.
*/
char** fReadFinalStates(FILE *file, char **K, int N, int M)
{
	int i;
	char **F, *tmp = NULL;

	F = (char**)calloc(M, sizeof(char*));
	if (!F)
		return NULL;
	
	// Read final states
	for (i = 0; i < M; i++)
	{
		tmp = fReadString(file, 0);
		if (!tmp)
			return NULL;

		// Find state in K
		F[i] = findState(tmp, K, N);
		free(tmp);

		if (F[i] == NULL)
			return NULL;
	}

	return F;
}

/*
    Read initial state and find it in K.
    Return its address.
*/
char* fReadInitialState(FILE *file, char **K, int N)
{
	char *tmp;

	// Read initial state
	tmp = fReadString(file, 0);
	if (!tmp)
		return NULL;

	// Find initial state in K
	char *s0 = findState(tmp, K, N);
	free(tmp);

	return s0;
}

/*
    Read numLines lines form file. 
    Used to read delta. Returns an array of strings.
*/
char** fReadLines(FILE *file, int numLines)
{
	int i, j;
	char **L;

	L = (char**)malloc(numLines * sizeof(char*));
	if (!L)
		return NULL;

	for (i = 0; i < numLines; i++)
	{
		L[i] = fReadString(file, 1);
		if (!L[i])
		{
			for (j = 0; j < i; j++)
				free(L[j]);
			return NULL;
		}
	}

	return L;
}

/* 
    Read each tape form file. 
    Return an array of strings (tapes).
*/
char** fReadTapes(FILE *file, int num)
{
	int i, j;
	char **tapes;

	tapes = (char**)malloc(NUM_OF_TAPES * sizeof(char*));
	if (!tapes)
		return NULL;

	for (i = 0; i < NUM_OF_TAPES; i++) 
	{
		// Store each line in a string
		tapes[i] = fReadString(file, 1);
		if (!tapes[i])
		{
			for (j = 0; j < i; j++)
				free(tapes[j]);
			return NULL;
		}
	}

	return tapes;
}

/*
    This function frees all variables given as
    parameters. It will return number of successfully 
    done frees. 
*/
int freeAll(char *format, ...) 
{
	int n = 0;
	char *p;
	va_list valist;
	va_start(valist, format);

	for (p = format; *p != '\0' || p[1] != '\0' || p[2] != '\0'; p++)
	{
		if (*p != '%')
			continue;

		if (p[1] == 's' && p[2] == 'a')		// array of strings
		{	
			char ***strings = va_arg(valist, char***);
			int num = va_arg(valist, int);
			freeArray(strings, num);
			n++;
		}
		else if (p[1] == 's' && p[2] == 's')	// 1 string
		{
			char **string = va_arg(valist, char**);
			if (*string) 
			{
				free(*string);
				*string = NULL;
				n++;
			}
		}
		else break;
	}	

	va_end(valist);

	return n;
}

void simulateTM(FILE *output, int N, char **K, int M, char **F, char *s0,
	int P, char **D, int NumTapes, char **tapes)
{
	// Init a Turing Machine
	TM *tm = InitTM(N, K, M, F, s0, P, D, NumTapes, tapes);
	if (!tm) 
	{
		freeAll("%sa%sa%ss%sa", &K, N, &D, P, &F, &tapes, NumTapes);
		return;
	}

	// Free the instruction array
	freeArray(&D, P);

	// Run the Turing Machine
	runTM(output, tm);

	// Distroy Turing Machine and release memory
	distroyTM(&tm);
}

/*
    Error codes:
       -1 :	allocation error or address not found
        0 :	success
	1 :	too many states or less than 0
	2 :	more final states than stats or less than 0
	3 :	too many transitions or less than 0

	TODO: check for memory leaks...
	
*/
int getDataFromFile(FILE *input, FILE *output)
{
	int N, M, P;
	char **K, **F, **D, *s0, **tapes;

	// Read number of states
	fscanf(input, "%d", &N);
	if (N < 1 || N > 1000)
		return 1;

	// Read states set
	K = fReadStates(input, N);
	if (!K)
		return -1;

	// Read number of final states
	fscanf(input, "%d", &M);
	if (M < 1 || M > N)
		return 2;

	// Read final states
	F = fReadFinalStates(input, K, N, M);
	if (!F)
	{
		freeAll("%sa", &K, N);
		return -1;
	}

	// Read initial state
	s0 = fReadInitialState(input, K, N);
	if (!s0)
	{
		freeAll("%sa", &K, N);
		free(F);
		return -1; 
	}

	// Read number of transitions
	fscanf(input, "%d", &P);
	if (P < 1 || P > 10000)
		return 3;

	// Read transitions
	D = fReadLines(input, P);
	if (!D)
	{
		freeAll("%sa", &K, N);
		return -1;
	}

	tapes = fReadTapes(input, NUM_OF_TAPES);
	if (!tapes)
	{
		freeAll("%sa%sa", &K, N, &D, P);
		return -1;
	}

	// Simulate the Turing Machine
	simulateTM(output, N, K, M, F, s0, P, D, NUM_OF_TAPES, tapes);

	return 0;
}

int main() 
{
	FILE *input, *output;
	input = fopen("task1.in", "rt");
	output = fopen("task1.out", "wt");

	if (input && output) 
	{
		getDataFromFile(input, output);
	}

	if (input)
		fclose(input);
	if (output)
		fclose(output);

	return 0;
}