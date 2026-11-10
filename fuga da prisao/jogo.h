#ifndef JOGO_H
#define JOGO_H

/* ------------------------------------------------------------------ */
/* Dimensoes e limites                                                 */
/* ------------------------------------------------------------------ */
/* Todas as fases usam um mapa deste tamanho (os desenhos ficam em fase.c).
   A quantidade de fases e calculada sozinha: use totalFases(). */
#define LINHAS 11
#define COLUNAS 31
#define MAX_CHAVES 5
#define MAX_GUARDAS 6
#define TAM_NOME 40
#define TAM_MENSAGEM 160

/* ------------------------------------------------------------------ */
/* Regras de pontuacao e dificuldade (ajuste aqui para balancear)      */
/* ------------------------------------------------------------------ */
#define PONTOS_CHAVE 500           /* cada chave coletada                  */
#define PONTOS_FASE 1000           /* concluir uma fase                    */
#define BONUS_RAPIDEZ_MAX 1500     /* bonus maximo por terminar rapido     */
#define CUSTO_MOVIMENTO 10         /* quanto cada turno reduz esse bonus   */
#define BONUS_DISCRICAO 20         /* por ponto de suspeita abaixo de 100  */
#define PENALIDADE_PORTA 100       /* tentar abrir a porta sem as chaves   */
#define SUSPEITA_PORTA 10
#define SUSPEITA_VISTO 20          /* ser visto por um guarda              */
#define SUSPEITA_ALIVIO 2          /* turno escondido reduz a suspeita     */
#define ALCANCE_VISAO 3            /* quantas casas um guarda enxerga      */

/* ------------------------------------------------------------------ */
/* Simbolos exibidos no mapa                                           */
/* ------------------------------------------------------------------ */
#define PAREDE '#'
#define CHAO '.'
#define SIMBOLO_JOGADOR '@'
#define SIMBOLO_CHAVE 'C'
#define SIMBOLO_PORTA 'P'
#define SIMBOLO_GUARDA 'G'
#define SIMBOLO_VISAO ':'

/* ------------------------------------------------------------------ */
/* Estruturas                                                          */
/* ------------------------------------------------------------------ */
typedef struct {
    int x, y;
    int faseAtual;
    int nivelSuspeita;   /* 0 a 100; em 100 o alarme toca */
    int pontuacao;
    int capturado;
    char nome[TAM_NOME];
} Jogador;

/* O guarda anda em linha reta e da meia-volta ao bater em algo.
   direcao: +1 ou -1 ao longo do eixo (horizontal ou vertical). */
typedef struct {
    int x, y;
    int direcao;
    int vertical;
    int ativo;
} Guarda;

typedef struct {
    int x, y;
    int coletada;
} Chave;

typedef struct {
    int x, y;
    int aberta;          /* abre sozinha quando todas as chaves sao pegas */
} Porta;

typedef struct {
    char mapa[LINHAS][COLUNAS];   /* somente paredes e chao */
    Jogador jogador;
    Guarda guardas[MAX_GUARDAS];
    int quantidadeGuardas;
    Chave chaves[MAX_CHAVES];
    int quantidadeChaves;
    Porta porta;
    int movimentos;               /* turnos gastos na fase atual */
    char mensagem[TAM_MENSAGEM];  /* avisos mostrados abaixo do mapa */
} Jogo;

#endif
