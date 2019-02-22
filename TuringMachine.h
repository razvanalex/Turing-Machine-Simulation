#ifndef _Turing_Machine_
#define _Turing_Machine_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX_LEN_TAPE 10000

/* Positioning the tape: L - Left; R - Right; H - Halt */
typedef enum { L, R, H } position;

/* Next state struct */
typedef struct {
	char *state;		// Name of the state
	char *letters;		// Letters from tapes
	position *pos;		// Movement of tapes
} TNextState;

/* Letters struct */
typedef struct crtLetters{
	char letter;			// Current letter
	struct crtLetters *childLetter;	// Child letter
	struct crtLetters *sibLetter;	// Siblings letter
	TNextState *nextState;		// Next state
} TCrtLetters;

/* Transition struct */
typedef struct {
	char *crtState;		// Current state name
	TCrtLetters *stateDef;	// All transitions of the associated state 
} TTransition;

/* Turing Machine struct */
typedef struct {
	char **K;		// Vector of states given as strings
	unsigned int numK;	// Number of states
	char **F;		// Vector of final states
	unsigned int numF;	// Number of final states
	char *crtState;		// Current state name
	TTransition **delta;	// Tree of instructions
	unsigned int numInstr;	// Number of instructions
	char **tapes;		// Vector of tapes of TM given as strings
	int *headTapes;		// Positions of all head tapes
	unsigned int numTapes;	// Number of tapes
} TM;

// Initialize a Turing Machine
TM* InitTM(unsigned int numK, char **K,
	unsigned int numF, char **F, char *s0,
	unsigned int numInstr, char **delta,
	unsigned int numTapes, char **tapes);

void distroyTM(TM** tm);
void freeArray(char ***strs, int n);
char* findState(char *state, char **K, unsigned int N);
void runTM(FILE *output, TM *t);

#endif