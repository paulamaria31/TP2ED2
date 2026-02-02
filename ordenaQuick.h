#ifndef ordenaQuick_h
#define ordenaQuick_h

#include "Aluno.h"
#include <stdio.h>
#include <stdbool.h>

#define TAM_AREA 10 // Memória interna para o Quicksort Externo (máx. 10 registros)


typedef struct {
    long long leituras;     // transferências (leitura) Memória Externa -> Interna
    long long escritas;     // transferências (escrita) Memória Interna -> Externa
    long long comparacoes;  // comparações de chaves (nota)
} QuickMetrics;

void QuickResetMetrics(void);
QuickMetrics QuickGetMetrics(void);

typedef struct {
    Aluno itens[TAM_AREA+1];
    int n;
} TipoArea;

void QuicksortExterno(FILE **ArqLi, FILE **ArqEi, FILE **ArqLEs, int Esq, int Dir);

#endif