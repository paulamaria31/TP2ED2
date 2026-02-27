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

// Protótipo ajustado para a main (2).c
void ordenarArquivo2F(int quantidade, FILE* arq, bool imprimir);

#endif