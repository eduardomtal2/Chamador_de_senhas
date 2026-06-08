# Chamador_de_senhas
Projeto desenvolvido para disciplina de Algoritmos e Estruturas de Dados.

1. Descrição do problema
O sistema foi desenvolvido para auxiliar no gerenciamento de filas de atendimento, permitindo a emissão de senhas normais e prioritárias, a organização da ordem de chamada e o registro do histórico de atendimentos realizados. Além disso, os dados são armazenados em arquivo, garantindo sua recuperação em futuras execuções do programa.

2. Estruturas de dados utilizadas e o porquê da escolha.

Foram empregadas duas estruturas de dados dinâmicas:
Fila encadeada: utilizada para armazenar as senhas que aguardam atendimento, pois representa adequadamente a lógica de espera e permite inserções eficientes.
Pilha encadeada: utilizada para registrar o histórico das senhas chamadas, possibilitando também o desfazimento da última chamada realizada.

3. Formato do arquivo de persistência

Os dados são armazenados no arquivo "senhas.csv", contendo:
- Os contadores das senhas normais e prioritárias;
- As senhas presentes na fila de espera;
- O histórico das senhas já chamadas.

Exemplo:
CONTADORES 6 4
FILA N1
FILA P1
PILHA P2

Cada linha identifica o tipo de informação armazenada e os respectivos dados necessários para reconstrução do sistema.

4. Limitações conhecidas

- As senhas prioritárias possuem precedência absoluta sobre as senhas normais.
- O sistema opera exclusivamente em ambiente de terminal, sem interface gráfica.
- Não há suporte para múltiplos usuários simultâneos.
- O arquivo de persistência utiliza uma estrutura própria, não seguindo o padrão CSV convencional.
- Em caso de encerramento inesperado do programa, alterações ainda não salvas podem ser perdidas.
