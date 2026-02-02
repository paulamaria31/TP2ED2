#ifndef ordena2F_h
#define ordena2F_h

#include "Aluno.h"
#include <stdio.h>
#include <stdbool.h>

#define F 10 
#define MEM_TAM 10 

typedef struct {
    Aluno aluno;
    int marcado; 
} RegistroHeap;

// Protótipos das funções
void ordernarArquivo2F(int quantidade, FILE* arq);
void faseGeracaoBlocos(FILE* arq, int* numBlocos);
void faseIntercalacao2F(int numBlocos);
void intercalarBlocos(int fitaMin, int fitaMax, int fitaDestinoIdx);
void refazerHeap(RegistroHeap heap[], int i, int n);

#endif