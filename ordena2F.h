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

void ordenarArquivo2F(int quantidade, FILE* arq, bool imprimir);
void faseGeracaoBlocos(FILE* arq, int* numBlocos, int quantidade);
void faseIntercalacao2F(int numBlocos);
void intercalarBlocos(int fitaMin, int fitaMax, int fitaDestinoIdx);
void refazerHeap2F(RegistroHeap heap[], int i, int n);

#endif