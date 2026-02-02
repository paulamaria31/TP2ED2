#include "ordena2F.h"
#include <stdlib.h>
#include <string.h>

long transferenciasLeitura = 0;
long transferenciasEscrita = 0;
long comparacoesChaves = 0;

void refazerHeap2F(RegistroHeap heap[], int i, int n) {
    int menor = i;
    int esq = 2 * i + 1;
    int dir = 2 * i + 2;

    if (esq < n) {
        comparacoesChaves++;
        if (heap[esq].marcado < heap[menor].marcado || 
           (heap[esq].marcado == heap[menor].marcado && heap[esq].aluno.nota < heap[menor].aluno.nota))
            menor = esq;
    }
    if (dir < n) {
        comparacoesChaves++;
        if (heap[dir].marcado < heap[menor].marcado || 
           (heap[dir].marcado == heap[menor].marcado && heap[dir].aluno.nota < heap[menor].aluno.nota))
            menor = dir;
    }

    if (menor != i) {
        RegistroHeap temp = heap[i];
        heap[i] = heap[menor];
        heap[menor] = temp;
        refazerHeap2F(heap, menor, n);
    }
}

void faseGeracaoBlocos(FILE* arq, int* numBlocos, int quantidade) {
    RegistroHeap heap[MEM_TAM];
    FILE* fitas[F];
    char nomeFita[20];
    int totalProcessados = 0;

    for(int i=0; i<F; i++) {
        sprintf(nomeFita, "fita%d.bin", i + 1);
        fitas[i] = fopen(nomeFita, "wb");
    }

    int lidos = 0;
    for(int i=0; i<MEM_TAM && totalProcessados < quantidade; i++) {
        if(fread(&heap[i].aluno, sizeof(Aluno), 1, arq)) {
            heap[i].marcado = 0;
            lidos++;
            totalProcessados++;
            transferenciasLeitura++;
        }
    }

    int fitaAtual = 0;
    int elementosNoHeap = lidos;
    int marcados = 0;
    Aluno ultimoGravado;

    for (int i = elementosNoHeap / 2 - 1; i >= 0; i--) refazerHeap2F(heap, i, elementosNoHeap);

    while (elementosNoHeap > 0) {
        ultimoGravado = heap[0].aluno;
        fwrite(&ultimoGravado, sizeof(Aluno), 1, fitas[fitaAtual]);
        transferenciasEscrita++;

        if (totalProcessados < quantidade && fread(&heap[0].aluno, sizeof(Aluno), 1, arq)) {
            transferenciasLeitura++;
            totalProcessados++;
            if (heap[0].aluno.nota < ultimoGravado.nota) {
                heap[0].marcado = 1;
                marcados++;
            } else {
                heap[0].marcado = 0;
            }
        } else {
            heap[0] = heap[elementosNoHeap - 1];
            elementosNoHeap--;
        }

        if (elementosNoHeap > 0 && marcados == elementosNoHeap) {
            for(int i=0; i<elementosNoHeap; i++) heap[i].marcado = 0;
            marcados = 0;
            (*numBlocos)++;
            fitaAtual = (fitaAtual + 1) % F;
        }
        
        if(elementosNoHeap > 0) refazerHeap2F(heap, 0, elementosNoHeap);
    }

    for(int i=0; i<F; i++) fclose(fitas[i]);
    if (lidos > 0) (*numBlocos)++; 
}

void intercalarBlocos(int fitaMin, int fitaMax, int fitaDestinoIdx) {
    FILE* entradas[10];
    char nome[20];
    RegistroHeap memoria[10];
    int fitasAtivas = 0;

    for (int i = 0; i < 10; i++) {
        sprintf(nome, "fita%d.bin", fitaMin + i);
        entradas[i] = fopen(nome, "rb");
        
        if (entradas[i] && fread(&memoria[i].aluno, sizeof(Aluno), 1, entradas[i])) {
            memoria[i].marcado = 0;
            fitasAtivas++;
            transferenciasLeitura++;
        } else {
            memoria[i].marcado = -1;
            if(entradas[i]) fclose(entradas[i]);
            entradas[i] = NULL;
        }
    }

    sprintf(nome, "fita%d.bin", fitaDestinoIdx);
    FILE* saida = fopen(nome, "ab"); 

    while (fitasAtivas > 0) {
        int menorIdx = -1;
        for (int i = 0; i < 10; i++) {
            if (memoria[i].marcado == 0) { 
                if (menorIdx == -1 || memoria[i].aluno.nota < memoria[menorIdx].aluno.nota) {
                    menorIdx = i;
                }
            }
        }

        if (menorIdx == -1) break;

        fwrite(&memoria[menorIdx].aluno, sizeof(Aluno), 1, saida);
        transferenciasEscrita++;

        if (fread(&memoria[menorIdx].aluno, sizeof(Aluno), 1, entradas[menorIdx])) {
            transferenciasLeitura++;
        } else {
            memoria[menorIdx].marcado = -1;
            fitasAtivas--;
            fclose(entradas[menorIdx]);
            entradas[menorIdx] = NULL;
        }
    }
    if (saida) fclose(saida);
}

void faseIntercalacao2F(int numBlocos) {
    int fitaEntradaMin = 1, fitaEntradaMax = 10;
    int fitaSaidaMin = 11, fitaSaidaMax = 20;
    
    while (numBlocos > 1) {
        int blocosGeradosNestaPassada = 0;
        int numIntercalacoes = (numBlocos + 9) / 10;

        for (int i = 0; i < numIntercalacoes; i++) {
            int fitaDestinoIdx = fitaSaidaMin + (i % 10);
            intercalarBlocos(fitaEntradaMin, fitaEntradaMax, fitaDestinoIdx);
            blocosGeradosNestaPassada++;
        }

        int tempMin = fitaEntradaMin;
        int tempMax = fitaEntradaMax;
        fitaEntradaMin = fitaSaidaMin;
        fitaEntradaMax = fitaSaidaMax;
        fitaSaidaMin = tempMin;
        fitaSaidaMax = tempMax;

        numBlocos = blocosGeradosNestaPassada;

        for (int j = fitaSaidaMin; j <= fitaSaidaMax; j++) {
            char nome[20];
            sprintf(nome, "fita%d.bin", j);
            FILE* f = fopen(nome, "wb");
            if(f) fclose(f);
        }
    }
    printf("\n--- Ordenacao Finalizada ---\n");
    printf("O arquivo final ordenado esta na fita%d.bin\n", fitaEntradaMin);
}

void ordernarArquivo2F(int quantidade, FILE* arq) {
    int numBlocos = 0;
    printf("Iniciando Fase 1: Geracao de Blocos (Selecao por Substituicao)...\n");
    faseGeracaoBlocos(arq, &numBlocos, quantidade);
    printf("Sucesso! %d blocos gerados.\n", numBlocos);

    printf("Iniciando Fase 2: Intercalacao Balanceada (2f fitas)...\n");
    faseIntercalacao2F(numBlocos);

    printf("\nResultados do Metodo 1:\n");
    printf("Transferencias de Leitura: %ld\n", transferenciasLeitura);
    printf("Transferencias de Escrita: %ld\n", transferenciasEscrita);
    printf("Comparacoes de Chaves: %ld\n", comparacoesChaves);
}