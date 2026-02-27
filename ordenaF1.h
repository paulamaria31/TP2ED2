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

void ordenarArquivo(int quantidade, FILE* arq, bool imprimir);
void redistribuir();
int contarBlocos(const char* nomeArquivo);
void imprime_resultado_final(const char* arquivo, int qtd);
void heap(Registro alunos[], int n);
void intercalacao();
void refazerHeap (Registro alunos[], int i, int n);
bool ehMaior(Registro a, Registro b);
void lerRegistro(int quantidade, FILE* arq);

#endif