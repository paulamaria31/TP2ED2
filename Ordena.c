#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define JOAO
#include "Aluno.h"

int compararAsc(const void *a, const void *b) {
    Aluno *alunoA = (Aluno *)a;
    Aluno *alunoB = (Aluno *)b;
    if (alunoA->nota < alunoB->nota) return -1;
    if (alunoA->nota > alunoB->nota) return 1;
    return 0;
}


int compararDesc(const void *a, const void *b) {
    return compararAsc(b, a); // Inverte a lógica da ascendente
}

void salvarTxt(const char *nomeArquivo, Aluno *alunos, int n) {
    FILE *f = fopen(nomeArquivo, "w");
    for (int i = 0; i < n; i++) {
        fprintf(f, "%08ld %04.1f %s %-50s %-30s\n", 
                alunos[i].nInscricao, alunos[i].nota, 
                alunos[i].estado, alunos[i].cidade, alunos[i].curso);
    }
    fclose(f);
}

void salvarBin(const char *nomeArquivo, Aluno *alunos, int n) {
    FILE *f = fopen(nomeArquivo, "wb");
    fwrite(alunos, sizeof(Aluno), n, f);
    fclose(f);
}

int main() {
    int maxAlunos = 471705;
    Aluno *alunos = (Aluno *)malloc(maxAlunos * sizeof(Aluno));
    
    FILE *arquivo = fopen("PROVAO.TXT", "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir PROVAO.TXT\n");
        return 1;
    }

    int i = 0;
    char linha[105];
    while (fgets(linha, sizeof(linha), arquivo) && i < maxAlunos) {
        // Leitura baseada nas colunas fixas do arquivo TXT
        sscanf(linha, "%8ld %5lf %2s", &alunos[i].nInscricao, &alunos[i].nota, alunos[i].estado);
        
        // Captura cidade (colunas 19 a 68) e curso (70 a 99)
        strncpy(alunos[i].cidade, &linha[18], 50);
        alunos[i].cidade[50] = '\0';
        strncpy(alunos[i].curso, &linha[69], 30);
        alunos[i].curso[30] = '\0';
        
        i++;
    }
    fclose(arquivo);

    salvarBin("PROVAO.bin", alunos, i);

    qsort(alunos, i, sizeof(Aluno), compararAsc);
    salvarTxt("Ascendente.txt", alunos, i);
    salvarBin("Ascendente.bin", alunos, i);

    qsort(alunos, i, sizeof(Aluno), compararDesc);
    salvarTxt("Descendente.txt", alunos, i);
    salvarBin("Descendente.bin", alunos, i);

    printf("Processamento concluído com sucesso!\n");
    free(alunos);
    return 0;
}