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

void intercalarBlocos(int fitaMin, int fitaMax, int fitaDestinoIdx) {
    FILE* entradas[10];
    char nome[20];
    RegistroHeap memoria[10];
    int fitasAtivas = 0;

    // 1. Inicialização
    for (int i = 0; i < 10; i++) {
        sprintf(nome, "fita%d.bin", fitaMin + i);
        entradas[i] = fopen(nome, "rb");
        
        if (entradas[i] && fread(&memoria[i].aluno, sizeof(Aluno), 1, entradas[i])) {
            memoria[i].marcado = 0; // 0 significa "FITA ATIVA"
            fitasAtivas++;
            transferenciasLeitura++;
        } else {
            memoria[i].marcado = -1; // -1 significa "FITA VAZIA/FINALIZADA"
            if(entradas[i]) {
                fclose(entradas[i]);
                entradas[i] = NULL;
            }
        }
    }

    sprintf(nome, "fita%d.bin", fitaDestinoIdx);
    FILE* saida = fopen(nome, "ab"); 

    // 2. Loop de Intercalação
    while (fitasAtivas > 0) {
        int menorIdx = -1;
        
        for (int i = 0; i < 10; i++) {
            // CORREÇÃO AQUI: Verifica se a fita ainda tem registros (marcado == 0)
            if (memoria[i].marcado == 0) { 
                if (menorIdx == -1 || memoria[i].aluno.nota < memoria[menorIdx].aluno.nota) {
                    menorIdx = i;
                }
            }
        }

        if (menorIdx == -1) break;

        fwrite(&memoria[menorIdx].aluno, sizeof(Aluno), 1, saida);
        transferenciasEscrita++;

        // 3. Tenta ler o próximo da fita que "venceu"
        if (fread(&memoria[menorIdx].aluno, sizeof(Aluno), 1, entradas[menorIdx])) {
            transferenciasLeitura++;
        } else {
            memoria[menorIdx].marcado = -1; // MARCA COMO FINALIZADA
            fitasAtivas--;
            fclose(entradas[menorIdx]);
            entradas[menorIdx] = NULL;
        }
    }
    if (saida) fclose(saida);
}

void faseIntercalacao2F(int numBlocos) {
    // No seu método, f=10. Fitas 1-10 (entrada) e 11-20 (saída)
    int fitaEntradaMin = 1, fitaEntradaMax = 10;
    int fitaSaidaMin = 11, fitaSaidaMax = 20;
    
    // Enquanto houver mais de um bloco, o arquivo ainda não está totalmente ordenado
    while (numBlocos > 1) {
        int blocosGeradosNestaPassada = 0;
        
        // Calculamos quantas vezes precisaremos rodar o "funil" de 10 caminhos
        // Se temos 100 blocos e lemos 10 por vez, faremos 10 intercalações.
        int numIntercalacoes = (numBlocos + 9) / 10;

        for (int i = 0; i < numIntercalacoes; i++) {
            // Define qual fita de saída vai receber o resultado desta rodada (11 a 20)
            int fitaDestinoIdx = fitaSaidaMin + (i % 10);
            
            // Chama a função que realmente faz o merge dos 10 caminhos
            intercalarBlocos(fitaEntradaMin, fitaEntradaMax, fitaDestinoIdx);
            blocosGeradosNestaPassada++;
        }

        // --- O "Pulo do Gato": Inversão de Papéis ---
        // Quem era saída vira entrada para a próxima rodada
        int tempMin = fitaEntradaMin;
        int tempMax = fitaEntradaMax;
        fitaEntradaMin = fitaSaidaMin;
        fitaEntradaMax = fitaSaidaMax;
        fitaSaidaMin = tempMin;
        fitaSaidaMax = tempMax;

        // Atualizamos o número de blocos para a próxima passada
        numBlocos = blocosGeradosNestaPassada;

        // Importante: Limpar as fitas de saída da rodada anterior para não misturar dados
        for (int j = fitaSaidaMin; j <= fitaSaidaMax; j++) {
            char nome[20];
            sprintf(nome, "fita%d.bin", j);
            FILE* f = fopen(nome, "wb"); // Abre em modo "w" para zerar o arquivo
            if(f) fclose(f);
        }
    }
    
    printf("\n--- Ordenação Finalizada ---\n");
    printf("O arquivo final ordenado está na fita%d.bin\n", fitaEntradaMin);
}

void ordernarArquivo2F(int quantidade, FILE* arq) {
    int numBlocos = 0;

    printf("Iniciando Fase 1: Geracao de Blocos (Selecao por Substituicao)...\n");
    faseGeracaoBlocos(arq, &numBlocos);
    printf("Sucesso! %d blocos gerados.\n", numBlocos);

    printf("Iniciando Fase 2: Intercalacao Balanceada (2f fitas)...\n");
    faseIntercalacao2F(numBlocos);

    printf("\nResultados do Metodo 1:\n");
    printf("Transferencias de Leitura: %ld\n", transferenciasLeitura);
    printf("Transferencias de Escrita: %ld\n", transferenciasEscrita);
    printf("Comparacoes de Chaves: %ld\n", comparacoesChaves);
}