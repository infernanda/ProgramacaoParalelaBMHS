// * ***Fernanda Ferreira Verdelho*** * //

#include <stdio.h> // biblioteca padrão
#include <stdlib.h> // Biblioteca de locação de memória, controle de processos, conversões e outras.
#include <string.h> //Biblioteca String
#include <omp.h> //incluindo a biblioteca de funções do openmp

//Numero de threads(1, 2, 4)
#define THREADS 4
//Numero de entradas(10000,20000,40000)
#define N 10000 
//define algumas entradas
#define MAX 256 //MAX char table (ASCII)
#define MAX_WORDSIZE 100 //tamanho do vetor
#define MAX_DATABASE 1000001 // tamanho da database
#define DNA_SECTIONS 10 // quantas seções de dna serão analisadas
//MPI
#define MASTER 0
#define ROOT 0
//arquivos de entrada e saída declarado por ponteiros
FILE *fdatabase, *fquery, *fout;

// Boyers-Moore-Hospool-Sunday algorithm for string matching
int bmhs(char *string, int n, char *substr, int m) {

	int d[MAX]; // d MAX char table 
	int i, j, k; //declaração de variáveis 

	// Pré- processamento para se obter a tabela de deslocamento 
	for (j = 0; j < MAX; j++)
		d[j] = m + 1; //
	for (j = 0; j < m; j++)
		d[(int) substr[j]] = m - j;//

	// Buscando
	i = m - 1;
	while (i < n) { //condição de repetição no caso ele busca m-1 < n
		k = i;
		j = m - 1;
		while ((j >= 0) && (string[k] == substr[j])) {
            //Pesquisa por um sufixo do texto(janela) que casa com o padrão (sufixo padrão)
			j--; //Valor de j na mesma linha do decremento
			k--; //Valor de k na mesma linha do decremento
		}
		if (j < 0) //se j menor que 0 retorna k + 1
			return k + 1; //Casamento na posição k + 1
		i = i + d[(int) string[i + 1]];
        //Deslocamento da janela de acordo com o valor de deslocamento relatico ao caractere que está na 
        //i-nésima posição do texto, ou seja, a posição seguinte ao ultimo caractere do padrão seguinte ao ultimo
        //caractere do padrão da *string (sunday)
	}

	return -1; // em caso de nenhuma correspondência
}

// remove o "\n" do final de line
void remove_eol(char *line) {
	int i = strlen(line) - 1;
	while (line[i] == '\n' || line[i] == '\r') {
		line[i] = 0;
		i--;
	}
}

// abre os arquivos dna.in, query.in e dna.out para leitura e escrita
void openfiles() {

	fdatabase = fopen("dna.in", "r+");
	if (fdatabase == NULL) {
		perror("dna.in");
	}

	fquery = fopen("query.in", "r");
	if (fquery == NULL) {
		perror("query.in");
	}

	fout = fopen("dna.out", "w"); //importante pois é onde vai estar a saida
	if (fout == NULL) {
		perror("fout");
	}

}
// fecha os arquivos fdatabase e fquery após leitura e escrita quando chamado
void closefiles() {
	fflush(fdatabase);
	fclose(fdatabase);

	fflush(fquery);
	fclose(fquery);

	fflush(fout);
}

char **bases; //quando uso ponteiro com ** eu defino o meu vetor ponteiro como uma matriz, aqui para o DNA
char *str; //ponteiro para entrada de queries
	
//usando a dica do professor deu em sala de aula 
int main() {

    //define os limites inferiores e superiores e os manda para os processos restantes através da função
        bases = (char**) malloc(DNA_SECTIONS * sizeof(char*)); 
        if (bases == NULL) { //Retorna NULL em caso de erro
		perror("malloc");//caso o arquivo não exista imprime malloc
		exit(EXIT_FAILURE);
         }
         for (int i = 0; i< DNA_SECTIONS; i++) {
             bases[i] = (char*) malloc(sizeof(char) * MAX_DATABASE); //Alocar um novo bloco de memória
             if (bases[i] == NULL) { //Retorna NULL em caso de erro
             perror("malloc");//caso o arquivo não exista imprime malloc
             exit(EXIT_FAILURE);
             }
        }

    str = (char*) malloc(sizeof(char) * MAX_DATABASE);//Alocar um novo bloco de memória
    if (str == NULL) {//Retorna NULL em caso de erro
    perror("malloc str");//caso o arquivo não exista imprime malloc
	exit(EXIT_FAILURE);
    } 

	openfiles();
    
	//Como estratégica principal, iniciamos o processo master = 0 , entrada e escrita dos 
	//nos arquivos deve ser feita apenas pelo processo 0. 
	char desc_dna[DNA_SECTIONS][MAX_WORDSIZE];
    char desc_query[MAX_WORDSIZE]; //declara localmente vetores para IDs das sequências de DNA do 
	char line[MAX_WORDSIZE]; //declara localmente vetor auxiliar a line
	int i=0, j=0, found, result[DNA_SECTIONS]; //declara localmente variaveis auxiliares usadas no controle da

	//laço auxiliar para dados não correspondentes 
	// Com isso ele recebe o vetor para evitar lixo na memoria no processamento algoritmo
	for (int aux = 0; aux < DNA_SECTIONS; aux++)
		result[aux] = -1;							
    // início da leitura da sequências de DNA 	
	fgets(line, MAX_WORDSIZE, fdatabase);// lê a ID da 1ª sequência de DNA
	remove_eol(line);// chama função que remove o "\n" do final de "line"
	// laço externo que percorre ás sequências de DNA para armazená-las na matriz de bases
    while (!feof(fdatabase)) {
		strcpy(desc_dna[i], line);// guarda em "desc_dna" a ID da sequência de DNA
		bases[i][0] = 0; // seta primeira posição da coluna "i" de "bases" com 0
		fgets(line, MAX_WORDSIZE, fdatabase);// lê as 80 bases de cada linha da sequência e joga no auxiliar "line"
		remove_eol(line); // chama função que remove o "\n" do final de "line"
		// 1º laço interno: lê uma sequência de DNA até o final
		do {
			strcat(bases[i], line);	// concactena a próxima linha da sequência de DNA com bases[i]
			if (fgets(line, MAX_WORDSIZE, fdatabase) == NULL)
				break;
			remove_eol(line); // chama função que remove o "\n" do final de "line" e deixa apenas o conteúdo de uma linha
			j += 80;// atualiza o índice "j" para andar na coluna "bases[i]" de acordo com o tamanho da linha (80) 		
		} while (line[0] != '>');
		i++;
		j = 0;

    }
	//Fazendo a leitura das querys
	fgets(desc_query, MAX_WORDSIZE, fquery); // lê a ID da 1ª query
	remove_eol(desc_query);	 // chama função que remove o "\n" do final de "desc_query"
	
    //2laço: em relação às query 
	while (!feof(fquery)) {
		fprintf(fout, "%s\n", desc_query);
		// prepara-se para ler uma nova query
		fgets(line, MAX_WORDSIZE, fquery);// bases da query de 99 em 99 e joga no auxiliar line
		remove_eol(line); // remove o "\n" do final de line
		str[0] = 0;								
		i = 0;										
		// 1º laço interno: lê uma query até o final
		do {										
			strcat(str + i, line);				
			if (fgets(line, MAX_WORDSIZE, fquery) == NULL)
				break;
			remove_eol(line);// chama função que remove o "\n" do final
			i += strlen(line);
		} while (line[0] != '>');
		strcpy(desc_query, line);

	// prepara-se para começar uma busca
	found = 0;// assume que não vai encontrar setando "found" com 0
    //para os testes de eficiência agregamos o N no schedule(guided,N)
        #pragma omp parallel for firstprivate(str) schedule(guided) num_threads(THREADS)
		for(int id =0; id < DNA_SECTIONS; id++) {
			result[id] = bmhs(bases[id], strlen(bases[id]), str, strlen(str)); //chama função bmhs 
			if (result > 0) { //quando encontrar uma correspondência
				//fprintf(fout, "%s\n%d\n", desc_dna[id], *result);
				found++; //// incrementa o contador "found"
			}
		}
    if (!found)//se ainda não foi encontrado
    //imprimir cadeia de caracteres formatadas em um fluxo, no caso do fout/erro
    fprintf(fout, "NOT FOUND\n");
    else
    	for (int aux = 0; aux < DNA_SECTIONS; aux++)
			if (result[aux] > 0) 
			fprintf(fout,"%s\n%d\n", desc_dna[aux], result[aux]);
    }
    closefiles(); //fecha os arquivos
    
	//Desalocar memória alocada
    free(str); // desaloca o ponteiro de str
	free(bases); //desaloca a matriz/ponteiro de base
	return EXIT_SUCCESS;
}