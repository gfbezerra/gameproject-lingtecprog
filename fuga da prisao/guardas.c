#include "guardas.h"

static int passoX(const Guarda *g) { return g->vertical ? 0 : g->direcao; }
static int passoY(const Guarda *g) { return g->vertical ? g->direcao : 0; }

/* Guardas nao atravessam paredes nem param em cima da porta de saida. */
static int bloqueadoParaGuarda(const Jogo *j, int x, int y) {
    if (x < 0 || x >= COLUNAS || y < 0 || y >= LINHAS) return 1;
    if (j->mapa[y][x] == PAREDE) return 1;
    return x == j->porta.x && y == j->porta.y;
}

int atualizarGuardas(Jogo *j) {
    int i;
    for (i = 0; i < j->quantidadeGuardas; i++) {
        Guarda *g = &j->guardas[i];
        if (!g->ativo) continue;

        /* Bateu em algo? Da meia-volta e continua a patrulha. */
        if (bloqueadoParaGuarda(j, g->x + passoX(g), g->y + passoY(g)))
            g->direcao = -g->direcao;
        if (!bloqueadoParaGuarda(j, g->x + passoX(g), g->y + passoY(g))) {
            g->x += passoX(g);
            g->y += passoY(g);
        }

        if (g->x == j->jogador.x && g->y == j->jogador.y) j->jogador.capturado = 1;
    }
    return j->jogador.capturado;
}

int guardaNaPosicao(const Jogo *j, int x, int y) {
    int i;
    for (i = 0; i < j->quantidadeGuardas; i++)
        if (j->guardas[i].ativo && j->guardas[i].x == x && j->guardas[i].y == y) return 1;
    return 0;
}

/* A visao segue em linha reta para onde o guarda anda e para na parede. */
int celulaVigiada(const Jogo *j, int x, int y) {
    int i, passo;
    for (i = 0; i < j->quantidadeGuardas; i++) {
        const Guarda *g = &j->guardas[i];
        if (!g->ativo) continue;
        for (passo = 1; passo <= ALCANCE_VISAO; passo++) {
            int vx = g->x + passoX(g) * passo, vy = g->y + passoY(g) * passo;
            if (vx < 0 || vx >= COLUNAS || vy < 0 || vy >= LINHAS) break;
            if (j->mapa[vy][vx] == PAREDE) break;
            if (vx == x && vy == y) return 1;
        }
    }
    return 0;
}

int guardaVeJogador(const Jogo *j) {
    return celulaVigiada(j, j->jogador.x, j->jogador.y);
}
