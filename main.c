#include "Aluno.h"
#include "ordenaF1.h"
#include <stdbool.h>
#include <stdio.h>

// Tipos de pesquisa
void ordena2F(int quantidade, int situacao);
void ordenaF(int quantidade, int situacao);
void ordenaQuick(int quantidade, int situacao);

int main(int argc, char *argv[])
{
    int metodo, quantidade, situacao, chave;
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

    // Vê qual metodo vai ser ultilizado
    switch (metodo)
    {
    case 1:
        ordena2F(quantidade, situacao);
        break;
    case 2:
        ordenaF(quantidade, situacao);
        break;
    case 3:
        ordenaQuick(quantidade, situacao);
        break;
    default:
        printf("Nao existe esse metodo de pesquisa\n");
        return 0;
    }

    return 0;
}

void ordena2F(int quantidade, int situacao)
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
}
void ordenaF(int quantidade, int situacao)
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

    lerRegistro(quantidade, arq);
    fclose(arq);

}
void ordenaQuick(int quantidade, int situacao)
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
}