# Trabalho Prático 2 - Estrutra de Dados 

O objetivo deste trabalho consiste em um estudo mais profundo da complexidade de desempenho dos
seguintes métodos de ordenação externa apresentados em sala de aula:

    1. Intercalação balanceada de vários caminhos (2f fitas) utilizando, na etapa de geração dos blocos
    ordenados, a técnica de seleção por substituição.
    
    2. Intercalação balanceada de vários caminhos (f+1 fitas) utilizando, na etapa de geração dos blocos
    ordenados, a técnica de seleção por substituição.
    
    3. Quicksort externo.

A 1ª fase deste trabalho corresponde à implementação em C++ dos 3 métodos mencionados. Para o
método 1, deve ser considerada a existência de (a) memória interna disponível para armazenar um
vetor de, no máximo, 10 registros, e (b) 20 fitas de armazenamento externo, sendo 10 fitas de entrada e
10 fitas de saída a cada etapa de intercalação. Para o método 2, deve ser considerada a existência de
(a) memória interna disponível para armazenar um vetor de, no máximo, 19 registros, e (b) 20 fitas de
armazenamento externo, sendo 19 fitas de entrada e 1 fita de saída a cada etapa de intercalação. Para
o método 3, deve ser considerada a existência de memória interna disponível para armazenar um vetor
de, no máximo, 10 registros.

A 2ª fase deste trabalho corresponde à análise experimental da complexidade de desempenho dos 3
métodos mencionados, considerando os seguintes quesitos:
- número de transferências (leitura) de registros da memória externa para a memória interna;
- número de transferências (escrita) de registros da memória interna para a memória externa;
- número de comparações entre valores do campo de ordenação dos registros;
- tempo de execução (tempo do término de execução menos o tempo do início de execução).

O programa deve ser implementado de tal forma que seja possível executá-lo, livremente, a partir da seguinte linha de comando no console:
ordena <método> <quantidade> <situação> [-P]

onde:
- <método> representa o método de ordenação externa a ser executado, podendo ser um número
inteiro de 1 a 3, de acordo com a ordem dos métodos mencionados;
- <quantidade> representa a quantidade de alunos (linhas do arquivo texto) a serem ordenados;
- <situação> representa a situação de ordem inicial do arquivo, podendo ser 1 (arquivo ordenado
ascendentemente pelas notas), 2 (arquivo ordenado descendentemente pelas notas) ou 3 (arquivo
desordenado aleatoriamente pelas notas);
- [-P] representa um argumento opcional que deve ser colocado quando se deseja que os dados dos
alunos a serem ordenados e o resultado da ordenação realizada sejam apresentados na tela.
