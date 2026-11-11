#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "fase.h"
#include "jogador.h"

/*
 * ======================== COMO CRIAR UMA FASE ========================
 *
 * Cada fase e SOMENTE um desenho. Para criar uma nova, copie um bloco do
 * vetor FASES e redesenhe; nada mais no codigo precisa mudar. Quantidade
 * de fases, chaves, guardas, inicio e porta sao lidos do proprio desenho.
 *
 *   #  parede                .  corredor
 *   @  inicio do jogador     C  chave           P  porta de saida
 *   >  <  guarda que patrulha na horizontal (seta = direcao inicial)
 *   ^  v  guarda que patrulha na vertical   (seta = direcao inicial)
 *
 * Regras: LINHAS linhas de COLUNAS caracteres, borda toda de '#', um '@',
 * uma 'P', de 1 a MAX_CHAVES chaves e ate MAX_GUARDAS guardas.
 * validarFases() confere tudo isso quando o jogo abre.
 * =====================================================================
 */
typedef struct {
    const char *nome;
    const char *dica;
    const char *mapa[LINHAS];
} DefinicaoFase;

static const DefinicaoFase FASES[] = {
    {
        "Bloco de Celas",
        "Os ':' mostram para onde o guarda olha. Fique fora da visao dele!",
        {
            "###############################",
            "#@......#...........#........C#",
            "#.......#...........#.........#",
            "#.......#....>......#.........#",
            "####.####...........####.######",
            "#.............................#",
            "#.............................#",
            "#######.#########.#############",
            "#...............#............P#",
            "#...............#.............#",
            "###############################"
        }
    },
    {
        "Corredores",
        "Guardas tambem patrulham na vertical. Use E para esperar a hora certa.",
        {
            "###############################",
            "#@..#..........v........#....C#",
            "#...#...................#.....#",
            "#...#...#######.#####...#.....#",
            "#.......#.....#.#...#.........#",
            "#####...#..C..#.#...#...#######",
            "#.......#.....#.....#.........#",
            "#...#####.....#######..<......#",
            "#...........................#.#",
            "#.......#...............#...#P#",
            "###############################"
        }
    },
    {
        "Refeitorio",
        "As mesas bloqueiam a visao dos guardas. Esconda-se atras delas.",
        {
            "###############################",
            "#@............#..............C#",
            "#.............#...............#",
            "#..##..##..##.#..##..##..##...#",
            "#.......>.....#.....v.........#",
            "#..##..##..##....##..##..##...#",
            "#.............#...............#",
            "######.########...<...#########",
            "#C............#...............#",
            "#.............#..............P#",
            "###############################"
        }
    },
    {
        "Patio",
        "Muitos guardas e pouco lugar para se esconder. Planeje a rota.",
        {
            "###############################",
            "#@......#.............#......C#",
            "#.......#....v........#.......#",
            "#...#####.............#####...#",
            "#.........###...###...........#",
            "#..>......#C#...#.#......<....#",
            "#.........#.#...#.#...........#",
            "#...#####.........#####...#####",
            "#.......#........^....#.......#",
            "#C......#.............#......P#",
            "###############################"
        }
    },
    {
        "Muralha",
        "Ultimo obstaculo: um labirinto de corredores. Boa sorte!",
        {
            "###############################",
            "#@..#.......>.....#.........C.#",
            "#...#.#####.#####.#.#######...#",
            "#.....#C......>...#.#.....#...#",
            "###.###.#########.#.#..C..#.###",
            "#.......#.......#...#.........#",
            "#.#####.#.#####.#####.#####.#.#",
            "#.#...#...#.v.#.......#.^.#.#.#",
            "#.#.C.#####...#########...#.#P#",
            "#.......>.....................#",
            "###############################"
        }
    }
};

#define QUANTIDADE_FASES ((int)(sizeof FASES / sizeof FASES[0]))

static int indiceFase(int numero) {
    if (numero < 1) return 0;
    if (numero > QUANTIDADE_FASES) return QUANTIDADE_FASES - 1;
    return numero - 1;
}

int totalFases(void) { return QUANTIDADE_FASES; }
const char *nomeFase(int numero) { return FASES[indiceFase(numero)].nome; }
const char *dicaFase(int numero) { return FASES[indiceFase(numero)].dica; }

static int ehGuarda(char c) { return c == '>' || c == '<' || c == '^' || c == 'v'; }

/* ------------------------------------------------------------------ */
/* Montagem da fase                                                    */
/* ------------------------------------------------------------------ */
void configurarFase(Jogo *j, int numero) {
    const DefinicaoFase *def = &FASES[indiceFase(numero)];
    int x, y;

    j->jogador.faseAtual = indiceFase(numero) + 1;
    j->quantidadeChaves = 0;
    j->quantidadeGuardas = 0;
    j->movimentos = 0;
    j->porta.aberta = 0;
    j->mensagem[0] = '\0';

    for (y = 0; y < LINHAS; y++) {
        for (x = 0; x < COLUNAS; x++) {
            char c = def->mapa[y][x];
            j->mapa[y][x] = (c == PAREDE) ? PAREDE : CHAO;

            if (c == SIMBOLO_JOGADOR) {
                j->jogador.x = x;
                j->jogador.y = y;
            } else if (c == SIMBOLO_PORTA) {
                j->porta.x = x;
                j->porta.y = y;
            } else if (c == SIMBOLO_CHAVE && j->quantidadeChaves < MAX_CHAVES) {
                Chave *ch = &j->chaves[j->quantidadeChaves++];
                ch->x = x;
                ch->y = y;
                ch->coletada = 0;
            } else if (ehGuarda(c) && j->quantidadeGuardas < MAX_GUARDAS) {
                Guarda *g = &j->guardas[j->quantidadeGuardas++];
                g->x = x;
                g->y = y;
                g->vertical = (c == '^' || c == 'v');
                g->direcao = (c == '>' || c == 'v') ? 1 : -1;
                g->ativo = 1;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Validacao dos desenhos                                              */
/* ------------------------------------------------------------------ */
static void erroFase(int indice, int *erros, const char *formato, ...) {
    va_list args;
    printf("ERRO na fase %d (%s): ", indice + 1, FASES[indice].nome);
    va_start(args, formato);
    vprintf(formato, args);
    va_end(args);
    putchar('\n');
    (*erros)++;
}

/* Busca em largura: marca as casas que o jogador consegue alcancar. */
static void marcarAlcancaveis(const DefinicaoFase *def, int inicioX, int inicioY,
                              int alcancavel[LINHAS][COLUNAS]) {
    static const int passoX[4] = {1, -1, 0, 0};
    static const int passoY[4] = {0, 0, 1, -1};
    int filaX[LINHAS * COLUNAS], filaY[LINHAS * COLUNAS];
    int inicio = 0, fim = 0, d;

    memset(alcancavel, 0, sizeof(int) * LINHAS * COLUNAS);
    alcancavel[inicioY][inicioX] = 1;
    filaX[fim] = inicioX;
    filaY[fim++] = inicioY;

    while (inicio < fim) {
        int x = filaX[inicio], y = filaY[inicio++];
        if (def->mapa[y][x] == SIMBOLO_PORTA) continue; /* a porta encerra a fase */
        for (d = 0; d < 4; d++) {
            int nx = x + passoX[d], ny = y + passoY[d];
            if (nx < 0 || nx >= COLUNAS || ny < 0 || ny >= LINHAS) continue;
            if (alcancavel[ny][nx] || def->mapa[ny][nx] == PAREDE) continue;
            alcancavel[ny][nx] = 1;
            filaX[fim] = nx;
            filaY[fim++] = ny;
        }
    }
}

static int validarFase(int indice) {
    const DefinicaoFase *def = &FASES[indice];
    int alcancavel[LINHAS][COLUNAS];
    int x, y, erros = 0, jogadores = 0, portas = 0, chaves = 0, guardas = 0;
    int inicioX = 0, inicioY = 0;

    for (y = 0; y < LINHAS; y++) {
        if (def->mapa[y] == NULL || (int)strlen(def->mapa[y]) != COLUNAS) {
            erroFase(indice, &erros, "a linha %d deve ter exatamente %d caracteres.", y, COLUNAS);
            return 0; /* sem o tamanho certo nao da para conferir o resto */
        }
    }

    for (y = 0; y < LINHAS; y++) {
        for (x = 0; x < COLUNAS; x++) {
            char c = def->mapa[y][x];
            int borda = (x == 0 || y == 0 || x == COLUNAS - 1 || y == LINHAS - 1);
            if (borda && c != PAREDE)
                erroFase(indice, &erros, "a borda precisa ser parede (linha %d, coluna %d).", y, x);
            if (c == SIMBOLO_JOGADOR) { jogadores++; inicioX = x; inicioY = y; }
            else if (c == SIMBOLO_PORTA) portas++;
            else if (c == SIMBOLO_CHAVE) chaves++;
            else if (ehGuarda(c)) guardas++;
            else if (c != PAREDE && c != CHAO)
                erroFase(indice, &erros, "simbolo desconhecido '%c' (linha %d, coluna %d).", c, y, x);
        }
    }

    if (jogadores != 1) erroFase(indice, &erros, "precisa ter exatamente um '@' (tem %d).", jogadores);
    if (portas != 1) erroFase(indice, &erros, "precisa ter exatamente uma 'P' (tem %d).", portas);
    if (chaves < 1 || chaves > MAX_CHAVES)
        erroFase(indice, &erros, "precisa ter de 1 a %d chaves (tem %d).", MAX_CHAVES, chaves);
    if (guardas > MAX_GUARDAS)
        erroFase(indice, &erros, "no maximo %d guardas (tem %d).", MAX_GUARDAS, guardas);

    if (jogadores == 1) {
        marcarAlcancaveis(def, inicioX, inicioY, alcancavel);
        for (y = 0; y < LINHAS; y++)
            for (x = 0; x < COLUNAS; x++) {
                char c = def->mapa[y][x];
                if ((c == SIMBOLO_CHAVE || c == SIMBOLO_PORTA) && !alcancavel[y][x])
                    erroFase(indice, &erros, "'%c' na linha %d, coluna %d nao pode ser alcancada.", c, y, x);
            }
    }
    return erros == 0;
}

int validarFases(void) {
    int i, ok = 1;
    for (i = 0; i < QUANTIDADE_FASES; i++)
        if (!validarFase(i)) ok = 0;
    return ok;
}

/* ------------------------------------------------------------------ */
/* Objetivos                                                           */
/* ------------------------------------------------------------------ */
int coletarChave(Jogo *j) {
    int i;
    for (i = 0; i < j->quantidadeChaves; i++) {
        Chave *ch = &j->chaves[i];
        if (!ch->coletada && ch->x == j->jogador.x && ch->y == j->jogador.y) {
            ch->coletada = 1;
            j->jogador.pontuacao += PONTOS_CHAVE;
            j->porta.aberta = todasChavesColetadas(j);
            return 1;
        }
    }
    return 0;
}

int chavesColetadas(const Jogo *j) {
    int i, total = 0;
    for (i = 0; i < j->quantidadeChaves; i++)
        if (j->chaves[i].coletada) total++;
    return total;
}

int todasChavesColetadas(const Jogo *j) {
    return chavesColetadas(j) == j->quantidadeChaves;
}

int tentarSair(Jogo *j, int x, int y) {
    if (x != j->porta.x || y != j->porta.y) return PORTA_NENHUMA;
    if (!todasChavesColetadas(j)) {
        atualizarSuspeita(j, SUSPEITA_PORTA);
        j->jogador.pontuacao -= PENALIDADE_PORTA;
        return PORTA_TRANCADA;
    }
    j->porta.aberta = 1;
    return PORTA_ABERTA;
}
