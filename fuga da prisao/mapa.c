#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "mapa.h"
#include "fase.h"
#include "guardas.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define SAIDA_E_TERMINAL() _isatty(_fileno(stdout))
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <unistd.h>
#define SAIDA_E_TERMINAL() isatty(STDOUT_FILENO)
#endif

/* Largura fixa de cada linha do quadro: ao redesenhar por cima, os espacos
   do final apagam o que sobrou do quadro anterior. */
#define LARGURA_TELA 76
#define LINHAS_MENSAGEM 2

static int saidaInterativa = 0; /* janela de terminal (nao arquivo/pipe) */
static int coresAtivas = 0;     /* terminal entende codigos ANSI         */
static int mapaNaTela = 0;      /* o ultimo desenho foi o mapa?          */
static int coluna = 0;          /* caracteres visiveis na linha atual    */

/* ------------------------------------------------------------------ */
/* Tela                                                                */
/* ------------------------------------------------------------------ */
void iniciarTela(void) {
    saidaInterativa = SAIDA_E_TERMINAL();
#ifdef _WIN32
    if (saidaInterativa) {
        /* Windows 10+ entende cores ANSI depois de ativar este modo. */
        HANDLE saida = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD modo;
        if (GetConsoleMode(saida, &modo) &&
            SetConsoleMode(saida, modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
            coresAtivas = 1;
    }
#else
    coresAtivas = saidaInterativa;
#endif
    mostrarCursor(0);
}

void finalizarTela(void) {
    usarCor(COR_NORMAL);
    mostrarCursor(1);
    fflush(stdout);
}

void usarCor(int cor) {
    if (coresAtivas) printf("\033[%dm", cor);
}

void mostrarCursor(int visivel) {
    if (!saidaInterativa) return;
#ifdef _WIN32
    {
        HANDLE saida = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        if (GetConsoleCursorInfo(saida, &info)) {
            info.bVisible = visivel ? TRUE : FALSE;
            SetConsoleCursorInfo(saida, &info);
        }
    }
#else
    fputs(visivel ? "\033[?25h" : "\033[?25l", stdout);
#endif
}

#ifdef _WIN32
/* Limpa pela API do console: funciona mesmo sem suporte a ANSI. */
static void limparConsoleWindows(void) {
    HANDLE saida = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD origem = {0, 0};
    DWORD escritos, total;
    if (!GetConsoleScreenBufferInfo(saida, &info)) return;
    total = (DWORD)info.dwSize.X * (DWORD)info.dwSize.Y;
    FillConsoleOutputCharacter(saida, ' ', total, origem, &escritos);
    FillConsoleOutputAttribute(saida, info.wAttributes, total, origem, &escritos);
    SetConsoleCursorPosition(saida, origem);
}
#endif

void limparTela(void) {
    mapaNaTela = 0;
    if (!saidaInterativa) {
        puts("\n----------------------------------------");
        return;
    }
    if (coresAtivas) {
        /* cursor no topo, apaga a tela e o historico de rolagem */
        fputs("\033[H\033[2J\033[3J", stdout);
    } else {
#ifdef _WIN32
        limparConsoleWindows();
#else
        fputs("\033[H\033[2J", stdout);
#endif
    }
    fflush(stdout);
}

/* Leva o cursor ao topo sem apagar nada: o quadro novo cobre o antigo. */
static void voltarAoTopo(void) {
    if (!saidaInterativa) {
        puts("\n----------------------------------------");
        return;
    }
#ifdef _WIN32
    if (!coresAtivas) {
        COORD origem = {0, 0};
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), origem);
        return;
    }
#endif
    fputs("\033[H", stdout);
}

/* printf que conta os caracteres visiveis da linha (para completar depois). */
static void escrever(const char *formato, ...) {
    char texto[256];
    va_list args;
    va_start(args, formato);
    vsprintf(texto, formato, args);
    va_end(args);
    fputs(texto, stdout);
    coluna += (int)strlen(texto);
}

static void escreverCaractere(char c) {
    putchar(c);
    coluna++;
}

static void terminarLinha(void) {
    usarCor(COR_NORMAL);
    while (coluna < LARGURA_TELA) escreverCaractere(' ');
    putchar('\n');
    coluna = 0;
}

/* ------------------------------------------------------------------ */
/* Desenho do mapa                                                     */
/* ------------------------------------------------------------------ */
static void desenharBarraSuspeita(int suspeita) {
    int i, cheias = suspeita / 5; /* 20 blocos */
    usarCor(suspeita < 40 ? COR_VERDE : suspeita < 70 ? COR_AMARELO : COR_VERMELHO);
    escreverCaractere('[');
    for (i = 0; i < 20; i++) escreverCaractere(i < cheias ? '#' : '-');
    escrever("] %3d%%", suspeita);
    usarCor(COR_NORMAL);
}

static void desenharCelula(char c, int portaAberta) {
    switch (c) {
    case PAREDE:          usarCor(COR_CINZA_CLARO); break;
    case CHAO:            usarCor(COR_CINZA); break;
    case SIMBOLO_VISAO:   usarCor(COR_AMARELO_ESCURO); break;
    case SIMBOLO_CHAVE:   usarCor(COR_AMARELO); break;
    case SIMBOLO_PORTA:   usarCor(portaAberta ? COR_VERDE : COR_VERMELHO); break;
    case SIMBOLO_GUARDA:  usarCor(COR_VERMELHO); break;
    case SIMBOLO_JOGADOR: usarCor(COR_CIANO); break;
    }
    escreverCaractere(c);
}

/* Quebra a mensagem em ate LINHAS_MENSAGEM linhas, sempre ocupando todas. */
static void desenharMensagem(const char *mensagem) {
    int linha, limite = LARGURA_TELA - 4;
    const char *resto = mensagem;
    for (linha = 0; linha < LINHAS_MENSAGEM; linha++) {
        int tamanho = (int)strlen(resto);
        escrever("  ");
        if (tamanho > limite) {
            tamanho = limite;
            while (tamanho > 0 && resto[tamanho] != ' ') tamanho--;
            if (tamanho == 0) tamanho = limite;
        }
        usarCor(COR_AMARELO);
        escrever("%.*s", tamanho, resto);
        terminarLinha();
        resto += tamanho;
        while (*resto == ' ') resto++;
    }
}

void exibirMapa(const Jogo *j) {
    char tela[LINHAS][COLUNAS];
    int x, y, i;

    /* Monta a imagem em camadas: chao/parede, visao, chaves, porta, guardas, jogador. */
    for (y = 0; y < LINHAS; y++)
        for (x = 0; x < COLUNAS; x++)
            tela[y][x] = (j->mapa[y][x] == CHAO && celulaVigiada(j, x, y)) ? SIMBOLO_VISAO : j->mapa[y][x];
    for (i = 0; i < j->quantidadeChaves; i++)
        if (!j->chaves[i].coletada) tela[j->chaves[i].y][j->chaves[i].x] = SIMBOLO_CHAVE;
    tela[j->porta.y][j->porta.x] = SIMBOLO_PORTA;
    for (i = 0; i < j->quantidadeGuardas; i++)
        if (j->guardas[i].ativo) tela[j->guardas[i].y][j->guardas[i].x] = SIMBOLO_GUARDA;
    tela[j->jogador.y][j->jogador.x] = SIMBOLO_JOGADOR;

    if (mapaNaTela) voltarAoTopo();
    else limparTela();
    coluna = 0;

    /* Painel */
    terminarLinha();
    usarCor(COR_BRANCO);
    escrever("  FUGA DA PRISAO");
    usarCor(COR_NORMAL);
    escrever("   Fase %d/%d: ", j->jogador.faseAtual, totalFases());
    usarCor(COR_CIANO);
    escrever("%s", nomeFase(j->jogador.faseAtual));
    terminarLinha();
    escrever("  Jogador: %-14.14s Pontos: %-7d Turnos: %d", j->jogador.nome, j->jogador.pontuacao, j->movimentos);
    terminarLinha();
    escrever("  Chaves: ");
    for (i = 0; i < j->quantidadeChaves; i++) {
        usarCor(j->chaves[i].coletada ? COR_AMARELO : COR_CINZA);
        escrever(j->chaves[i].coletada ? "[C]" : "[ ]");
    }
    usarCor(COR_NORMAL);
    escrever("   Suspeita: ");
    desenharBarraSuspeita(j->jogador.nivelSuspeita);
    terminarLinha();
    terminarLinha();

    /* Mapa */
    for (y = 0; y < LINHAS; y++) {
        escrever("  ");
        for (x = 0; x < COLUNAS; x++) desenharCelula(tela[y][x], j->porta.aberta);
        terminarLinha();
    }
    terminarLinha();

    /* Avisos do ultimo turno */
    desenharMensagem(j->mensagem);
    terminarLinha();

    /* Legenda */
    usarCor(COR_CINZA);
    escrever("  @ voce  G guarda  : visao do guarda  C chave  P porta (%s)",
             j->porta.aberta ? "aberta" : "trancada");
    terminarLinha();
    usarCor(COR_CINZA);
    escrever("  WASD/setas: mover   E: esperar   X: salvar e sair   Q: abandonar");
    terminarLinha();

    mapaNaTela = 1;
    fflush(stdout);
}

/* ------------------------------------------------------------------ */
/* Movimento                                                           */
/* ------------------------------------------------------------------ */
int moverJogador(Jogo *j, char comando) {
    int nx = j->jogador.x, ny = j->jogador.y, porta;

    switch (comando) {
    case 'w': case 'W': ny--; break;
    case 's': case 'S': ny++; break;
    case 'a': case 'A': nx--; break;
    case 'd': case 'D': nx++; break;
    case 'e': case 'E': j->movimentos++; return MOV_OK; /* esperar parado */
    default: return MOV_INVALIDO;
    }

    if (nx < 0 || nx >= COLUNAS || ny < 0 || ny >= LINHAS || j->mapa[ny][nx] == PAREDE)
        return MOV_PAREDE;

    porta = tentarSair(j, nx, ny);
    j->movimentos++;
    if (porta == PORTA_TRANCADA) return MOV_PORTA_TRANCADA; /* nao entra na porta */

    j->jogador.x = nx;
    j->jogador.y = ny;
    return porta == PORTA_ABERTA ? MOV_SAIU : MOV_OK;
}
