#include "ordenaF1.h"
#include <string.h>
#include <time.h>

long long f1_transferenciasLeitura = 0;
long long f1_transferenciasEscrita = 0;
long long f1_comparacoesChaves = 0;

void ordenarArquivo(int quantidade, FILE *arq, bool imprimir)
{
    clock_t t_inicio = clock();

    // Reset das métricas para cada execução
    f1_transferenciasLeitura = 0;
    f1_transferenciasEscrita = 0;
    f1_comparacoesChaves = 0;

    printf("Iniciando Fase 1: Geracao de blocos (Selecao por Substituicao)...\n");

    // Chamo a funcao que vai ler os registros
    lerRegistro(quantidade, arq);

    int numBlocos;
    do
    {
        // Chamo a intercalação
        intercalacao();

        // Conto os blocos da fita de saida
        numBlocos = contarBlocos("Saida.bin");

        // Se o numero de blocos for maior que um, precisa redistribuir
        if (numBlocos > 1)
        {
            redistribuir(); // Se ainda não acabou, espalha para as fitas e repete
        }

        // Enquanto tiver blocos continuo o processo
    } while (numBlocos > 1);

    clock_t t_fim = clock();
    double tempoTotal = (double)(t_fim - t_inicio) / CLOCKS_PER_SEC;

    // Saída obrigatória conforme exigência do trabalho
    printf("\n--- Resultados do Metodo 2 (F+1 fitas) ---\n");
    printf("Transferencias de Leitura: %lld\n", f1_transferenciasLeitura);
    printf("Transferencias de Escrita: %lld\n", f1_transferenciasEscrita);
    printf("Comparacoes de Chaves: %lld\n", f1_comparacoesChaves);
    printf("Tempo de Execucao: %.6f s\n", tempoTotal);

    if (imprimir)
    {
        imprime_resultado_final("Saida.bin", quantidade);
    }

    printf("Ordenação concluída! O arquivo Saida.bin está 100%% ordenado.\n");
}

// Meu quantidade igual a 100,200,etc
void lerRegistro(int quantidade, FILE *arq)
{
    int numFitas = 1;
    char nomeArquivo[20];
    sprintf(nomeArquivo, "Fita%d.bin", numFitas);
    FILE *fitaAtual = fopen(nomeArquivo, "wb");

    Registro alunos[TAM_VET];
    Registro aluno;
    Registro ultimoSair;
    int contMarcados = 0;

    // 1. Carga inicial do Heap (lendo Aluno, convertendo para Registro)
    Aluno tempAluno;
    int lidosInicial = 0;
    for (int i = 0; i < TAM_VET && lidosInicial < quantidade; i++)
    {
        // Lemos apenas o tamanho da struct Aluno (formato do PROVAO.bin)
        if (fread(&tempAluno, sizeof(Aluno), 1, arq) == 1)
        {
            alunos[i].aluno = tempAluno;
            alunos[i].marcado = 0;
            lidosInicial++;
            f1_transferenciasLeitura++;
        }
    }

    heap(alunos, lidosInicial);

    int lidosTotal = lidosInicial;
    while (lidosTotal < quantidade || lidosInicial > 0)
    {
        if (lidosInicial == 0) break;

        // Escrevo o menor atual na fita (Formato Registro para manter alinhamento)
        ultimoSair = alunos[0];
        fwrite(&ultimoSair, sizeof(Registro), 1, fitaAtual);
        f1_transferenciasEscrita++;

        // Tenta ler o próximo aluno do arquivo original
        if (lidosTotal < quantidade && fread(&tempAluno, sizeof(Aluno), 1, arq) == 1)
        {
            f1_transferenciasLeitura++;
            lidosTotal++;
            
            aluno.aluno = tempAluno;
            if (aluno.aluno.nota < ultimoSair.aluno.nota)
            {
                aluno.marcado = 1;
                contMarcados++;
            }
            else
            {
                aluno.marcado = 0;
            }
            alunos[0] = aluno;
        }
        else
        {
            // Se o arquivo original acabar, reduz o heap
            alunos[0] = alunos[lidosInicial - 1];
            lidosInicial--;
        }

        // Se todos estiverem marcados, cria novo bloco/fita
        if (lidosInicial > 0 && contMarcados == lidosInicial)
        {
            fclose(fitaAtual);
            numFitas++;
            int indiceFita = ((numFitas - 1) % 19) + 1;
            sprintf(nomeArquivo, "Fita%d.bin", indiceFita);
            fitaAtual = (numFitas <= 19) ? fopen(nomeArquivo, "wb") : fopen(nomeArquivo, "ab");

            for (int i = 0; i < lidosInicial; i++) alunos[i].marcado = 0;
            contMarcados = 0;
            heap(alunos, lidosInicial);
        }
        else if (lidosInicial > 0)
        {
            refazerHeap(alunos, 0, lidosInicial);
        }
    }

    if (fitaAtual) fclose(fitaAtual);
}

void intercalacao()
{
    FILE *fitasEntrada[19] = {NULL};
    FILE *fitaSaida = fopen("Saida.bin", "wb");
    char nomeFita[20];

    // Abre as 19 fitas de entrada
    for (int i = 0; i < 19; i++)
    {
        sprintf(nomeFita, "Fita%d.bin", i + 1);
        fitasEntrada[i] = fopen(nomeFita, "rb");
    }

    bool aindaTemDados = true;
    while (aindaTemDados)
    {
        aindaTemDados = false;
        Registro heapIntercalacao[19];

        // Inicializa o heap com o primeiro registro disponível de cada fita
        for (int i = 0; i < 19; i++)
        {
            memset(&heapIntercalacao[i], 0, sizeof(Registro));
            // IMPORTANTE: Ler sizeof(Registro) pois as fitas foram gravadas assim
            if (fitasEntrada[i] != NULL && fread(&heapIntercalacao[i], sizeof(Registro), 1, fitasEntrada[i]))
            {
                heapIntercalacao[i].idFitaOrigem = i;
                heapIntercalacao[i].marcado = 0;
                aindaTemDados = true;
                f1_transferenciasLeitura++;
            }
            else
            {
                heapIntercalacao[i].marcado = 2; // Fita vazia
            }
        }

        if (!aindaTemDados) break;

        heap(heapIntercalacao, 19);

        // Processa o heap enquanto houver registros válidos (marcado < 2)
        while (heapIntercalacao[0].marcado < 2)
        {
            // Grava o menor na saída
            fwrite(&heapIntercalacao[0], sizeof(Registro), 1, fitaSaida);
            f1_transferenciasEscrita++;

            int f = heapIntercalacao[0].idFitaOrigem;

            // Tenta ler o próximo da mesma fita para repor o heap
            if (fread(&heapIntercalacao[0], sizeof(Registro), 1, fitasEntrada[f]))
            {
                heapIntercalacao[0].idFitaOrigem = f;
                heapIntercalacao[0].marcado = 0;
                f1_transferenciasLeitura++;
            }
            else
            {
                heapIntercalacao[0].marcado = 2; // Fim desta fita
            }
            refazerHeap(heapIntercalacao, 0, 19);
        }
    }

    // Fecha tudo
    fclose(fitaSaida);
    for (int i = 0; i < 19; i++)
    {
        if (fitasEntrada[i] != NULL) fclose(fitasEntrada[i]);
    }
}

void redistribuir()
{
    FILE *fitaSaida = fopen("Saida.bin", "rb");
    if (!fitaSaida) return;

    FILE *fitasEntrada[19];
    char nomeFita[20];
    for (int i = 0; i < 19; i++)
    {
        sprintf(nomeFita, "Fita%d.bin", i + 1);
        fitasEntrada[i] = fopen(nomeFita, "wb");
    }

    Registro atual, anterior;
    int fitaAlvo = 0;
    bool primeiro = true;

    // Lemos Registro para manter o alinhamento de bytes
    while (fread(&atual, sizeof(Registro), 1, fitaSaida))
    {
        f1_transferenciasLeitura++;

        // Se a nota atual for menor que a anterior, um novo bloco começou
        if (!primeiro && atual.aluno.nota < anterior.aluno.nota)
        {
            f1_comparacoesChaves++;
            fitaAlvo = (fitaAlvo + 1) % 19; 
        }

        // Escrevemos Registro completo nas fitas
        fwrite(&atual, sizeof(Registro), 1, fitasEntrada[fitaAlvo]);
        f1_transferenciasEscrita++;

        anterior = atual;
        primeiro = false;
    }

    fclose(fitaSaida);
    for (int i = 0; i < 19; i++) fclose(fitasEntrada[i]);
}

// Recebo o nome do arquivo
int contarBlocos(const char *nomeArquivo)
{
    FILE *arq = fopen(nomeArquivo, "rb");
    if (!arq) return 0;
    
    int blocos = 0;
    Registro atual, anterior;
    
    // Leitura usando sizeof(Registro) para alinhar com a gravacao da Saida.bin
    if (fread(&anterior, sizeof(Registro), 1, arq))
    {
        blocos = 1;
        while (fread(&atual, sizeof(Registro), 1, arq))
        {
            if (atual.aluno.nota < anterior.aluno.nota)
                blocos++;
            anterior = atual;
        }
    }
    fclose(arq);
    return blocos;
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
    int esq = 2 * i + 1;
    int dir = 2 * i + 2;

    if (esq < n) {
        f1_comparacoesChaves++; // Conta aqui: você VAI fazer a comparação abaixo
        if (ehMaior(alunos[menor], alunos[esq])) {
            menor = esq;
        }
    }

    // Compara com filho da direita
    if (dir < n) {
        f1_comparacoesChaves++; // Conta aqui: você VAI fazer a comparação abaixo
        if (ehMaior(alunos[menor], alunos[dir])) {
            menor = dir;
        }
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
    if (a.marcado != b.marcado)
    {
        return a.marcado > b.marcado;
    }
    return a.aluno.nota > b.aluno.nota;
}

void imprime_resultado_final(const char *arquivo, int qtd)
{
    FILE *f = fopen(arquivo, "rb");
    if (!f) return;
    Registro r;
    printf("\n--- Primeiros registros ordenados ---\n");
    for (int i = 0; i < qtd; i++)
    {
        if (fread(&r, sizeof(Registro), 1, f))
            printf("Nota: %.2f | Inscricao: %ld\n", r.aluno.nota, r.aluno.nInscricao);
    }
    fclose(f);
}