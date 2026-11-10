#ifndef JOGADOR_H
#define JOGADOR_H
#include "jogo.h"

void iniciarJogador(Jogo *jogo, const char *nome);

/* Soma (ou subtrai) suspeita, sempre entre 0 e 100.
   Em 100 o alarme toca e o jogador e capturado. */
void atualizarSuspeita(Jogo *jogo, int valor);

#endif
