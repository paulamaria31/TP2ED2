#include "ordenaF1.h"
#include <string.h>
#include <time.h>

long long f1_transferenciasLeitura = 0;
long long f1_transferenciasEscrita = 0;
long long f1_comparacoesChaves = 0;

// Funcao que gerencia tudo
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

    // Numero de blocos
    int numBlocos;
    do
    {
        // Chamo a intercalação para intercalar os blocos
        intercalacao();

        // Conto os blocos da fita de saida
        numBlocos = contarBlocos("Saida.bin");

        // Se o numero de blocos for maior que um, precisa redistribuir
        if (numBlocos > 1)
        {
            // Se ainda não acabou, espalha para as fitas e repete
            redistribuir();
        }

        // Enquanto tiver blocos continuo o processo
    } while (numBlocos > 1);

    // Final da funcao
    clock_t t_fim = clock();
    // Converto o tempo
    double tempoTotal = (double)(t_fim - t_inicio) / CLOCKS_PER_SEC;

    // Mostro as metricas
    printf("\n--- Resultados do Metodo 2 (F+1 fitas) ---\n");
    printf("Transferencias de Leitura: %lld\n", f1_transferenciasLeitura);
    printf("Transferencias de Escrita: %lld\n", f1_transferenciasEscrita);
    printf("Comparacoes de Chaves: %lld\n", f1_comparacoesChaves);
    printf("Tempo de Execucao: %.6f s\n", tempoTotal);

    // Se quiser imprimir
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
    // Crio a fita atual
    sprintf(nomeArquivo, "Fita%d.bin", numFitas);
    FILE *fitaAtual = fopen(nomeArquivo, "wb");

    Registro alunos[TAM_VET];
    Registro aluno;
    Registro ultimoSair;
    int contMarcados = 0;

    // Começo lendo
    Aluno tempAluno;
    int lidosInicial = 0;
    // Leio os primeiros 19
    for (int i = 0; i < TAM_VET && lidosInicial < quantidade; i++)
    {
        // Leio os primeiros 19 alunos
        if (fread(&tempAluno, sizeof(Aluno), 1, arq) == 1)
        {
            // Salvo no meu vetor de aluno
            alunos[i].aluno = tempAluno;
            alunos[i].marcado = 0;
            // Atulizado o contador
            lidosInicial++;
            // Atualizo transferencia
            f1_transferenciasLeitura++;
        }
    }

    // Faço o heap com os 19 primeiros alunos
    heap(alunos, lidosInicial);

    // Os lidos ate o momento (19)
    int lidosTotal = lidosInicial;
    // Enquanto o lidos total for menor que a quantidade pedida ou lidos inicial for maior que zero vai lendo
    while (lidosTotal < quantidade || lidosInicial > 0)
    {
        // Se for igual a zero encerro
        if (lidosInicial == 0)
            break;

        // Pego o menor do heap
        ultimoSair = alunos[0];
        // Escrevo na fita atual
        fwrite(&ultimoSair, sizeof(Registro), 1, fitaAtual);
        // Incremento escrita
        f1_transferenciasEscrita++;

        // Se o lidos total ainda for menor que a quantidade e tiver oque ler, vou ver
        if (lidosTotal < quantidade && fread(&tempAluno, sizeof(Aluno), 1, arq) == 1)
        {
            // Incremento leitura e lidos total
            f1_transferenciasLeitura++;
            lidosTotal++;

            // Comparo o aluno novo com o ultimo que saiu, se for menor, eu marco ele
            aluno.aluno = tempAluno;
            if (aluno.aluno.nota < ultimoSair.aluno.nota)
            {
                // Marco o aluno
                aluno.marcado = 1;
                // Atualizo o contador
                contMarcados++;
            }
            else
            {
                // Senao for menor, nao fica marcado
                aluno.marcado = 0;
            }
            // Coloco o aluno novo na posicao do que saiu
            alunos[0] = aluno;
        }
        else
        {
            // Se nao tiver oque ler, tiro o que estava na ultima posicao e coloco na primeira
            alunos[0] = alunos[lidosInicial - 1];
            // Diminuo para funcionar o heap
            lidosInicial--;
        }

        // Se todos estiverem marcados, cria novo bloco/fita
        if (lidosInicial > 0 && contMarcados == lidosInicial)
        {
            // Fecho a fita atual
            fclose(fitaAtual);
            // Incremento o numero de fitas
            numFitas++;
            // Mudo o indice da fita
            int indiceFita = ((numFitas - 1) % 19) + 1;
            // Crio uma fita nova
            sprintf(nomeArquivo, "Fita%d.bin", indiceFita);
            // Decide se vai criar fita nova ou inserir
            fitaAtual = (numFitas <= 19) ? fopen(nomeArquivo, "wb") : fopen(nomeArquivo, "ab");

            // Marco os alunos novos como 0
            for (int i = 0; i < lidosInicial; i++)
                alunos[i].marcado = 0;
            // Zero o contador de marcados
            contMarcados = 0;
            // Construo um novo heap
            heap(alunos, lidosInicial);
        }
        else if (lidosInicial > 0)
        {
            // Nenhum bloco foi criado mas o topo foi alterado
            refazerHeap(alunos, 0, lidosInicial);
        }
    }

    // Se tiver fita aberta eu fecho
    if (fitaAtual)
        fclose(fitaAtual);
}

// Intercalar os blocos
void intercalacao()
{
    // Crio as 19 fitas como nula
    FILE *fitasEntrada[19] = {NULL};
    // Crio a fita de saida
    FILE *fitaSaida = fopen("Saida.bin", "wb");
    char nomeFita[20];

    // Abre as 19 fitas de entrada
    for (int i = 0; i < 19; i++)
    {
        sprintf(nomeFita, "Fita%d.bin", i + 1);
        fitasEntrada[i] = fopen(nomeFita, "rb");
    }

    // Inicializo como se ainda tivesse dados
    bool aindaTemDados = true;
    // Enquanto tiver dados vai acontecer
    while (aindaTemDados)
    {
        // Recebe falso pois ainda nao li nada
        aindaTemDados = false;
        // Crio um vetor para heap
        Registro heapIntercalacao[19];

        // Inicializa o heap com o primeiro registro disponível de cada fita
        for (int i = 0; i < 19; i++)
        {
            // Limpo a memoria para nao vir lixo
            memset(&heapIntercalacao[i], 0, sizeof(Registro));
            // Se a fita for diferente de nulo e ter o que ler da fita
            if (fitasEntrada[i] != NULL && fread(&heapIntercalacao[i], sizeof(Registro), 1, fitasEntrada[i]))
            {
                // Salvo a fita de origem e coloco marcado igual a 0
                heapIntercalacao[i].idFitaOrigem = i;
                heapIntercalacao[i].marcado = 0;
                // E como ainda tem oque ler, marco o marcado como true
                aindaTemDados = true;
                // Incremento leitura
                f1_transferenciasLeitura++;
            }
            else
            {
                // Minha fita atual esta vazia
                heapIntercalacao[i].marcado = 2;
            }
        }

        // Se nao tiver oque ler, dou um break
        if (!aindaTemDados)
            break;

        // Chamo o heap para esse meu primeiro vetor
        heap(heapIntercalacao, 19);

        // Enquanto o heap tiver marcado com 0 ou 1, ou seja ainda tem informação
        while (heapIntercalacao[0].marcado < 2)
        {
            // Grava o menor na saída
            fwrite(&heapIntercalacao[0], sizeof(Registro), 1, fitaSaida);
            // Incremento contador de escrita
            f1_transferenciasEscrita++;

            // Pego a fita que saiu o aluno
            int f = heapIntercalacao[0].idFitaOrigem;

            // Tenta ler o próximo da mesma fita para repor o heap
            if (fread(&heapIntercalacao[0], sizeof(Registro), 1, fitasEntrada[f]))
            {
                // Se tiver o que ler pego os dados
                heapIntercalacao[0].idFitaOrigem = f;
                heapIntercalacao[0].marcado = 0;
                // Incremento a leitura
                f1_transferenciasLeitura++;
            }
            else
            {
                // Acabou a fita que saiu o aluno, ou seja ele era o ultimo
                heapIntercalacao[0].marcado = 2;
            }
            // refaço o heap
            refazerHeap(heapIntercalacao, 0, 19);
        }
    }

    // Fecha todas as fitas
    fclose(fitaSaida);
    for (int i = 0; i < 19; i++)
    {
        if (fitasEntrada[i] != NULL)
            fclose(fitasEntrada[i]);
    }
}

void redistribuir()
{
    // Pego a fita de saida que esta com os blocos
    FILE *fitaSaida = fopen("Saida.bin", "rb");
    if (!fitaSaida)
        return;

    // Abro as 19 fitas
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

    // Enquanto tiver o que ler
    while (fread(&atual, sizeof(Registro), 1, fitaSaida))
    {
        // Incremento leitura
        f1_transferenciasLeitura++;

        // Se a nota atual for menor que a anterior, um novo bloco começou
        if (!primeiro && atual.aluno.nota < anterior.aluno.nota)
        {
            // Incremento comparações
            f1_comparacoesChaves++;
            // Vou pra proxima fita
            fitaAlvo = (fitaAlvo + 1) % 19;
        }

        // Escrevo o registro na fita atual
        fwrite(&atual, sizeof(Registro), 1, fitasEntrada[fitaAlvo]);
        // Incremento escrita
        f1_transferenciasEscrita++;

        // anterior recebe atual para proxima comparação
        anterior = atual;
        // Nao vai ter mais primeiro
        primeiro = false;
    }

    // Fecho as fitas
    fclose(fitaSaida);
    for (int i = 0; i < 19; i++)
        fclose(fitasEntrada[i]);
}

// Recebo o nome do arquivo
int contarBlocos(const char *nomeArquivo)
{
    // Abro o arquivo atual
    FILE *arq = fopen(nomeArquivo, "rb");
    if (!arq)
        return 0;

    int blocos = 0;
    Registro atual, anterior;

    // Leio um aluno
    if (fread(&anterior, sizeof(Registro), 1, arq))
    {
        // Se tiver oque lê, ja tem um bloco
        blocos = 1;
        // Leio o proximo
        while (fread(&atual, sizeof(Registro), 1, arq))
        {
            // Se o atual for menor que o anterior, comecei um bloco novo
            if (atual.aluno.nota < anterior.aluno.nota)
                // incremento contador de blocos
                blocos++;
            // anterior recebe atual para proxima comparação
            anterior = atual;
        }
    }
    // fecho o arquivo
    fclose(arq);
    // retorno o numero de blocos
    return blocos;
}

void heap(Registro alunos[], int n)
{
    // Começa do ultimo no pai e vai ate a raiz
    for (int c = (n / 2) - 1; c >= 0; c--)
    {
        // Chama refazer heap para consertar cada nivel
        refazerHeap(alunos, c, n);
    }
}

void refazerHeap(Registro alunos[], int i, int n)
{
    // no atual
    int menor = i;
    // Filho a esquerda
    int esq = 2 * i + 1;
    // Filho a direita
    int dir = 2 * i + 2;

    // Se for uma posicao acessivel
    if (esq < n)
    {

        // Incremento de comparação
        f1_comparacoesChaves++;
        // Verifica se o pai é maior que o filho
        if (ehMaior(alunos[menor], alunos[esq]))
        {
            // Atualizo o indice pois o filho é um candido melhor para subir ao topo
            menor = esq;
        }
    }

    // Se for uma posicao acessivel
    if (dir < n)
    {
        // Verifica se o pai é maior que o filho
        f1_comparacoesChaves++;
        if (ehMaior(alunos[menor], alunos[dir]))
        {
            // Atualizo o indice pois o filho é um candido melhor para subir ao topo
            menor = dir;
        }
    }

    //Se o menor mudou, a regra foi violada
    if (menor != i)
    {
        //Troca o pai pelo menor dos filhos
        Registro temp = alunos[i];
        alunos[i] = alunos[menor];
        alunos[menor] = temp;
        //Refaço o heap
        refazerHeap(alunos, menor, n);
    }
}

bool ehMaior(Registro a, Registro b)
{
    // Se algum for marcado, o marcado prevalece
    if (a.marcado != b.marcado)
    {
        return a.marcado > b.marcado;
    }
    // Retorno quando nota é maior
    return a.aluno.nota > b.aluno.nota;
}

void imprime_resultado_final(const char *arquivo, int qtd)
{
    // Crio variavel de arquivo
    FILE *f = fopen(arquivo, "rb");
    // Verifico se é nula
    if (!f)
        return;
    Registro r;
    // imprimo os registros
    printf("\n--- Primeiros registros ordenados ---\n");
    for (int i = 0; i < qtd; i++)
    {
        if (fread(&r, sizeof(Registro), 1, f))
            printf("Nota: %.2f | Inscricao: %ld\n", r.aluno.nota, r.aluno.nInscricao);
    }
    fclose(f);
}