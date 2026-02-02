#include "ordena2F.h"
#include <stdlib.h>
#include <string.h>

// Contadores para o relatório
long transferenciasLeitura = 0;
long transferenciasEscrita = 0;
long comparacoesChaves = 0;

void refazerHeap(RegistroHeap heap[], int i, int n) {
    int menor = i;
    int esq = 2 * i + 1;
    int dir = 2 * i + 2;

    // Critério: primeiro compara se está marcado, depois a nota
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
        refazerHeap(heap, menor, n);
    }
}

void faseGeracaoBlocos(FILE* arq, int* numBlocos) {
    RegistroHeap heap[MEM_TAM];
    FILE* fitas[F];
    char nomeFita[20];
    
    for(int i=0; i<F; i++) {
        sprintf(nomeFita, "fita%d.bin", i + 1);
        fitas[i] = fopen(nomeFita, "wb");
    }

    // Inicializa o Heap
    int lidos = 0;
    for(int i=0; i<MEM_TAM && !feof(arq); i++) {
        if(fread(&heap[i].aluno, sizeof(Aluno), 1, arq)) {
            heap[i].marcado = 0;
            lidos++;
            transferenciasLeitura++;
        }
    }

    int fitaAtual = 0;
    int elementosNoHeap = lidos;
    int marcados = 0;
    Aluno ultimoGravado;

    // Constrói heap inicial
    for (int i = MEM_TAM / 2 - 1; i >= 0; i--) refazerHeap(heap, i, elementosNoHeap);

    while (elementosNoHeap > 0) {
        // Grava o menor no bloco atual
        ultimoGravado = heap[0].aluno;
        fwrite(&ultimoGravado, sizeof(Aluno), 1, fitas[fitaAtual]);
        transferenciasEscrita++;

        // Tenta ler próximo
        if (fread(&heap[0].aluno, sizeof(Aluno), 1, arq)) {
            transferenciasLeitura++;
            comparacoesChaves++;
            if (heap[0].aluno.nota < ultimoGravado.nota) {
                heap[0].marcado = 1;
                marcados++;
            }
        } else {
            // Se o arquivo acabou, reduz o heap
            heap[0] = heap[elementosNoHeap - 1];
            elementosNoHeap--;
        }

        // Se todos marcados, fechou um bloco
        if (marcados == elementosNoHeap && elementosNoHeap > 0) {
            for(int i=0; i<elementosNoHeap; i++) heap[i].marcado = 0;
            marcados = 0;
            (*numBlocos)++;
            fitaAtual = (fitaAtual + 1) % F; // Distribui para a próxima fita
        }
        
        if(elementosNoHeap > 0) refazerHeap(heap, 0, elementosNoHeap);
    }

    for(int i=0; i<F; i++) fclose(fitas[i]);
    (*numBlocos)++; // Conta o último bloco
}

void ordernarArquivo2F(int quantidade, FILE* arq) {
    int numBlocos = 0;
    faseGeracaoBlocos(arq, &numBlocos);
    // Aqui viria a chamada da intercalação balanceada...
    printf("Gerados %d blocos usando Seleção por Substituição.\n", numBlocos);
    printf("Transferencias - Leitura: %ld, Escrita: %ld\n", transferenciasLeitura, transferenciasEscrita);
}