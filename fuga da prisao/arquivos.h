#ifndef ARQUIVOS_H
#define ARQUIVOS_H
#include "jogo.h"

#define ARQUIVO_SAVE "save.txt"
#define ARQUIVO_RECORDES "recordes.txt"

int salvarJogo(const Jogo *jogo, const char *caminho);
int carregarJogo(Jogo *jogo, const char *caminho);   /* 0 se ausente ou invalido */
int existeSave(const char *caminho);

/* Grava o resultado da partida e devolve a posicao no ranking (1 = melhor). */
int registrarRecorde(const Jogo *jogo, const char *caminho);
void exibirRecordes(const char *caminho);

#endif
