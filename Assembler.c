#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#define MAX 21  // including file format?
#define MAXREAD 256


// print a message and quit program in case of errors
void Exit(char *msg) {
	printf("%s", msg);
	exit(1);
}

// A utility function to convert a decimal number to binary
int *numberToBinary(int arr[], int num) {
	int remainder;


	while (num != 0) {
		remainder = (int)(log2(num));
		arr[16 - remainder - 1] = 1;
		num = num % ((int)pow(2, remainder));
	}

	return arr;
	//return numberToBinary(arr, (num % (int)pow(2, remainder)));
}

void AInstTranslate(int address, FILE *wf) {
	int arr[16] = { 0 }, *binaryarr, i;
	binaryarr = numberToBinary(arr, address);

	for (i = 0; i < 16; i++)
		fprintf(wf, "%d", *(binaryarr+i));
	fputc('\n', wf);
}


void TranslateDest(char dest[], int *outputarr) {
	int i;
	
	for (i = 0; i < 3; i++) {
		if (dest[i] == 'A')
			outputarr[10] = 1;

		else if (dest[i] == 'D')
			outputarr[11] = 1;

		else if (dest[i] == 'M')
			outputarr[12] = 1;
	}
}

void TranslateComp(char comp[], int *outputarr) {
	char c[7] = { 0 };
	int i, j = 4;

	if (comp[0] == '0')
		strcpy(c, "101010");

	else if(comp[0] == '1')
		strcpy(c, "111111");

	else if (comp[0] == '-') {
		if (comp[1] == '1')
			strcpy(c, "111010");
		
		else if (comp[1] == 'D')
			strcpy(c, "001111");

		else if (comp[1] == 'A' || comp[1] == 'M')
			strcpy(c, "110011");
	}

	else if (comp[0] == 'D') {
		if (comp[1] == '+') {
			if (comp[2] == '1')
				strcpy(c, "011111");

			else if (comp[2] == 'A' || comp[2] == 'M')
				strcpy(c, "000010");
		}

		else if (comp[1] == '-') {
			if (comp[2] == '1')
				strcpy(c, "001110");

			else if (comp[2] == 'A' || comp[2] == 'M')
				strcpy(c, "010011");
		}

		else if (comp[1] == '&') {
			strcpy(c, "000000");
		}

		else if (comp[1] == '|') {
			strcpy(c, "010101");
		}

		else
			strcpy(c, "001100");
	}

	else if (comp[0] == '!') {
		if (comp[1] == 'D')
			strcpy(c, "001101");

		else if (comp[1] == 'A' || comp[1] == 'M')
			strcpy(c, "110001");
	}

	else if (comp[0] == 'A' || comp[0] == 'M') {
		if (comp[1] == '+') {
			strcpy(c, "110111");
		}

		else if (comp[1] == '-') {
			if (comp[2] == '1')
				strcpy(c, "110010");

			else if (comp[2] == 'D')
				strcpy(c, "000111");
		}

		else
			strcpy(c, "110000");
	}

	for (i = 0; i < 6; i++) {
		if (c[i] == '0') outputarr[j + i] = 0;
		else outputarr[j + i] = 1;
	}
}


void TranslateJump(char jump[], int *outputarr) {
	char jumpCode[4] = { 0 };
	int i;

	if (strcmp(jump, "JGT") == 0)
		strcpy(jumpCode, "001");

	else if (strcmp(jump, "JEQ") == 0)
		strcpy(jumpCode, "010");

	else if (strcmp(jump, "JGE") == 0)
		strcpy(jumpCode, "011");

	else if (strcmp(jump, "JLT") == 0)
		strcpy(jumpCode, "100");

	else if (strcmp(jump, "JNE") == 0)
		strcpy(jumpCode, "101");

	else if (strcmp(jump, "JLE") == 0)
		strcpy(jumpCode, "110");

	else if (strcmp(jump, "JMP") == 0)
		strcpy(jumpCode, "111");

	for (i = 0; i < 3; i++) {
		if (jumpCode[i] == '0') outputarr[i + 13] = 0;
		else outputarr[i + 13] = 1;
	}
}

void CInstTranslate(char instruction[], FILE *outf) {
	int i=0, j;
	char dest[4] = { 0 };
	char comp[4] = { 0 };
	char jump[4] = { 0 };
	int outputInst[16] = { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	//Check if dest is null
	if (strchr(instruction, '=') != NULL) {
		// Translate dest. Scan the instruction array, and save
		// every char before the '=' to dest[], then call func TranslateDest
		// to translate the chars we got to machine code and write it to the output file
		i = 0;
		while (instruction[i] != '=') {
			dest[i] = instruction[i];
			i++;
		}
		i++;  // skip '='
		TranslateDest(dest, outputInst);
	}

	// Translate comp. continue scanning the instruction array from 
	// the i(th) index, and save every char to comp[] until the end
	// of instruction array is reached. then call TranslateComp() to translate
	// the chars we got to machine code and write it to the output file
	j = 0; 
	while (instruction[i] != '\n' && instruction[i] != ';' && instruction[i] != '\0') {
		if (instruction[i] == 'M')
			outputInst[3] = 1; // a = 1

		comp[j] = instruction[i];
		i++;
		j++;
	}
	TranslateComp(comp, outputInst);

	// Translate jump
	if (instruction[i] == ';') {
		i++; // skip ';'
		for (j = 0; j < 3; j++)
			jump[j] = instruction[i+j];

		TranslateJump(jump, outputInst);
	}


	// write translation (outputInst) to file
	for (i = 0; i < 16; i++) {
		if (outputInst[i] == 1) fputc('1', outf);
		else fputc('0', outf);
	}
	fputc('\n', outf);
}

void ReadAndTranslateF(FILE *rf, FILE *wf) {
	int address;
	char buffer[MAXREAD];

	while (fgets(buffer, 109, rf)) {
		if (buffer[0] == '/' || buffer[0] == '\n')
			continue;

		else if (buffer[0] == '@') {
			buffer[0] = '0';
			address = atoi(buffer);
			AInstTranslate(address, wf);
		}

		else if (buffer[0] == 'A' || buffer[0] == 'D' || buffer[0] == 'M' || buffer[0] == '0' || buffer[0] == '-' || buffer[0] == '!')
			CInstTranslate(buffer, wf);
	}
}


int main() {
	FILE *asmf, *writef;
	char fname[MAX] = { 0 };  // input file name
	char wfname[MAX + 3] = { 0 };  // output file name
	int i;

	// Get assembly file from the user
	printf("Please enter assembly file name: ");
	scanf("%s", fname);
	
	// set the output file name to be the input file name
	// but we change the file format from .asm to .hack
	for (i = 0; i < MAX; i++) {
		if (fname[i] != '.')
			wfname[i] = fname[i];

		else
			break;
	}
	strcat(wfname, ".hack");  // add .hack to the end of the string

	// open the input and output files
	asmf = fopen(fname, "rt");
	if (asmf == NULL) Exit("Cannot open file.");

	writef = fopen(wfname, "wt");
	if (writef == NULL) Exit("Cannot create file");

	// Translate the input file from assembly to machine code
	// and write translation to the output file
	ReadAndTranslateF(asmf, writef);

	//Print the output file is ready and close the files
	printf("Output file %s is now available.\n", wfname);
	fclose(asmf);
	fclose(writef);

	return 0;
}