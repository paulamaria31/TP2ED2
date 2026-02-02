#ifndef ordena2F_h
#define ordena2F_h

#include "Aluno.h"
#include <stdio.h>
#include <stdbool.h>

#define F 10 // Número de fitas de entrada (total 2f = 20)
#define MEM_TAM 10 // Limite de registros na RAM para o Método 1

typedef struct {
    Aluno aluno;
    int marcado; // 0 para bloco atual, 1 para próximo bloco
} RegistroHeap;

// Funções principais
void ordernarArquivo2F(int quantidade, FILE* arq);
void faseGeracaoBlocos(FILE* arq, int* numBlocos);
void faseIntercalacao2F(int numBlocos);

#endif