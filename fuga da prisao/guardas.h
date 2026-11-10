#ifndef GUARDAS_H
#define GUARDAS_H
#include "jogo.h"

/* Move todos os guardas um passo. Retorna 1 se algum alcancou o jogador. */
int atualizarGuardas(Jogo *jogo);

/* 1 se existe um guarda ativo exatamente na casa (x, y). */
int guardaNaPosicao(const Jogo *jogo, int x, int y);

/* 1 se a casa (x, y) esta no campo de visao de algum guarda. */
int celulaVigiada(const Jogo *jogo, int x, int y);

/* 1 se algum guarda esta vendo o jogador agora. */
int guardaVeJogador(const Jogo *jogo);

#endif
