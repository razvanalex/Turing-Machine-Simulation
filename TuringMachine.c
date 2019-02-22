#include "TuringMachine.h"

/*
    This function finds all elements from array. In returns
    a positive number if all elements are found and negative
    number if there are only some elements in array.
*/
int findStringsInArray(char **array, unsigned int size_array,
	char **elements, unsigned int num_elements)
{
	unsigned int i, j;
	int num_found = 0;

	for (i = 0; i < num_elements; i++)
	{
		for (j = 0; j < size_array; j++)
		{
			if (!strcmp(elements[i], array[j]))
			{
				num_found++;
				break;
			}
		}
	}

	if (num_found == num_elements)
		return num_found;
	return -num_found;
}

/*
    Check if the parameters which may caracterise a
    Turing Machine are corect given.
    Error codes:
        1 :	more final states than states
        2 :	some final states are not in K
        3 :	initial state is not from K
        4 :	number of tapes cannot be 0
*/
int CheckValidTM(unsigned int numK, char **K,
	unsigned int numF, char **F, char *s0,
	unsigned int numInstr, char **delta,
	unsigned int numTapes, char **tapes)
{
	if (numK < numF)
		return 1;

	if (findStringsInArray(K, numK, F, numF) <= 0)
		return 2;

	if (findStringsInArray(K, numK, &s0, 1) <= 0)
		return 3;

	if (numTapes == 0)
		return 4;

	return 0;
}

/*
    Read next state form Delta[i]. Returns a struture TNextState.
*/
TNextState* ReadNextState(char* str, unsigned int numTapes, int offset)
{
	char buff[100], _pos;
	unsigned int i;

	str += offset;
	sscanf(str, "%s", buff);

	// Alloc memory for all fields needed
	char *state = (char*)malloc((strlen(buff) + 1) * sizeof(char));
	if (!state)
		return NULL;

	strcpy(state, buff);
	str += strlen(buff) + 1;

	char *letters = (char*)malloc(numTapes * sizeof(char));
	if (!letters)
	{
		free(state);
		return NULL;
	}

	position *pos = (position*)malloc(numTapes * sizeof(position));
	if (!pos)
	{
		free(letters);
		free(state);
		return NULL;
	}

	// Get letters and positions
	for (i = 0; i < numTapes; i++)
	{
		sscanf(str, "%c", &letters[i]);
		str += 2;
		sscanf(str, "%c", &_pos);
		pos[i] = (_pos == 'L') ? L : ((_pos == 'R') ? R : H);

		str += 2;
	}

	// Create a TNextState instance
	TNextState *nextState = (TNextState*)malloc(sizeof(TNextState));
	if (!nextState)
	{
		free(state);
		free(letters);
		free(pos);
		return NULL;
	}

	nextState->state = state;
	nextState->letters = letters;
	nextState->pos = pos;

	return nextState;
}

/* 
    Create a new letter for tree
*/
TCrtLetters* createLetter(char letter)
{
	TCrtLetters *l = (TCrtLetters*)malloc(sizeof(TCrtLetters));
	if (!l)
		return NULL;

	l->letter = letter;
	l->childLetter = NULL;
	l->nextState = NULL;
	l->sibLetter = NULL;

	return l;
}


/*
    Read current state form Delta[i]. Returns a struture TState.
*/
TCrtLetters* ReadCrtState(char** delta, unsigned int numInstr, 
	unsigned int numTapes, char *crtState)
{
	unsigned int i, j, offset;
	char buff[100], letter;

	TCrtLetters *crtLetter = NULL, 
		*antLetter = NULL, 
		*firstLetter = NULL;

	for (i = 0; i < numInstr; i++)
	{
		sscanf(delta[i], "%s", buff);
		offset = strlen(buff) + 1;

		// Find state
		if (strcmp(buff, crtState))
			continue;

		crtLetter = firstLetter;

		// Add letters to crt state tree
		for (j = 0; j < numTapes; j++)
		{
			sscanf(delta[i] + offset, "%c", &letter);
			offset += 2;

			if (!crtLetter)
			{
				crtLetter = createLetter(letter);
				if (!crtLetter)
					continue;

				if (!antLetter)
					firstLetter = crtLetter;
				else antLetter->childLetter = crtLetter;

				antLetter = crtLetter;
				crtLetter = crtLetter->sibLetter;
				continue;
			}

			// Find siblings of current j level
			for (;
				crtLetter && crtLetter->letter != letter;
				antLetter = crtLetter,
				crtLetter = crtLetter->sibLetter);

			if (!crtLetter)
			{
				crtLetter = createLetter(letter);
				if (!crtLetter)
					continue;

				antLetter->sibLetter = crtLetter;
			}

			antLetter = crtLetter;
			crtLetter = crtLetter->childLetter;
		}

		// Finally, append the next state
		if (antLetter)
		{
			antLetter->nextState = ReadNextState(delta[i],
				numTapes, offset);
		}

		antLetter = NULL;
	}

	return firstLetter;
}


/*
	Convert delta from char** to TTransition. It will not
	free memory form char**. Returns a new TTransition.
*/
TTransition** createDelta(char **delta, unsigned int numInstr, 
	unsigned int numTapes, char **K, unsigned int numStates)
{
	unsigned int i;
	TTransition **newDelta;

	newDelta = (TTransition**)malloc(numStates * sizeof(TTransition*));
	if (!newDelta)
		return NULL;

	for (i = 0; i < numStates; i++)
	{
		newDelta[i] = (TTransition*)malloc(sizeof(TTransition));
		if (!newDelta[i])
			continue;

		newDelta[i]->crtState = K[i];
		newDelta[i]->stateDef = ReadCrtState(delta, numInstr, 
			numTapes, K[i]);
	}

	return newDelta;
}

/*
    Init all head tapes to the first letter (position 1)
*/
int *initHeadTapes(unsigned int numTapes)
{
	int *ht = (int*)calloc(numTapes, sizeof(int));
	if (!ht)
		return NULL;
	
	int i;
	for (i = 0; i < numTapes; i++)
		ht[i] = 1;

	return ht;
}

/*
    Release memory for an array.
*/
void freeArray(char ***strs, int n)
{
	if (!*strs)
		return;

	int i;
	for (i = 0; i < n; i++)
	{
		if ((*strs)[i])
			free((*strs)[i]);
	}

	free(*strs);
	*strs = NULL;
}


/*
    Release memory for a tree of TCrtLetters
*/
void freeLetters(TCrtLetters *l)
{
	if (!l)
		return;
	freeLetters(l->childLetter);
	freeLetters(l->sibLetter);
	
	if (l->nextState)
	{
		free(l->nextState->letters);
		free(l->nextState->pos);
		free(l->nextState->state);
		free(l->nextState);
	}

	free(l);
	l = NULL;
}

/*
    Release memory for Delta given as TTransition.
*/
void freeDelta(TTransition ***delta, unsigned int numStates)
{
	if (!*delta)
		return;

	unsigned int i;
	for (i = 0; i < numStates; i++)
	{
		if ((*delta)[i])
		{
			freeLetters((*delta)[i]->stateDef);
			free((*delta)[i]);
		}
	}

	free(*delta);
	*delta = NULL;
}


/*
     Init a new Turing Machine
*/
TM* InitTM(unsigned int numK, char **K,
	unsigned int numF, char **F, char *s0,
	unsigned int numInstr, char **delta,
	unsigned int numTapes, char **tapes)
{
	if (CheckValidTM(numK, K, numF, F, s0,
		numInstr, delta,
		numTapes, tapes))
	{
		return NULL;
	}

	// Init Transition function
	TTransition **_delta = createDelta(delta, numInstr, numTapes, K, numK);
	if (!_delta)
		return NULL;

	// Init head tapes
	int *headTapes = initHeadTapes(numTapes);
	if (!headTapes)
	{
		freeDelta(&_delta, numK);
		return NULL;
	}

	// Alloc Turing Machine
	TM *tm = (TM*)malloc(sizeof(TM));
	if (!tm)
	{
		freeDelta(&_delta, numK);
		free(headTapes);
		return NULL;
	}

	// Increase size of tapes
	unsigned int i;
	for (i = 0; i < numTapes; i++)
	{
		char *tmp = (char*)realloc(tapes[i], MAX_LEN_TAPE);
		if (!tmp)
		{
			freeDelta(&_delta, numK);
			free(headTapes);
			free(tm);
			return NULL;
		}
		tapes[i] = tmp;
	}

	tm->numK = numK;
	tm->K = K;
	tm->numF = numF;
	tm->F = F;
	tm->crtState = s0;
	tm->numInstr = numInstr;
	tm->delta = _delta;
	tm->numTapes = numTapes;
	tm->headTapes = headTapes;
	tm->tapes = tapes;

	return tm;
}

/*
    Find state in K and return its address.
*/
char* findState(char *state, char **K, unsigned int N)
{
	unsigned int i;
	for (i = 0; i < N; i++)
		if (!strcmp(state, K[i]))
			return K[i];

	return NULL;
}

/*
	Go to next instruction. It's like fetch-decode-execute
	from CPU.
*/
void moveToInstruction(TM *t)
{
	unsigned int i, j;

	for (i = 0; i < t->numK; i++)
	{
		char *crtState = t->delta[i]->crtState;

		// Find state
		if (strcmp(t->crtState, crtState))
			continue;

		TCrtLetters *crtLetter = t->delta[i]->stateDef;

		for (j = 0; j < t->numTapes; j++)
		{
			int htPos = t->headTapes[j];
			char *l = &t->tapes[j][htPos];

			// Move '\0' to next character and put #.
			if (*l == '\0' &&
				htPos + 1 < MAX_LEN_TAPE)
			{
				*l = '#';
				t->tapes[j][htPos + 1] = '\0';
			}

			// Find crt letters from tape in delta
			for (;
				crtLetter && crtLetter->letter != *l;
				crtLetter = crtLetter->sibLetter);

			if (!crtLetter)
				return;

			if (crtLetter->childLetter)
				crtLetter = crtLetter->childLetter;
		}

		// Execute instruction
		TNextState *next = crtLetter->nextState;
		if (!next)
			return;

		// Go to next state
		t->crtState = next->state;
		for (j = 0; j < t->numTapes; j++)
		{
			int *htPos = &t->headTapes[j];

			// Write letter
			t->tapes[j][*htPos] = next->letters[j];

			// Move tape head
			switch (next->pos[j])
			{
			case R:
				(*htPos)++;
				break;
			case L:
				(*htPos)--;
				if (*htPos < 0)
					*htPos = 0;
				break;
			case H: break;
			default: break;
			}
		}
		break;

	}
}

/*
     Print tapes without '#' on a single line each tape.
*/
void printTapes(FILE *output, TM *t)
{
	unsigned int i, j;
	char *str;

	for (i = 0; i < t->numTapes; i++)
	{
		str = t->tapes[i];
		j = 0;

		while (j < strlen(str))
		{
			// Just ignore #
			if (str[j] == '#')
			{
				j++;
				continue;
			}
			fprintf(output, "%c", str[j]);
			j++;
		}
		fprintf(output, "\n");
	}

}

/*
     Run Turing Machine
*/
void runTM(FILE *output, TM *t)
{
	unsigned int i, sameState;

	// Create a buffer to store crt position of Tapes
	int *crtTapePos = (int*)malloc(t->numTapes * sizeof(int));
	if (!crtTapePos)
	{
		fprintf(output, "An error has occured while "
			"running the Turing Machine\n");
		return;
	}

	// Ignore '#' from the begining
	for (i = 0; i < t->numTapes; i++)
	{
		int *htPos = &t->headTapes[i];
		while (t->tapes[i][*htPos] == '#' &&
			*htPos < MAX_LEN_TAPE)
		{
			(*htPos)++;
			if (*htPos == MAX_LEN_TAPE - 1)
				t->tapes[i][*htPos] = '\0';
		}
	}

	// Run machine until final state
	while (!findState(t->crtState, t->F, t->numF))
	{
		char *crtState = t->crtState;

		// Get current head tapes positions
		for (i = 0; i < t->numTapes; i++)
			crtTapePos[i] = t->headTapes[i];

		// Move to next intruction
		moveToInstruction(t);

		// Check if there is an change
		sameState = 1;
		if (!strcmp(crtState, t->crtState) && 
			!findState(t->crtState, t->F, t->numF))
		{
			for (i = 0; i < t->numTapes; i++)
			{
				if (crtTapePos[i] != t->headTapes[i])
				{
					sameState = 0;
					break;
				}
			}

			// The machine has blocked in a state.
			if (sameState)
			{
				fprintf(output, "The machine has blocked!\n");
				free(crtTapePos);
				return;
			}
		}
	}

	// Print output
	printTapes(output, t);

	// Release memory for buffer
	free(crtTapePos);
}

/*
    Distroy a TM entity. Release memory and make it NULL.
*/
void distroyTM(TM **tm)
{
	freeArray(&(*tm)->K, (*tm)->numK);
	free((*tm)->F);
	free((*tm)->headTapes);
	freeArray(&(*tm)->tapes, (*tm)->numTapes);
	freeDelta(&(*tm)->delta, (*tm)->numK);

	free(*tm);
	*tm = NULL;
}