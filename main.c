#include "Aluno.h"
#include <stdbool.h>

int main(int argc, char *argv[])
{
    int metodo, quantidade, situacao, chave;
    bool imprimir;

    //Entrada da pesquisa
    if (argc < 4)
    {
        printf("Formato: ordeno <metodo> <quantidade> <situacao> [-P]\n");
        return 0;
    }

    //Distribuição das variaveis
    metodo = atoi(argv[1]);
    quantidade = atoi(argv[2]);
    situacao = atoi(argv[3]);

    //Se quer imprimir ou não
    if (argc >= 5 && strcmp(argv[4], "-P") == 0)
    {
        imprimir = true;
    }
    else
    {
        imprimir = false;
    }

    //Vê qual metodo vai ser ultilizado
    switch (metodo)
    {
    case 1:
        ordena2F();
        break;
    case 2:
        ordenaF();
        break;
    case 3:
        ordenaQuick();
        break;
    default:
        printf("Nao existe esse metodo de pesquisa\n");
        return 0;
    }

    return 0;
}