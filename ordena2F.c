#include "ordena2F.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static long long m1_leituras = 0;
static long long m1_escritas = 0;
static long long m1_comparacoes = 0;

void refazerHeap2F(RegistroHeap heap[], int i, int n) {
    int menor = i;
    int esq = 2 * i + 1;
    int dir = 2 * i + 2;
    if (esq < n) {
        m1_comparacoes++;
        if (heap[esq].marcado < heap[menor].marcado || 
           (heap[esq].marcado == heap[menor].marcado && heap[esq].aluno.nota < heap[menor].aluno.nota))
            menor = esq;
    }
    if (dir < n) {
        m1_comparacoes++;
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

void ordenarArquivo2F(int quantidade, FILE* arq, bool imprimir) {
    m1_leituras = m1_escritas = m1_comparacoes = 0;
    clock_t t0 = clock();
    if (!arq) return;
    rewind(arq);

    // FASE 1: Geração de Blocos por Seleção por Substituição
    printf("Metodo 1 - Fase 1: Gerando blocos iniciais...\n"); fflush(stdout);
    int numBlocos = 0;
    RegistroHeap heap[MEM_TAM];
    FILE* fitas[F];
    char nome[32];
    for(int i=0; i<F; i++) { sprintf(nome, "fita%d.bin", i + 1); fitas[i] = fopen(nome, "wb"); }
    
    int heapSize = 0, processados = 0;
    for(int i=0; i<MEM_TAM && processados < quantidade; i++) {
        if(fread(&heap[i].aluno, sizeof(Aluno), 1, arq)) {
            heap[i].marcado = 0; heapSize++; processados++; m1_leituras++;
        }
    }
    for (int i = heapSize / 2 - 1; i >= 0; i--) refazerHeap2F(heap, i, heapSize);

    int fitaAtiva = 0, marcados = 0;
    while (heapSize > 0) {
        Aluno ultimo = heap[0].aluno;
        fwrite(&ultimo, sizeof(Aluno), 1, fitas[fitaAtiva]); m1_escritas++;
        if (processados < quantidade && fread(&heap[0].aluno, sizeof(Aluno), 1, arq)) {
            m1_leituras++; processados++;
            m1_comparacoes++;
            if (heap[0].aluno.nota < ultimo.nota) { heap[0].marcado = 1; marcados++; }
            else heap[0].marcado = 0;
        } else {
            heap[0] = heap[heapSize - 1]; heapSize--;
        }
        if (heapSize > 0 && marcados == heapSize) {
            for(int i=0; i<heapSize; i++) heap[i].marcado = 0;
            marcados = 0; numBlocos++; fitaAtiva = (fitaAtiva + 1) % F;
        }
        if(heapSize > 0) refazerHeap2F(heap, 0, heapSize);
    }
    if (processados > 0) numBlocos++;
    for(int i=0; i<F; i++) fclose(fitas[i]);
    printf("Blocos iniciais gerados: %d\n", numBlocos); fflush(stdout);

    // FASE 2: Intercalação Balanceada (10 vias)
    int fEntMin = 1, fSaiMin = 11;
    while (numBlocos > 1) {
        printf("Intercalando: %d blocos restantes...\n", numBlocos); fflush(stdout);
        int blocosSaida = 0, fitaSaidaAtiva = 0;
        
        struct { FILE* f; Aluno reg; int status; } curs[10];
        for(int i=0; i<10; i++) {
            sprintf(nome, "fita%d.bin", fEntMin + i); curs[i].f = fopen(nome, "rb");
            if (curs[i].f && fread(&curs[i].reg, sizeof(Aluno), 1, curs[i].f)) { curs[i].status = 1; m1_leituras++; }
            else { curs[i].status = -1; if(curs[i].f) fclose(curs[i].f); curs[i].f = NULL; }
        }
        FILE* sFit[10];
        for(int i=0; i<10; i++) { sprintf(nome, "fita%d.bin", fSaiMin + i); sFit[i] = fopen(nome, "wb"); }

        while (1) {
            int best = -1;
            for(int i=0; i<10; i++) {
                if (curs[i].status == 1) {
                    m1_comparacoes++;
                    if (best == -1 || curs[i].reg.nota < curs[best].reg.nota) best = i;
                }
            }
            if (best == -1) {
                // Fim de um bloco nas 10 fitas, rotaciona fita de saída
                blocosSaida++; fitaSaidaAtiva = (fitaSaidaAtiva + 1) % 10;
                int restamCursos = 0;
                for(int i=0; i<10; i++) if (curs[i].status == 0) { curs[i].status = 1; restamCursos = 1; }
                if (!restamCursos) break;
                continue;
            }
            fwrite(&curs[best].reg, sizeof(Aluno), 1, sFit[fitaSaidaAtiva]); m1_escritas++;
            Aluno ant = curs[best].reg;
            if (fread(&curs[best].reg, sizeof(Aluno), 1, curs[best].f)) {
                m1_leituras++; m1_comparacoes++;
                if (curs[best].reg.nota < ant.nota) curs[best].status = 0;
            } else { curs[best].status = -1; fclose(curs[best].f); curs[best].f = NULL; }
        }
        for(int i=0; i<10; i++) { if(curs[i].f) fclose(curs[i].f); if(sFit[i]) fclose(sFit[i]); }
        int t = fEntMin; fEntMin = fSaiMin; fSaiMin = t;
        numBlocos = blocosSaida;
    }
    
    // Copia o resultado final da fita vencedora para Saida.bin (para a main imprimir se -P)
    sprintf(nome, "fita%d.bin", fEntMin);
    FILE* finalF = fopen(nome, "rb");
    FILE* finalS = fopen("Saida.bin", "wb");
    if (finalF && finalS) {
        Aluno a;
        while(fread(&a, sizeof(Aluno), 1, finalF)) fwrite(&a, sizeof(Aluno), 1, finalS);
    }
    if(finalF) fclose(finalF); if(finalS) fclose(finalS);

    clock_t t1 = clock();
    printf("leituras: %lld\n", m1_leituras);
    printf("escritas: %lld\n", m1_escritas);
    printf("comparacoes: %lld\n", m1_comparacoes);
    printf("tempo: %.6f s\n", (double)(t1 - t0) / CLOCKS_PER_SEC);
}