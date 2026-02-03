#ifndef ordenaF1_h
#define ordenaF1_h
//Variavel definida
#define TAM_VET 19
#include "Aluno.h"
#include <stdbool.h>
#include <stdio.h>

//Struct para receber os daods
typedef struct Registro
{
    Aluno aluno;
    int marcado;
    int idFitaOrigem;
}Registro;

void ordenarArquivo(int quantidade, FILE* arq, bool imprimir);
void redistribuir();
int contarBlocos(const char* nomeArquivo);
void heap(Registro alunos[], int n);
void intercalacao();
void refazerHeap (Registro alunos[], int i, int n);
bool ehMaior(Registro a, Registro b);
void lerRegistro(int quantidade, FILE* arq);
void imprime_resultado_final(const char* arquivo, int qtd);


#endif