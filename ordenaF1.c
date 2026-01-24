#include "ordenaF1.h"

// Meu quantidade igual a 100,200,etc
void lerRegistro(int quantidade, FILE *arq)
{   
    //Numero de fitas
    int numFitas = 1;
    char nomeArquivo[20];
    sprintf(nomeArquivo, "Fita%d.bin", numFitas);
        //Criacao do arquivo bin

    FILE *fitaAtual = fopen(nomeArquivo, "wb");

    //Meu vetor de 19 alunos
    Registro alunos[TAM_VET];
    Registro aluno;
    Registro ultimoSair;

    // Leio os 19 primeiros alunos
    fread(alunos, sizeof(Registro), TAM_VET, arq);
    // Construo o heap
    heap(alunos, TAM_VET);

    int c = TAM_VET;
    // Contador de registros da fita
    int cont = 0;

    //Começo o processo
    while (c < quantidade)
    {
        // Escrevo o aluno do heap na primeira fita de entrada
        ultimoSair = alunos[0];
        fwrite(&ultimoSair, sizeof(Registro), 1, fitaAtual);

        // Leio outro aluno para entrar
        if (fread(&aluno, sizeof(Registro), 1, arq) == 1)
        {
            // Contador da quantidade total
            c++;
            // Vendo se o que entrou agora é menor que o que saiu
            if (aluno.aluno.nota < ultimoSair.aluno.nota)
            {
                // Marco ele pois é menor, logo vai ter prioridade
                aluno.marcado = 1;
                // Atualizo contador da fita
                cont++;
            }
            else
            {
                aluno.marcado = 0;
            }
            // Coloco o aluno novo no lugar do aluno que saiu
            alunos[0] = aluno;
        }
        else
        {
            break;
        }

        //Refaço o heap com o novo aluno
        refazerHeap(alunos, 0, TAM_VET);

        //rEFAZER PARA VER QUANTOS BLOCOS VAO FICAR EM CADA FITA
        if (cont == TAM_VET)
        {
            fclose(fitaAtual);
            numFitas++;
            sprintf(nomeArquivo, "Fita%d.bin", numFitas);
            fitaAtual = fopen(nomeArquivo, "wb");

            for (int i = 0; i < TAM_VET; i++)
            {
                alunos[i].marcado = 0;
            }
            heap(alunos, TAM_VET);
            cont = 0;
        }
    }

    //Inicializo o marcado diferentes para os novos registros
    for (int i = 0; i < TAM_VET; i++)
    {
        fwrite(&alunos[0], sizeof(Registro), 1, fitaAtual);
        alunos[0].marcado = 2;
        refazerHeap(alunos, 0, TAM_VET);
    }

    fclose(fitaAtual);
}

void heap(Registro alunos[], int n)
{
    for (int c = (n / 2) - 1; c >= 0; c--)
    {
        refazerHeap(alunos, c, n);
    }
}

void refazerHeap(Registro alunos[], int i, int n)
{
    int menor = i;
    // Filho da esquerda
    int esq = 2 * i + 1;
    // Filho da direita
    int dir = 2 * i + 2;

    // Compara com o filho da esquerda
    if (esq < n && ehMaior(alunos[menor], alunos[esq]))
    {
        menor = esq;
    }

    // Compara com filho da direita
    if (dir < n && ehMaior(alunos[menor], alunos[dir]))
    {
        menor = dir;
    }

    if (menor != i)
    {
        Registro temp = alunos[i];
        alunos[i] = alunos[menor];
        alunos[menor] = temp;
        refazerHeap(alunos, menor, n);
    }
}

bool ehMaior(Registro a, Registro b)
{
    // Verificando se algum esta marcado
    if (a.marcado != b.marcado)
    {
        // Se tiver marcado (1) vai ser maior que o nao marcado, logo ele é maior
        return a.marcado > b.marcado;
    }
    // Se nenhum dos dois estiverem marcado, retorna de acordo com a nota
    return a.aluno.nota > b.aluno.nota;
}