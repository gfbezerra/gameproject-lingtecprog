#include <string.h>
#include "jogador.h"

void iniciarJogador(Jogo *j, const char *nome) {
    memset(&j->jogador, 0, sizeof j->jogador);
    strncpy(j->jogador.nome, nome, TAM_NOME - 1);
    j->jogador.faseAtual = 1;
}

void atualizarSuspeita(Jogo *j, int valor) {
    j->jogador.nivelSuspeita += valor;
    if (j->jogador.nivelSuspeita < 0) j->jogador.nivelSuspeita = 0;
    if (j->jogador.nivelSuspeita >= 100) {
        j->jogador.nivelSuspeita = 100;
        j->jogador.capturado = 1;
    }
}
