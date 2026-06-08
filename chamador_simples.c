#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQUIVO "senhas.csv"

/* ======================================================
   FILA - guarda as senhas esperando para ser chamadas
   Cada nó tem: numero da senha, tipo e proximo nó
     Tipo: 'N' = Normal, 'P' = Prioritario
     A fila é encadeada, com ponteiros para o primeiro (frente) e ultimo (tras)
     Contadores separados para cada tipo de senha (para gerar o numero correto)
     A função desenfileirar percorre a fila procurando a senha de maior prioridade (P > E > N)
     A função listar_fila percorre a fila do inicio ao fim mostrando as senhas na ordem de espera
     A função limpar_fila remove todos os nós da fila e reseta os contadores
     A função contar_fila conta quantas senhas tem na fila (usada no menu para mostrar o numero)
     A função salvar grava a fila no arquivo (tipo e numero de cada nó)
     A função carregar recria a fila a partir do arquivo (le o tipo e numero de cada nó e recria a estrutura encadeada)
   ====================================================== */

typedef struct NoFila {
    int numero;
    char tipo;          /* 'N' = Normal, 'P' = Prioritario */
    struct NoFila *prox;
} NoFila;

NoFila *frente = NULL;  /* primeiro da fila */
NoFila *tras   = NULL;  /* ultimo da fila   */

int contador_normal = 1;
int contador_prior  = 1;

/* Adiciona uma senha no final da fila */
void enfileirar(char tipo) {
    NoFila *novo = (NoFila *)malloc(sizeof(NoFila));

    novo->tipo = tipo;
    novo->prox = NULL;

    if (tipo == 'P') novo->numero = contador_prior++;
    else novo->numero = contador_normal++;

    if (tras == NULL) {
        frente = tras = novo;
    } else {
        tras->prox = novo;
        tras = novo;
    }
}

/* Remove e retorna a senha de maior prioridade (P > E > N) */
NoFila *desenfileirar(void) {
    if (frente == NULL) return NULL;

    char prioridade[] = {'P', 'N'};

    for (int t = 0; t < 2; t++) {
        NoFila *prev        = NULL;
        NoFila *senha_atual = frente;

        while (senha_atual != NULL) {
            if (senha_atual->tipo == prioridade[t]) {
                if (prev == NULL) frente = senha_atual->prox;
                else prev->prox = senha_atual->prox;
                if (senha_atual == tras) tras = prev;
                senha_atual->prox = NULL;
                return senha_atual;
            }
            prev        = senha_atual;
            senha_atual = senha_atual->prox;
        }
    }
    return NULL;
}

void listar_fila(void) {
    if (frente == NULL) { printf("  Fila vazia.\n"); return; }
    NoFila *senha_atual = frente;
    int pos = 1;
    while (senha_atual != NULL) {
        printf("  %d) Senha %c%d\n", pos, senha_atual->tipo, senha_atual->numero);
        senha_atual = senha_atual->prox;
        pos++;
    }
}

/* ======================================================
   PILHA - guarda o historico das senhas ja chamadas
   Cada no tem: numero, tipo e o proximo no (abaixo)
   ====================================================== */

typedef struct NoPilha {
    int numero;
    char tipo;
    struct NoPilha *prox;
} NoPilha;

NoPilha *topo = NULL;  /* senha mais recente */

/* Empilha uma senha no historico */
void empilhar(char tipo, int numero) {
    NoPilha *novo = (NoPilha *)malloc(sizeof(NoPilha));
    novo->tipo   = tipo;
    novo->numero = numero;
    novo->prox   = topo;
    topo = novo;
}

void listar_historico(void) {
    if (topo == NULL) { printf("  Historico vazio.\n"); return; }
    NoPilha *chamada_atual = topo;
    int pos = 1;
    while (chamada_atual != NULL) {
        printf("  %d) Senha %c%d\n", pos, chamada_atual->tipo, chamada_atual->numero);
        chamada_atual = chamada_atual->prox;
        pos++;
    }
}

/* ======================================================
   PERSISTENCIA - salva e carrega o arquivo senhas.csv
   ====================================================== */

void salvar(void) {
    FILE *fp = fopen(ARQUIVO, "w");
    if (!fp) { printf("Erro ao salvar.\n"); return; }

    /* Salva contadores */
    fprintf(fp, "CONTADORES %d %d %d\n",
            contador_normal, contador_prior);

    /* Salva fila */
    NoFila *cf = frente;
    while (cf != NULL) {
        fprintf(fp, "FILA %c %d\n", cf->tipo, cf->numero);
        cf = cf->prox;
    }

    /* Salva pilha (do fundo para o topo para recarregar na ordem certa) */
    /* Primeiro conta quantos tem */
    int sz = 0;
    NoPilha *cp = topo;
    while (cp != NULL) { sz++; cp = cp->prox; }

    /* Guarda ponteiros num array */
    NoPilha **arr = (NoPilha **)malloc(sz * sizeof(NoPilha *));
    cp = topo;
    for (int i = 0; i < sz; i++) { arr[i] = cp; cp = cp->prox; }

    /* Grava de baixo para cima */
    for (int i = sz - 1; i >= 0; i--)
        fprintf(fp, "PILHA %c %d\n", arr[i]->tipo, arr[i]->numero);

    free(arr);
    fclose(fp);
    printf("  Dados salvos!\n");
}

void carregar(void) {
    FILE *fp = fopen(ARQUIVO, "r");
    if (!fp) return;  /* primeira vez, arquivo nao existe ainda */

    char secao[12], tipo;
    int numero;

    while (fscanf(fp, "%s", secao) == 1) {
        if (strcmp(secao, "CONTADORES") == 0) {
            fscanf(fp, "%d %d %d",
                   &contador_normal, &contador_prior);

        } else if (strcmp(secao, "FILA") == 0) {
            fscanf(fp, " %c %d", &tipo, &numero);
            /* Recria o no direto sem usar enfileirar (nao incrementa contador) */
            NoFila *novo = (NoFila *)malloc(sizeof(NoFila));
            novo->tipo   = tipo;
            novo->numero = numero;
            novo->prox   = NULL;
            if (tras == NULL) frente = tras = novo;
            else { tras->prox = novo; tras = novo; }

        } else if (strcmp(secao, "PILHA") == 0) {
            fscanf(fp, " %c %d", &tipo, &numero);
            empilhar(tipo, numero);
        }
    }
    fclose(fp);
}

/* ======================================================
   LIMPEZA - remove itens da fila 
   ====================================================== */

void limpar_fila(void) {
    NoFila *senha_atual = frente;
    while (senha_atual != NULL) {
        NoFila *proximo = senha_atual->prox;
        free(senha_atual);
        senha_atual = proximo;
    }
    frente = tras = NULL;
    contador_normal = contador_prior = 1;
}

/* ======================================================
   MENU
   ====================================================== */

int contar_fila(void) {
    int n = 0;
    NoFila *senha_atual = frente;
    while (senha_atual != NULL) { n++; senha_atual = senha_atual->prox; }
    return n;
}

int main(void) {
    carregar();

    int opcao;
    do {
        printf("\n=== CHAMADOR DE SENHAS ===\n");
        printf("Fila: %d senha(s)\n\n", contar_fila());
        printf("1 - Gerar senha Normal\n");
        printf("2 - Gerar senha Prioritaria\n");
        printf("3 - Chamar proxima senha\n");
        printf("4 - Listar fila de espera\n");
        printf("5 - Ver historico de chamadas\n");
        printf("6 - Limpar fila de espera\n");
        printf("0 - Sair\n");
        printf("Escolha: ");
        scanf("%d", &opcao);

        if (opcao == 1) {
            enfileirar('N');
            printf("  Senha N%d gerada!\n", contador_normal - 1);

        } else if (opcao == 2) {
            enfileirar('P');
            printf("  Senha P%d gerada!\n", contador_prior - 1);

        } else if (opcao == 3) {
            NoFila *chamado = desenfileirar();
            if (chamado == NULL) {
                printf("  Nenhuma senha na fila!\n");
            } else {
                printf("\n  >>> CHAMAR SENHA %c%d <<<\n",
                       chamado->tipo, chamado->numero);
                empilhar(chamado->tipo, chamado->numero);
                free(chamado);
            }

        } else if (opcao == 4) {
            printf("\n  --- Fila de espera ---\n");
            listar_fila();

        } else if (opcao == 5) {
            printf("\n  --- Historico (mais recente primeiro) ---\n");
            listar_historico();

        } else if (opcao == 6) {
            limpar_fila();
            printf("  Fila de espera limpa!\n");
        } 
    } while (opcao != 0);

    salvar();
    printf("  Ate logo!\n");
    return 0;
}