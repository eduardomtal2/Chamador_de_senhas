#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQUIVO "senhas.csv"

/* ======================================================
   FILA - guarda as senhas esperando para ser chamadas
   Cada no tem: numero da senha, tipo e proximo no
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
    if (!novo) { printf("  Erro: sem memoria!\n"); return; }

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

/* Remove e retorna a senha de maior prioridade (P > N) */
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

/* Lista a fila na ordem em que as senhas serao chamadas (P > N) */
void listar_fila(void) {
    if (frente == NULL) { printf("  Fila vazia.\n"); return; }

    char prioridade[] = {'P', 'N'};
    const char *nomes[] = {"Prioritario", "Normal"};
    int pos = 1;

    for (int t = 0; t < 2; t++) {
        NoFila *senha_atual = frente;
        while (senha_atual != NULL) {
            if (senha_atual->tipo == prioridade[t]) {
                printf("  %d) Senha %c%d  [%s]\n",
                       pos, senha_atual->tipo, senha_atual->numero, nomes[t]);
                pos++;
            }
            senha_atual = senha_atual->prox;
        }
    }
}

/* Libera todos os nos da fila */
void liberar_fila(void) {
    NoFila *senha_atual = frente;
    while (senha_atual != NULL) {
        NoFila *prox = senha_atual->prox;
        free(senha_atual);
        senha_atual = prox;
    }
    frente = tras = NULL;
}

/* ======================================================
   PILHA - guarda o historico das senhas ja chamadas
   Permite desfazer: a ultima senha chamada volta para
   o inicio da fila (reinsercao como prioridade maxima)
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
    if (!novo) { printf("  Erro: sem memoria!\n"); return; }
    novo->tipo   = tipo;
    novo->numero = numero;
    novo->prox   = topo;
    topo = novo;
}

/* Desempilha e devolve o no do topo (ou NULL se vazia) */
NoPilha *desempilhar(void) {
    if (topo == NULL) return NULL;
    NoPilha *removido = topo;
    topo = topo->prox;
    removido->prox = NULL;
    return removido;
}

/* Desfaz a ultima chamada: retira da pilha e reinserese na frente da fila */
void desfazer_chamada(void) {
    NoPilha *ult = desempilhar();
    if (ult == NULL) {
        printf("  Nada para desfazer.\n");
        return;
    }

    /* Reinsere na FRENTE da fila para ser chamada novamente */
    NoFila *novo = (NoFila *)malloc(sizeof(NoFila));
    if (!novo) { printf("  Erro: sem memoria!\n"); free(ult); return; }
    novo->tipo   = ult->tipo;
    novo->numero = ult->numero;
    novo->prox   = frente;
    frente = novo;
    if (tras == NULL) tras = novo;   /* fila estava vazia */

    printf("  Senha %c%d retornou para a fila de espera.\n",
           ult->tipo, ult->numero);
    free(ult);
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

/* Libera todos os nos da pilha */
void liberar_pilha(void) {
    NoPilha *chamada_atual = topo;
    while (chamada_atual != NULL) {
        NoPilha *prox = chamada_atual->prox;
        free(chamada_atual);
        chamada_atual = prox;
    }
    topo = NULL;
}

/* ======================================================
   PERSISTENCIA - salva e carrega o arquivo senhas.csv
   Formato:
     CONTADORES <normal> <prior>
     FILA <tipo> <numero>          (uma linha por senha na fila)
     PILHA <tipo> <numero>         (do fundo ao topo, para recarregar certo)
   ====================================================== */

void salvar(void) {
    FILE *fp = fopen(ARQUIVO, "w");
    if (!fp) { printf("  Erro ao salvar.\n"); return; }

    /* Salva contadores */
    fprintf(fp, "CONTADORES %d %d\n",
            contador_normal, contador_prior);

    /* Salva fila (ordem de insercao) */
    NoFila *cf = frente;
    while (cf != NULL) {
        fprintf(fp, "FILA %c %d\n", cf->tipo, cf->numero);
        cf = cf->prox;
    }

    /* Salva pilha do fundo para o topo para que, ao recarregar com
       empilhar(), a ordem fique identica a original. */
    int sz = 0;
    NoPilha *cp = topo;
    while (cp != NULL) { sz++; cp = cp->prox; }

    if (sz > 0) {
        NoPilha **arr = (NoPilha **)malloc(sz * sizeof(NoPilha *));
        if (!arr) { printf("  Aviso: nao foi possivel salvar historico.\n"); }
        else {
            cp = topo;
            for (int i = 0; i < sz; i++) { arr[i] = cp; cp = cp->prox; }
            for (int i = sz - 1; i >= 0; i--)
                fprintf(fp, "PILHA %c %d\n", arr[i]->tipo, arr[i]->numero);
            free(arr);
        }
    }

    fclose(fp);
    printf("  Dados salvos!\n");
}

void carregar(void) {
    FILE *fp = fopen(ARQUIVO, "r");
    if (!fp) return;  /* primeira execucao, arquivo ainda nao existe */

    char secao[20];
    char tipo;
    int numero;

    while (fscanf(fp, "%19s", secao) == 1) {
        if (strcmp(secao, "CONTADORES") == 0) {
            fscanf(fp, "%d %d",
                   &contador_normal, &contador_prior);

        } else if (strcmp(secao, "FILA") == 0) {
            if (fscanf(fp, " %c %d", &tipo, &numero) != 2) break;
            /* Recria o no direto sem usar enfileirar() para nao alterar contadores */
            NoFila *novo = (NoFila *)malloc(sizeof(NoFila));
            if (!novo) break;
            novo->tipo   = tipo;
            novo->numero = numero;
            novo->prox   = NULL;
            if (tras == NULL) frente = tras = novo;
            else { tras->prox = novo; tras = novo; }

        } else if (strcmp(secao, "PILHA") == 0) {
            if (fscanf(fp, " %c %d", &tipo, &numero) != 2) break;
            empilhar(tipo, numero);
        }
    }
    fclose(fp);
}

/* ======================================================
   UTILITARIOS
   ====================================================== */

int contar_fila(void) {
    int n = 0;
    NoFila *senha_atual = frente;
    while (senha_atual != NULL) { n++; senha_atual = senha_atual->prox; }
    return n;
}

/* Le um inteiro do teclado com seguranca; descarta lixo no buffer */
int ler_opcao(void) {
    int val;
    if (scanf("%d", &val) != 1) {
        /* entrada invalida: limpa o buffer ate a proxima quebra de linha */
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return -1;
    }
    return val;
}

/* ======================================================
   MENU PRINCIPAL
   ====================================================== */

int main(void) {
    carregar();

    int opcao;
    do {
        printf("\n=== CHAMADOR DE SENHAS ===\n");
        printf("Fila: %d senha(s)\n\n", contar_fila());
        printf("1 - Gerar senha Normal\n");
        printf("2 - Gerar senha Prioritaria\n");
        printf("3 - Chamar proxima senha\n");
        printf("4 - Desfazer ultima chamada\n");
        printf("5 - Listar fila de espera\n");
        printf("6 - Ver historico de chamadas\n");
        printf("0 - Sair\n");
        printf("Escolha: ");

        opcao = ler_opcao();

        switch (opcao) {
            case 1:
                enfileirar('N');
                printf("  Senha N%d gerada!\n", contador_normal - 1);
                break;

            case 2:
                enfileirar('P');
                printf("  Senha P%d gerada!\n", contador_prior - 1);
                break;

            case 3: {
                NoFila *chamado = desenfileirar();
                if (chamado == NULL) {
                    printf("  Nenhuma senha na fila!\n");
                } else {
                    printf("\n  >>> CHAMAR SENHA %c%d <<<\n",
                           chamado->tipo, chamado->numero);
                    empilhar(chamado->tipo, chamado->numero);
                    free(chamado);
                }
                break;
            }

            case 4:
                desfazer_chamada();
                break;

            case 5:
                printf("\n  --- Fila de espera (ordem de chamada) ---\n");
                listar_fila();
                break;

            case 6:
                printf("\n  --- Historico (mais recente primeiro) ---\n");
                listar_historico();
                break;

            case 0:
                break;  /* sai do loop */

            default:
                printf("  Opcao invalida. Tente novamente.\n");
                break;
        }

    } while (opcao != 0);

    salvar();
    liberar_fila();
    liberar_pilha();
    printf("  Ate logo!\n");
    return 0;
}
