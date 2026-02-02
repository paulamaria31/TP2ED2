#include "ordenaF1.h"

void ordernarArquivo(int quantidade, FILE* arq) {

    //Chamo a funcao que vai ler os registros
    lerRegistro(quantidade, arq);

    int numBlocos;
    //Primeira passa para intercalar
    int passada = 1;

    do {
        printf("Iniciando Passada %d...\n", passada++);
        
        //Chamo a intercalação
        intercalacao();
        
        //Conto os blocos da fita de saida
        numBlocos = contarBlocos("Saida.bin");
        printf("Blocos restantes na Saida.bin: %d\n", numBlocos);

        //Se o numero de blocos for maior que um, precisa redistribuir
        if (numBlocos > 1) {
            redistribuir(); // Se ainda não acabou, espalha para as fitas e repete
        }

        //Enquanto tiver blocos continuo o processo
    } while (numBlocos > 1);

    printf("Ordenação concluída! O arquivo Saida.bin está 100%% ordenado.\n");
}

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

    intercalação();
}

void intercalacao()
{
    Registro matrizBuffers[19][19];
    int quantidadeLidaFita[19];
    FILE *fitasEntrada[19];
    int indiceAtualFita[19];
    FILE *fitaSaida = fopen("Saida.bin", "wb");
    char nomeFita[20];
    int contFitaSaida = 0;

    // Abro as 19 fitas de entrada no início (uma única vez)
    for (int i = 0; i < 19; i++)
    {
        sprintf(nomeFita, "Fita%d.bin", i + 1);
        fitasEntrada[i] = fopen(nomeFita, "rb");
    }

    // Loop para processar os blocos
    bool aindaTemDados = true;
    while (aindaTemDados)
    {
        // Ainda tem dados inicia falso mas fica verdadeiro depois
        aindaTemDados = false;

        // Vou ler bloco por bloco por fita
        for (int i = 0; i < 19; i++)
        {
            // Verifico se a fita tem informação ou senão é o final
            if (fitasEntrada[i] != NULL && !feof(fitasEntrada[i]))
            {
                // Leio os 19 registros da fita e armazeno
                int lidos = fread(matrizBuffers[i], sizeof(Registro), 19, fitasEntrada[i]);

                if (lidos > 0)
                {
                    // Se o lidos for maior que 0, ainda tem informação para ser lido
                    aindaTemDados = true;
                    // Guardamos quantos registros realmente lemos dessa fita (pode ser < 19)
                    quantidadeLidaFita[i] = lidos;
                    // Ainda nao usamos nenhum desses novos dados
                    indiceAtualFita[i] = 0;
                }
                else
                {
                    // Se o lidos for zero quer dizer que eu nao li nada
                    quantidadeLidaFita[i] = 0;
                }
            }
        }

        // Se não tiver dados, posso dar um break
        if (!aindaTemDados)
            break;

        // Criamos um vetor para o heap
        Registro heapIntercalacao[19];
        for (int i = 0; i < 19; i++)
        {
            // Se tiver registro na fita
            if (quantidadeLidaFita[i] > 0)
            {
                // Pego o primeiro registro de cada fita
                heapIntercalacao[i] = matrizBuffers[i][0];
                // Coloco de qual fita esse registro veio
                heapIntercalacao[i].idFitaOrigem = i; // Importante para saber de onde veio
            }
            else
            {
                // A posicao especifica da fita esta vazia
                heapIntercalacao[i].marcado = 2;
            }
        }
        // Faço um heap com os elementos
        heap(heapIntercalacao, 19);

        // Se a fita nao estiver vazia, entao tem dados para processar
        while (heapIntercalacao[0].marcado < 2)
        {
            // Pego o menor valor que esta no heap e coloco na fita de saida
            fwrite(&heapIntercalacao[0], sizeof(Registro), 1, fitaSaida);
            contFitaSaida++;

            // Salvo a fita de origem dele
            int f = heapIntercalacao[0].idFitaOrigem;

            // Incremento o indice da fita de onde saiu para pegar mais um registro
            indiceAtualFita[f]++;

            // Se ainda tiver registros nessa fita
            if (indiceAtualFita[f] < quantidadeLidaFita[f])
            {
                // Pego o registro para fazer o heap e coloco no lugar de quem saiu
                heapIntercalacao[0] = matrizBuffers[f][indiceAtualFita[f]];
                // Coloco a fita de origem dele
                heapIntercalacao[0].idFitaOrigem = f;
            }
            else
            {
                // Se nao tiver mais fita marcamos como dois pois não tem como mais pegar
                heapIntercalacao[0].marcado = 2;
            }

            // Refaço o heap para o novo que entrou
            refazerHeap(heapIntercalacao, 0, 19);
        }
    }

}

void redistribuir()
{
    char nomeFita[20];
    FILE *fitasEntrada[19];
    FILE *fitaSaida = fopen("Saida.bin", "rb");


    // Abro as fitas de entrada
    for (int i = 0; i < 19; i++)
    {
        sprintf(nomeFita, "Fita%d.bin", i + 1);
        fitasEntrada[i] = fopen(nomeFita, "wb");
    }

    //Registros para poder comparar
    Registro alunoAtual, alunoAnterior;
    //Fita atual
    int fitaAlvo = 0;
    //Não dar erro no primeiro
    bool primeiro = true;

    //Leio um registro da fita de saida
    while (fread(&alunoAtual, sizeof(Registro), 1, fitaSaida)) {
        //Se a nota atual for menor que a anterior, quer dizer que começou um bloco novo
        if (!primeiro && alunoAtual.aluno.nota < alunoAnterior.aluno.nota) {
            //Inicio uma nova fita
            fitaAlvo = (fitaAlvo + 1) % 19;
        }
        //Se não for, ainda estamos no mesmo bloco na saida e escrevo na fita atual
        fwrite(&alunoAtual, sizeof(Registro), 1, fitasEntrada[fitaAlvo]);
        //Agora o anterior recebe o atual para comparar
        //E como não é o primeiro, não vai dar erro
        alunoAnterior = alunoAtual;
        primeiro = false;
    }

    //Fecho a fita de saida
    fclose(fitaSaida);
    //Fecho as fitas
    for (int i = 0; i < 19; i++) fclose(fitasEntrada[i]);
}

//Recebo o nome do arquivo
int contarBlocos(const char* nomeArquivo) {
    //Abro o arquivo
    FILE *arq = fopen(nomeArquivo, "rb");
    //Vejo se é vazio
    if (!arq) return 0;
    //Contdador de blocos
    int blocos = 0;
    Registro atual, anterior;
    //Leio o registro atual do arquivo para usar como base
    if (fread(&anterior, sizeof(Registro), 1, arq)) {
        //Ja incremento o contador pois pode ter um aluno so
        blocos = 1;
        //Leio o proximo para usar como base
        while (fread(&atual, sizeof(Registro), 1, arq)) {
            //Se o atual for menor que o anterior, logo iniciei um novo bloco
            if (atual.aluno.nota < anterior.aluno.nota) blocos++;
            //O anterior passa a ser o atual
            anterior = atual;
        }
    }
    //fecho o arquivo
    fclose(arq);
    //retorno o numero de blocos
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