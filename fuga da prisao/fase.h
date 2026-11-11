#ifndef FASE_H
#define FASE_H
#include "jogo.h"

/* Resultado de tentarSair */
#define PORTA_NENHUMA 0    /* a casa nao e a porta              */
#define PORTA_TRANCADA -1  /* faltam chaves: penalidade aplicada */
#define PORTA_ABERTA 1     /* pode sair: fase concluida          */

int totalFases(void);
const char *nomeFase(int numero);
const char *dicaFase(int numero);

/* Confere o desenho de todas as fases (tamanho, borda, simbolos e se as
   chaves e a porta podem ser alcancadas). Mostra os erros; 1 = tudo ok. */
int validarFases(void);

/* Monta mapa, chaves, guardas, porta e posicao inicial a partir do desenho. */
void configurarFase(Jogo *jogo, int numero);

int coletarChave(Jogo *jogo);
int chavesColetadas(const Jogo *jogo);
int todasChavesColetadas(const Jogo *jogo);

/* Chamada quando o jogador tenta entrar na casa (x, y). */
int tentarSair(Jogo *jogo, int x, int y);

#endif
