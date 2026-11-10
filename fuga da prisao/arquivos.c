#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arquivos.h"
#include "fase.h"
#include "mapa.h"

#define VERSAO_SAVE "FUGA3"
#define MAX_RECORDES 200
#define RECORDES_EXIBIDOS 10

/* ------------------------------------------------------------------ */
/* Partida salva                                                       */
/* ------------------------------------------------------------------ */
/*
 * Formato do save (texto):
 *   FUGA3
 *   <nome>
 *   <fase> <pontuacao> <suspeita> <turnos>
 *   <x> <y>
 *   <quantidade de chaves> <coletada 1> <coletada 2> ...
 *   <quantidade de guardas>
 *   <x> <y> <direcao> <vertical> <ativo>     (uma linha por guarda)
 *
 * O mapa nao e salvo: ele e reconstruido a partir do desenho da fase.
 */
int salvarJogo(const Jogo *j, const char *caminho) {
    FILE *f = fopen(caminho, "w");
    int i;
    if (!f) return 0;

    fprintf(f, "%s\n%s\n", VERSAO_SAVE, j->jogador.nome);
    fprintf(f, "%d %d %d %d\n", j->jogador.faseAtual, j->jogador.pontuacao,
            j->jogador.nivelSuspeita, j->movimentos);
    fprintf(f, "%d %d\n", j->jogador.x, j->jogador.y);
    fprintf(f, "%d", j->quantidadeChaves);
    for (i = 0; i < j->quantidadeChaves; i++) fprintf(f, " %d", j->chaves[i].coletada);
    fprintf(f, "\n%d\n", j->quantidadeGuardas);
    for (i = 0; i < j->quantidadeGuardas; i++) {
        const Guarda *g = &j->guardas[i];
        fprintf(f, "%d %d %d %d %d\n", g->x, g->y, g->direcao, g->vertical, g->ativo);
    }
    return fclose(f) == 0;
}

static int casaLivre(const Jogo *j, int x, int y) {
    return x > 0 && x < COLUNAS - 1 && y > 0 && y < LINHAS - 1 && j->mapa[y][x] != PAREDE;
}

int carregarJogo(Jogo *j, const char *caminho) {
    FILE *f = fopen(caminho, "r");
    char versao[16], nome[TAM_NOME];
    int fase, pontuacao, suspeita, turnos, chaves, guardas, i, ok = 0;
    Jogo lido; /* so substitui *j se o arquivo inteiro estiver correto */

    if (!f) return 0;
    memset(&lido, 0, sizeof lido);

    if (fscanf(f, "%15s ", versao) != 1 || strcmp(versao, VERSAO_SAVE) != 0) goto fim;
    if (!fgets(nome, sizeof nome, f)) goto fim;
    nome[strcspn(nome, "\r\n")] = '\0';
    if (fscanf(f, "%d %d %d %d", &fase, &pontuacao, &suspeita, &turnos) != 4) goto fim;
    if (fase < 1 || fase > totalFases() || suspeita < 0 || suspeita >= 100 || turnos < 0) goto fim;

    /* Recria a fase a partir do desenho e aplica o progresso por cima. */
    configurarFase(&lido, fase);
    strcpy(lido.jogador.nome, nome);
    lido.jogador.pontuacao = pontuacao;
    lido.jogador.nivelSuspeita = suspeita;
    lido.movimentos = turnos;

    if (fscanf(f, "%d %d", &lido.jogador.x, &lido.jogador.y) != 2) goto fim;
    if (!casaLivre(&lido, lido.jogador.x, lido.jogador.y)) goto fim;

    if (fscanf(f, "%d", &chaves) != 1 || chaves != lido.quantidadeChaves) goto fim;
    for (i = 0; i < chaves; i++)
        if (fscanf(f, "%d", &lido.chaves[i].coletada) != 1) goto fim;
    lido.porta.aberta = todasChavesColetadas(&lido);

    if (fscanf(f, "%d", &guardas) != 1 || guardas != lido.quantidadeGuardas) goto fim;
    for (i = 0; i < guardas; i++) {
        Guarda *g = &lido.guardas[i];
        if (fscanf(f, "%d %d %d %d %d", &g->x, &g->y, &g->direcao, &g->vertical, &g->ativo) != 5) goto fim;
        if (!casaLivre(&lido, g->x, g->y) || (g->direcao != 1 && g->direcao != -1)) goto fim;
    }

    *j = lido;
    ok = 1;
fim:
    fclose(f);
    return ok;
}

int existeSave(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) return 0;
    fclose(f);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Recordes: uma linha por partida -> "<pontos> <fase> <E|C> <nome>"   */
/* (E = escapou, C = capturado). O nome fica no fim e pode ter espaco. */
/* ------------------------------------------------------------------ */
typedef struct {
    int pontos, fase;
    char resultado;
    char nome[TAM_NOME];
} Recorde;

static Recorde lista[MAX_RECORDES];

static int lerRecordes(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    char linha[128];
    int n = 0;
    if (!f) return 0;
    while (n < MAX_RECORDES && fgets(linha, sizeof linha, f)) {
        Recorde *r = &lista[n];
        int lidos = 0;
        if (sscanf(linha, "%d %d %c %n", &r->pontos, &r->fase, &r->resultado, &lidos) < 3 || lidos == 0)
            continue; /* ignora linhas de formato antigo ou corrompidas */
        strncpy(r->nome, linha + lidos, TAM_NOME - 1);
        r->nome[TAM_NOME - 1] = '\0';
        r->nome[strcspn(r->nome, "\r\n")] = '\0';
        n++;
    }
    fclose(f);
    return n;
}

static int compararRecordes(const void *a, const void *b) {
    return ((const Recorde *)b)->pontos - ((const Recorde *)a)->pontos;
}

int registrarRecorde(const Jogo *j, const char *caminho) {
    int n = lerRecordes(caminho), i, posicao = 1;
    FILE *f;

    for (i = 0; i < n; i++)
        if (lista[i].pontos > j->jogador.pontuacao) posicao++;

    f = fopen(caminho, "a");
    if (f) {
        fprintf(f, "%d %d %c %s\n", j->jogador.pontuacao, j->jogador.faseAtual,
                j->jogador.capturado ? 'C' : 'E', j->jogador.nome);
        fclose(f);
    }
    return posicao;
}

void exibirRecordes(const char *caminho) {
    int n = lerRecordes(caminho), i;

    limparTela();
    usarCor(COR_AMARELO);
    puts("\n  ============== TOP 10 RECORDES ==============\n");
    usarCor(COR_NORMAL);
    if (n == 0) {
        puts("  Ainda nao ha recordes. Seja o primeiro!");
        return;
    }
    qsort(lista, n, sizeof lista[0], compararRecordes);
    for (i = 0; i < n && i < RECORDES_EXIBIDOS; i++) {
        usarCor(i == 0 ? COR_AMARELO : COR_NORMAL);
        printf("  %2d. %-20.20s %7d pts  ", i + 1, lista[i].nome, lista[i].pontos);
        if (lista[i].resultado == 'E') {
            usarCor(COR_VERDE);
            puts("ESCAPOU");
        } else {
            usarCor(COR_VERMELHO);
            printf("preso na fase %d\n", lista[i].fase);
        }
        usarCor(COR_NORMAL);
    }
}
