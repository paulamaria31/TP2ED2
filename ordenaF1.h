#ifndef ordenaF1_h
#define ordenaF1_h
#define TAM_VET 19
#include "Aluno.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct Registro
{
    Aluno aluno;
    int marcado;
    int idFitaOrigem;
}Registro;

void ordernarArquivo(int quantidade, FILE* arq);
void redistribuir();
void heap(Registro alunos[], int n);
void intercalacao();
void refazerHeap (Registro alunos[], int i, int n);
bool ehMaior(Registro a, Registro b);
void lerRegistro(int quantidade, FILE* arq);

#endif