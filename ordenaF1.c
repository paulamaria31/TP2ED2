#include "ordenaF1.h"

// Meu quantidade igual a 100,200,etc
void lerRegistro(int quantidade, FILE *arq)
{
    // Numero de fitas
    int registrosPorFitas[19] = {0};
    int contRegistrosFitas = 0;
    int numFitas = 1;
    char nomeArquivo[20];
    sprintf(nomeArquivo, "Fita%d.bin", numFitas);
    FILE *fitaAtual = fopen(nomeArquivo, "wb");

    Registro alunos[TAM_VET];
    Registro aluno;
    Registro ultimoSair;
    int contMarcados = 0; // Variável para controlar os marcados de forma eficiente

    // Leio os 19 primeiros alunos
    int lidosInicial = fread(alunos, sizeof(Registro), TAM_VET, arq);
    heap(alunos, lidosInicial);

    int c = lidosInicial;

    while (c < quantidade)
    {
        // Escrevo o aluno do heap na fita de entrada
        ultimoSair = alunos[0];
        fwrite(&ultimoSair, sizeof(Registro), 1, fitaAtual);
        registrosPorFitas[(numFitas - 1) % 19]++;

        // Leio outro aluno para entrar
        if (fread(&aluno, sizeof(Registro), 1, arq) == 1)
        {
            c++;
            // Vendo se o que entrou agora é menor que o que saiu
            if (aluno.aluno.nota < ultimoSair.aluno.nota)
            {
                aluno.marcado = 1;
                contMarcados++;
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
            // Se o arquivo original acabar, temos que tratar o fim do arquivo
            break;
        }

        // Refaço o heap com o novo aluno
        refazerHeap(alunos, 0, TAM_VET);

        // Se todos estiverem marcados, está na hora de criar uma nova fita (ou bloco)
        if (contMarcados == TAM_VET)
        {
            fclose(fitaAtual);
            numFitas++;

            // Lógica circular para 19 fitas
            int indiceFita = ((numFitas - 1) % 19) + 1;
            sprintf(nomeArquivo, "Fita%d.bin", indiceFita);

            // Abre nova fita ou anexa se já existir
            if (numFitas <= 19)
                fitaAtual = fopen(nomeArquivo, "wb");
            else
                fitaAtual = fopen(nomeArquivo, "ab");

            for (int i = 0; i < TAM_VET; i++)
            {
                alunos[i].marcado = 0;
            }
            heap(alunos, TAM_VET);
            contMarcados = 0;
        }
    }

    // Preciso descarregar os 19 que estao na memoria RAM
    for (int i = 0; i < TAM_VET; i++)
    {
        // Coloco na fita o aluno com heap
        ultimoSair = alunos[0];
        fwrite(&ultimoSair, sizeof(Registro), 1, fitaAtual);

        registrosPorFitas[(numFitas - 1) % 19]++;

        // 2. Para o próximo menor subir, "matamos" o atual
        // dando a ele uma marcação impossível (ex: 2)
        alunos[0].marcado = 2;

        // 3. Reorganizamos para o próximo menor real ir para o topo
        refazerHeap(alunos, 0, TAM_VET);
    }

    fclose(fitaAtual);

    intercalação(registrosPorFitas);
}

void intercalacao(int registrosPorFitas[])
{
    Registro matrizBuffers[19][19];
    FILE* fitasEntrada[19];
    FILE* fitaSaida = fopen("Saida.bin", "wb");
    char nomeFita[20];

    // 1. Abrir as 19 fitas de entrada no início (uma única vez)
    for (int i = 0; i < 19; i++) {
        sprintf(nomeFita, "Fita%d.bin", i + 1);
        fitasEntrada[i] = fopen(nomeFita, "rb");
    }

    // 2. Loop principal para processar TODOS os blocos
    // Precisamos de uma condição para saber se ainda há dados nas fitas
    bool aindaTemDados = true;
    while (aindaTemDados) {
        aindaTemDados = false;

        // PREENCHIMENTO DA MATRIZ (O que você pediu)
        for (int i = 0; i < 19; i++) {
            if (fitasEntrada[i] != NULL && !feof(fitasEntrada[i])) {
                // Lê os 19 registros da fita 'i' para a linha 'i' da matriz
                int lidos = fread(matrizBuffers[i], sizeof(Registro), 19, fitasEntrada[i]);
                if (lidos > 0) aindaTemDados = true;
            }
        }

        if (!aindaTemDados) break;

        // 3. FAZER O HEAP E JOGAR NA SAÍDA
        // Aqui você precisa decidir: você quer ordenar a matriz 19x19 inteira 
        // ou quer intercalar os topos? 
        // Se for intercalar, usamos o Heap de 19 elementos (um de cada fita).
        processarHeapIntercalacao(matrizBuffers, fitaSaida);

        // O loop 'while' vai voltar, ler os PRÓXIMOS 19 de cada fita
        // e preencher a matriz novamente, exatamente como você quer.
    }

    // 4. Fechar tudo ao final
    for (int i = 0; i < 19; i++) if (fitasEntrada[i]) fclose(fitasEntrada[i]);
    fclose(fitaSaida);
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