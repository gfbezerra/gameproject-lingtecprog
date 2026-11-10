#ifndef MAPA_H
#define MAPA_H
#include "jogo.h"

/* ------------------------------------------------------------------ */
/* Tela: limpar, redesenhar no mesmo lugar e cores                     */
/* ------------------------------------------------------------------ */
#define COR_NORMAL 0
#define COR_AMARELO_ESCURO 33
#define COR_CINZA_CLARO 37
#define COR_CINZA 90
#define COR_VERMELHO 91
#define COR_VERDE 92
#define COR_AMARELO 93
#define COR_CIANO 96
#define COR_BRANCO 97

void iniciarTela(void);         /* chamar uma vez no inicio do programa */
void finalizarTela(void);       /* devolve o terminal ao normal          */
void limparTela(void);
void usarCor(int cor);          /* ignorado se o terminal nao tiver cores */
void mostrarCursor(int visivel);

/* Desenha painel, mapa, mensagens e legenda. Jogada apos jogada o mapa e
   redesenhado por cima do anterior, sem rolar a tela e sem piscar. */
void exibirMapa(const Jogo *jogo);

/* ------------------------------------------------------------------ */
/* Movimento do jogador                                                */
/* ------------------------------------------------------------------ */
#define MOV_INVALIDO 0        /* tecla que nao e de movimento           */
#define MOV_PAREDE 1          /* bateu na parede: nao gasta turno       */
#define MOV_OK 2              /* andou uma casa (ou esperou com E)      */
#define MOV_PORTA_TRANCADA 3  /* tentou sair sem as chaves: gasta turno */
#define MOV_SAIU 4            /* entrou na porta aberta: fase concluida */

int moverJogador(Jogo *jogo, char comando);

#endif
