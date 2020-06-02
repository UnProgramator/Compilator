#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "token.h"
#include "alex.h"
#include "ansin.h"
#include "externfunc.h"
#include "mv.h"
#include "gc.h"



#define BUFFSIZE 4096

size_t fileSize(const char* filepath) { /*surprinzator merge si pe windows*/
	struct stat st;
	stat(filepath, &st);
	return st.st_size;
}

char* getInputBuffer(const char* filePath) {
	char* FileBuffer;
	size_t bytesRead;
	size_t buffsize;
	FILE* inputFilePointer = NULL;

	if ((inputFilePointer = fopen(filePath, "rb")) == NULL) {
		printf("Eroare la deschiderea fisierului! verificati calea fisierului de intrare");
		exit(2);
	}

	buffsize = fileSize(filePath);

	if ((FileBuffer = (char*)malloc(buffsize + 1)) == NULL) {
		perror("Memorei insuficienta");
		exit(-1);
	}

	if ((bytesRead = fread(FileBuffer, sizeof(char), buffsize, inputFilePointer)) < buffsize) {
		if (!feof(inputFilePointer)) {
			perror("Eroare la citirea din fisier. Verificati permisiuniile fisierului!");
		}
	}
	fclose(inputFilePointer);
	FileBuffer[bytesRead] = '\0';
	return FileBuffer;
}

char* createName(char*const  path, const char* fileName) {
	static char v[100];
	strcpy(v, path);
	strcat(v, fileName);
	return v;
}  

void seeAll(const char* const filepath) {
	char file[4] = "0.c";
	char* filename;
	char* inputFileBuffer = NULL;
	for (; file[0] <= '9'; file[0]++) {
		filename = createName(filepath, file);
		printf("%s\n", filename);
		inputFileBuffer = getInputBuffer(filename);
		//printf("%s\n", inputFileBuffer);
		ALEX(inputFileBuffer);
		free(inputFileBuffer);

		printf("Codul din fisierul %s este corect din punct de vedere lexical\n", file);

		if (!ANSIN(getTockensList())) {
			exit(-20);
		}

		printf("Codul din fisierul %s este corect din punct de vedere semantic\n", file);
		freeList(getTockensList());
	}
}

void TextMV() {
	initExtFunc();
	mvTest();
	run(instructions);
}

void testTot(int argc, char* argv[]) {
	char* inputFileBuffer = NULL;
	char* filename;

	if (argc == 1) {
		printf("Eroare la apel, trebuie apelat cu numele fisierului de test");
		exit(1); // codul de eroare pentru apel necorespunzator este 1
	}

	const char fis[] = "gc_test.c";

	filename = createName(argv[1], fis);
	printf("%s\n", filename);
	inputFileBuffer = getInputBuffer(filename);
	//printf("%s\n", inputFileBuffer);
	ALEX(inputFileBuffer);
	free(inputFileBuffer);
	printAllTokens();
	//printf("Codul din fisierul %s este corect din punct de vedere lexical\n", fis);

	initExtFunc();

	if (!ANSIN(getTockensList())) {
		exit(-20);
	}

	//partea de testare a codului generat
	printf("\noperations\n");
	printInstr(labelMain);
	printf("\n\nrun\n\n");
	run(labelMain);
}

int main(int argc, char* argv[]) {
	testTot(argc, argv);

	return 0;
}	