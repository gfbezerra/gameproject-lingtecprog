#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "jogo.h"
#include "jogador.h"
#include "fase.h"
#include "mapa.h"
#include "guardas.h"
#include "arquivos.h"

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#define ENTRADA_E_TERMINAL() _isatty(_fileno(stdin))
#else
#include <termios.h>
#include <unistd.h>
#define ENTRADA_E_TERMINAL() isatty(STDIN_FILENO)
#endif

#define TECLA_FIM (-1) /* a entrada acabou (arquivo/pipe) */

/* ================================================================== */
/* Entrada do teclado                                                  */
/* ================================================================== */

/* Le UMA tecla sem precisar de Enter. Setas viram w/a/s/d. */
static int lerTecla(void) {
    int c;
    fflush(stdout);

    if (!ENTRADA_E_TERMINAL()) {
        /* Entrada redirecionada (testes): uma linha por tecla. */
        char linha[64];
        if (!fgets(linha, sizeof linha, stdin)) return TECLA_FIM;
        return tolower((unsigned char)linha[0]);
    }
#ifdef _WIN32
    c = _getch();
    if (c == 0 || c == 224) { /* teclas especiais */
        switch (_getch()) {
        case 72: return 'w';
        case 80: return 's';
        case 75: return 'a';
        case 77: return 'd';
        default: return 0;
        }
    }
#else
    {
        struct termios antigo, novo;
        tcgetattr(STDIN_FILENO, &antigo);
        novo = antigo;
        novo.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &novo);
        c = getchar();
        if (c == 27 && getchar() == '[') { /* setas: ESC [ A/B/C/D */
            switch (getchar()) {
            case 'A': c = 'w'; break;
            case 'B': c = 's'; break;
            case 'D': c = 'a'; break;
            case 'C': c = 'd'; break;
            default: c = 0;
            }
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &antigo);
        if (c == EOF) return TECLA_FIM;
    }
#endif
    return tolower(c);
}

static void esperarTecla(void) {
    usarCor(COR_CINZA);
    printf("\n  Pressione qualquer tecla para continuar...");
    usarCor(COR_NORMAL);
    lerTecla();
}

static void lerTexto(const char *rotulo, char *destino, int tamanho) {
    printf("%s", rotulo);
    mostrarCursor(1);
    fflush(stdout);
    if (!fgets(destino, tamanho, stdin)) destino[0] = '\0';
    destino[strcspn(destino, "\r\n")] = '\0';
    mostrarCursor(0);
}

/* Acrescenta um aviso que aparece abaixo do mapa no proximo desenho. */
static void avisar(Jogo *j, const char *texto) {
    size_t usado = strlen(j->mensagem);
    if (usado > 0 && usado + 1 < sizeof j->mensagem) j->mensagem[usado++] = ' ';
    strncpy(j->mensagem + usado, texto, sizeof j->mensagem - usado - 1);
    j->mensagem[sizeof j->mensagem - 1] = '\0';
}

/* ================================================================== */
/* Telas                                                               */
/* ================================================================== */
static void desenharTitulo(void) {
    limparTela();
    usarCor(COR_CINZA_CLARO);
    puts("");
    puts("   +==========================================+");
    puts("   |  ||  ||  ||  ||  ||  ||  ||  ||  ||  ||  |");
    printf("   |       ");
    usarCor(COR_AMARELO);
    printf("F U G A   D A   P R I S A O");
    usarCor(COR_CINZA_CLARO);
    puts("        |");
    puts("   |  ||  ||  ||  ||  ||  ||  ||  ||  ||  ||  |");
    puts("   +==========================================+");
    usarCor(COR_NORMAL);
}

static void instrucoes(void) {
    desenharTitulo();
    puts("\n  OBJETIVO");
    printf("    Pegue todas as chaves (C) e saia pela porta (P). Sao %d fases.\n", totalFases());
    puts("\n  CONTROLES");
    puts("    W A S D ou setas  mover            E  esperar um turno");
    puts("    X  salvar e voltar ao menu         Q  abandonar a partida");
    puts("\n  GUARDAS");
    puts("    Os guardas (G) patrulham indo e voltando. Os ':' mostram o que");
    printf("    cada um enxerga: %d casas a frente; paredes bloqueiam a visao.\n", ALCANCE_VISAO);
    printf("    Ser visto: +%d de suspeita. Cada turno escondido: -%d.\n", SUSPEITA_VISTO, SUSPEITA_ALIVIO);
    puts("    Suspeita em 100% ou encostar num guarda: voce e capturado.");
    puts("\n  PONTUACAO");
    printf("    Chave +%d   Fase +%d   Rapidez ate +%d (-%d por turno)\n",
           PONTOS_CHAVE, PONTOS_FASE, BONUS_RAPIDEZ_MAX, CUSTO_MOVIMENTO);
    printf("    Porta trancada: -%d pontos e +%d de suspeita.\n", PENALIDADE_PORTA, SUSPEITA_PORTA);
    printf("    Ao escapar: +%d por ponto de suspeita abaixo de 100.\n", BONUS_DISCRICAO);
    esperarTecla();
}

static void telaInicioFase(const Jogo *j) {
    limparTela();
    usarCor(COR_CIANO);
    printf("\n\n      FASE %d de %d\n", j->jogador.faseAtual, totalFases());
    usarCor(COR_BRANCO);
    printf("      %s\n\n", nomeFase(j->jogador.faseAtual));
    usarCor(COR_NORMAL);
    printf("      Chaves: %d   Guardas: %d\n\n", j->quantidadeChaves, j->quantidadeGuardas);
    usarCor(COR_AMARELO);
    printf("      Dica: %s\n", dicaFase(j->jogador.faseAtual));
    usarCor(COR_NORMAL);
    esperarTecla();
}

static void telaCaptura(Jogo *j) {
    int posicao;
    exibirMapa(j);
    usarCor(COR_VERMELHO);
    puts("\n  ============ VOCE FOI CAPTURADO ============");
    usarCor(COR_NORMAL);
    printf("  Pontuacao final: %d (fase %d)\n", j->jogador.pontuacao, j->jogador.faseAtual);
    posicao = registrarRecorde(j, ARQUIVO_RECORDES);
    printf("  Posicao no ranking: %d\n", posicao);
    remove(ARQUIVO_SAVE);
    esperarTecla();
}

/* Pontua a fase. Retorna 1 se era a ultima (o jogador escapou). */
static int concluirFase(Jogo *j) {
    int bonus = BONUS_RAPIDEZ_MAX - j->movimentos * CUSTO_MOVIMENTO;
    if (bonus < 0) bonus = 0;
    j->jogador.pontuacao += PONTOS_FASE + bonus;

    limparTela();
    usarCor(COR_VERDE);
    printf("\n  FASE %d CONCLUIDA: %s\n\n", j->jogador.faseAtual, nomeFase(j->jogador.faseAtual));
    usarCor(COR_NORMAL);
    printf("  Fase concluida ......... +%d\n", PONTOS_FASE);
    printf("  Bonus de rapidez ....... +%d  (%d turnos)\n", bonus, j->movimentos);

    if (j->jogador.faseAtual == totalFases()) {
        int discricao = (100 - j->jogador.nivelSuspeita) * BONUS_DISCRICAO, posicao;
        j->jogador.pontuacao += discricao;
        printf("  Bonus de discricao ..... +%d  (suspeita %d%%)\n", discricao, j->jogador.nivelSuspeita);
        usarCor(COR_AMARELO);
        puts("\n  *************************************************");
        puts("  *      VOCE ESCAPOU DA PRISAO!  LIBERDADE!      *");
        puts("  *************************************************");
        usarCor(COR_NORMAL);
        printf("\n  Pontuacao final: %d\n", j->jogador.pontuacao);
        posicao = registrarRecorde(j, ARQUIVO_RECORDES);
        if (posicao == 1) {
            usarCor(COR_AMARELO);
            puts("  NOVO RECORDE!");
            usarCor(COR_NORMAL);
        } else {
            printf("  Posicao no ranking: %d\n", posicao);
        }
        remove(ARQUIVO_SAVE);
        esperarTecla();
        return 1;
    }

    j->jogador.nivelSuspeita /= 2; /* a tensao diminui ao mudar de area */
    printf("  Pontuacao total ........ %d\n", j->jogador.pontuacao);
    printf("  Suspeita caiu para ..... %d%%\n", j->jogador.nivelSuspeita);
    esperarTecla();

    configurarFase(j, j->jogador.faseAtual + 1);
    salvarJogo(j, ARQUIVO_SAVE); /* salvamento automatico no inicio da fase */
    telaInicioFase(j);
    avisar(j, "Progresso salvo automaticamente.");
    return 0;
}

/* ================================================================== */
/* Partida                                                             */
/* ================================================================== */

/* Executa um turno. Retorna 1 se a partida acabou (vitoria ou captura). */
static int processarTurno(Jogo *j, char tecla) {
    char texto[TAM_MENSAGEM];
    int vistoAntes, resultado = moverJogador(j, tecla);

    switch (resultado) {
    case MOV_INVALIDO:
        avisar(j, "Tecla invalida. Use W A S D, setas, E, X ou Q.");
        return 0;
    case MOV_PAREDE:
        avisar(j, "Uma parede bloqueia o caminho.");
        return 0; /* nao gasta turno */
    case MOV_SAIU:
        return concluirFase(j);
    case MOV_PORTA_TRANCADA:
        sprintf(texto, "Porta trancada! Faltam %d chave(s). -%d pontos, +%d suspeita.",
                j->quantidadeChaves - chavesColetadas(j), PENALIDADE_PORTA, SUSPEITA_PORTA);
        avisar(j, texto);
        break;
    }

    if (guardaNaPosicao(j, j->jogador.x, j->jogador.y)) {
        j->jogador.capturado = 1;
        avisar(j, "Voce trombou com um guarda!");
    } else {
        if (coletarChave(j)) {
            sprintf(texto, "Chave coletada! +%d.", PONTOS_CHAVE);
            avisar(j, texto);
            if (j->porta.aberta) avisar(j, "A porta foi destrancada!");
        }

        /* Ser visto antes OU depois de o guarda andar conta uma vez so. */
        vistoAntes = guardaVeJogador(j);
        if (atualizarGuardas(j)) {
            avisar(j, "Um guarda te pegou!");
        } else if (vistoAntes || guardaVeJogador(j)) {
            sprintf(texto, "Um guarda te viu! Suspeita +%d.", SUSPEITA_VISTO);
            avisar(j, texto);
            atualizarSuspeita(j, SUSPEITA_VISTO);
            if (j->jogador.capturado) avisar(j, "A suspeita chegou a 100%: o alarme tocou!");
        } else {
            atualizarSuspeita(j, -SUSPEITA_ALIVIO);
        }
    }

    if (j->jogador.capturado) {
        telaCaptura(j);
        return 1;
    }
    return 0;
}

static void jogar(Jogo *j) {
    for (;;) {
        int tecla;
        exibirMapa(j);
        j->mensagem[0] = '\0';
        tecla = lerTecla();

        if (tecla == TECLA_FIM) return;
        if (tecla == 'x') {
            limparTela();
            if (salvarJogo(j, ARQUIVO_SAVE)) puts("\n  Partida salva. Use \"Continuar\" no menu para voltar.");
            else puts("\n  Nao foi possivel salvar a partida.");
            esperarTecla();
            return;
        }
        if (tecla == 'q') {
            avisar(j, "Abandonar a partida? O que nao foi salvo sera perdido. (S/N)");
            exibirMapa(j);
            j->mensagem[0] = '\0';
            if (lerTecla() == 's') return;
            continue;
        }
        if (processarTurno(j, (char)tecla)) return;
    }
}

static void novaPartida(Jogo *j) {
    char nome[TAM_NOME];
    desenharTitulo();
    lerTexto("\n  Nome do jogador: ", nome, sizeof nome);
    if (!nome[0]) strcpy(nome, "Jogador");

    memset(j, 0, sizeof *j);
    iniciarJogador(j, nome);
    configurarFase(j, 1);
    telaInicioFase(j);
    jogar(j);
}

void executarJogo(void) {
    Jogo jogo;
    int opcao;

    iniciarTela();
    if (!validarFases()) {
        puts("\nCorrija o desenho das fases em fase.c antes de jogar.");
        finalizarTela();
        return;
    }

    do {
        int temSave = existeSave(ARQUIVO_SAVE);
        desenharTitulo();
        puts("\n     1. Novo jogo");
        if (!temSave) usarCor(COR_CINZA);
        puts("     2. Continuar jogo salvo");
        usarCor(COR_NORMAL);
        puts("     3. Recordes");
        puts("     4. Como jogar");
        puts("     5. Sair");
        printf("\n  Escolha uma opcao: ");
        opcao = lerTecla();

        switch (opcao) {
        case '1':
            novaPartida(&jogo);
            break;
        case '2':
            if (carregarJogo(&jogo, ARQUIVO_SAVE)) {
                avisar(&jogo, "Partida carregada.");
                jogar(&jogo);
            } else {
                puts("\n\n  Nenhum jogo salvo valido foi encontrado.");
                esperarTecla();
            }
            break;
        case '3':
            exibirRecordes(ARQUIVO_RECORDES);
            esperarTecla();
            break;
        case '4':
            instrucoes();
            break;
        case '5':
        case TECLA_FIM:
            opcao = '5';
            break;
        }
    } while (opcao != '5');

    limparTela();
    puts("Ate a proxima!");
    finalizarTela();
}
