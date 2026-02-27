#include "Aluno.h"
#include "ordenaF1.h"
#include "ordena2F.h"
#include "ordenaQuick.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Tipos de pesquisa
void ordena2F(int quantidade, int situacao, bool imprimir);
void limparFitas();
void ordenaF(int quantidade, int situacao, bool imprimir);
void ordenaQuick(int quantidade, int situacao, bool imprimir);
void imprime_primeiros(const char *nomeArquivo, int qtd);
int copia_n_registros(const char *src, const char *dst, int n);

int main(int argc, char *argv[])
{
    int metodo, quantidade, situacao;
    bool imprimir;

    // Entrada da pesquisa
    if (argc < 4)
    {
        printf("Formato: ordeno <metodo> <quantidade> <situacao> [-P]\n");
        return 0;
    }

    // Distribuição das variaveis
    metodo = atoi(argv[1]);
    quantidade = atoi(argv[2]);
    situacao = atoi(argv[3]);

    // Se quer imprimir ou não
    if (argc >= 5 && strcmp(argv[4], "-P") == 0)
    {
        imprimir = true;
    }
    else
    {
        imprimir = false;
    }

    limparFitas();

    // Vê qual metodo vai ser ultilizado
    switch (metodo)
    {
    case 1:
        ordena2F(quantidade, situacao, imprimir);
        break;
    case 2:
        ordenaF(quantidade, situacao, imprimir);
        break;
    case 3:
        ordenaQuick(quantidade, situacao, imprimir);
        break;
    default:
        printf("Nao existe esse metodo de pesquisa\n");
        return 0;
    }

    return 0;
}

void ordena2F(int quantidade, int situacao, bool imprimir)
{
    FILE *arq;

    switch (situacao)
    {
    case 1:
        // ascendente
        arq = fopen("Ascendente.bin", "rb");
        break;
    case 2:
        // descendente
        arq = fopen("Descendente.bin", "rb");
        break;
    case 3:
        // aleatorio
        arq = fopen("PROVAO.bin", "rb");
        break;
    default:
        printf("Situacao invalida.\n");
        return;
    }

    if (arq == NULL)
    {
        printf("Erro ao abrir o arquivo binario. Verifique se os arquivos de teste foram gerados.\n");
        return;
    }

    ordenarArquivo2F(quantidade, arq, imprimir);
    if (imprimir) imprime_primeiros("Saida.bin", quantidade);

    fclose(arq);
}
void ordenaF(int quantidade, int situacao, bool imprimir)
{
    FILE *arq;

    switch (situacao)
    {
    case 1:
        // ascendente
        arq = fopen("Ascendente.bin", "rb");
        break;
    case 2:
        // descendente
        arq = fopen("Descendente.bin", "rb");
        break;
    case 3:
        // aleatorio
        arq = fopen("PROVAO.bin", "rb");
        break;
    default:
        break;
    }

    if (arq != NULL) {
        ordenarArquivo(quantidade, arq, imprimir);
        fclose(arq);
    } else {
        printf("Erro ao abrir arquivo para o Metodo 2.\n");
    }
}

void ordenaQuick(int quantidade, int situacao, bool imprimir)
{
    const char *entrada;
    switch (situacao)
    {
    case 1:
        entrada = "Ascendente.bin";
        break;
    case 2:
        entrada = "Descendente.bin";
        break;
    case 3:
        entrada = "PROVAO.bin";
        break;
    default:
        fprintf(stderr, "Situação inválida. Use 1, 2 ou 3.\n");
        return;
    }

    const char *work = "WORK.bin";

    // NÃO altera o arquivo de entrada: copia apenas os n primeiros registros para um arquivo de trabalho
    if (!copia_n_registros(entrada, work, quantidade))
        return;

    if (imprimir)
    {
        printf("Antes da ordenação:\n");
        imprime_primeiros(work, quantidade);
    }

    FILE *ArqLi = fopen(work, "r+b");
    FILE *ArqEi = fopen(work, "r+b");
    FILE *ArqLEs = fopen(work, "r+b");

    if (!ArqLi || !ArqEi || !ArqLEs)
    {
        fprintf(stderr, "Erro ao abrir arquivo de trabalho.\n");
        if (ArqLi)
            fclose(ArqLi);
        if (ArqEi)
            fclose(ArqEi);
        if (ArqLEs)
            fclose(ArqLEs);
        return;
    }

    QuickResetMetrics();

    clock_t t0 = clock();
    QuicksortExterno(&ArqLi, &ArqEi, &ArqLEs, 1, quantidade);
    clock_t t1 = clock();

    fclose(ArqLi);
    fclose(ArqEi);
    fclose(ArqLEs);

    QuickMetrics m = QuickGetMetrics();
    double tempo = (double)(t1 - t0) / (double)CLOCKS_PER_SEC;

    // Saída obrigatória do TP (sempre)
    printf("leituras: %lld\n", m.leituras);
    printf("escritas: %lld\n", m.escritas);
    printf("comparacoes: %lld\n", m.comparacoes);
    printf("tempo: %.6f s\n", tempo);

    if (imprimir)
    {
        printf("Depois da ordenação:\n");
        imprime_primeiros(work, quantidade);
    }
}

void imprime_primeiros(const char *nomeArquivo, int qtd)
{
    FILE *f = fopen(nomeArquivo, "rb");
    if (!f)
        return;
    Aluno a;
    for (int i = 0; i < qtd; i++)
    {
        if (fread(&a, sizeof(Aluno), 1, f) != 1)
            break;
        printf("Inscrição: %ld | Nota: %.2f\n", a.nInscricao, a.nota);
    }
    fclose(f);
}

int copia_n_registros(const char *src, const char *dst, int n)
{
    FILE *in = fopen(src, "rb");
    if (!in)
    {
        fprintf(stderr, "Erro: não consegui abrir %s\n", src);
        return 0;
    }
    FILE *out = fopen(dst, "wb");
    if (!out)
    {
        fprintf(stderr, "Erro: não consegui criar %s\n", dst);
        fclose(in);
        return 0;
    }

    Aluno a;
    for (int i = 0; i < n; i++)
    {
        if (fread(&a, sizeof(Aluno), 1, in) != 1)
            break;
        fwrite(&a, sizeof(Aluno), 1, out);
    }

    fclose(in);
    fclose(out);
    return 1;
}

void limparFitas() {
    system("rm -f fita*.bin Fita*.bin Saida.bin WORK.bin");
    printf("Arquivos temporários limpos com sucesso.\n");
}