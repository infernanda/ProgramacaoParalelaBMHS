# Programação Paralela

Trabalhos de OpenMP e MPI 

Universidade Federal do Paraná - UFPR 

Mestrado em Informática

O BMHS (Boyer-Moore-Hospool-Sunday), é um algoritmo amplamente usado para correspondência de ‘strings’. A ideia por trás do algoritmo BMHS é basicamente o igual ao BMH(Boyer-Moore-Hospool), onde ele passa pela linha e se ocorrer uma incompatibilidade na leitura, o processo se deslocado para a direita. BMHS literalmente leva esta ideia um passo à frente.
O algoritmo BMHS calcula com base no caractere diretamente à direita da janela de texto, alinhado com o caractere mais à direita da linha. O valor do salto é calculado com base na última ocorrência desse personagem. O algoritmo BMHS permite um salto máximo de comprimento da linha (caracteres no nosso caso ‘DNA’) + 1, tornando-a capaz de fazer saltos maiores. 
