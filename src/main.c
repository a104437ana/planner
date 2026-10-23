#include <stdio.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <ncurses.h>

#include "state.h"
#include "datecalc.h"
#include "storage.h"
#include "draw.h"
#include "input.h"
#include "view.h"

static void init_state(STATE *st) {
    memset(st, 0, sizeof(*st));

    /* Objetivos, hábitos e tarefas começam a vermelho (por fazer) por
     * omissão -- só alternam entre vermelho e verde, ao contrário dos
     * eventos/agenda, que começam sem cor e percorrem as 6 cores todas.
     * storage_load() substitui isto pelos valores guardados, se já existir
     * um ficheiro. */
    for (int i = 0; i < GOALS_N; i++) st->goals[i].color = 1;
    for (int i = 0; i < HABITS_N; i++) st->habits[i].color = 1;
    for (int i = 0; i < TODOS_N; i++) st->todos[i].color = 1;

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    st->today = *tm;

    academic_year(st->today.tm_mon + 1, st->today.tm_year + 1900, &st->ano1, &st->ano2);

    st->func = 0;
    st->cursor.row = 0;
    st->cursor.col = 0;

    storage_load(st);
}

static void refresh_clock(STATE *st) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    st->today = *tm;
}

static int size_ok(STATE *st) {
    return st->term_rows >= MIN_ROWS && st->term_cols >= MIN_COLS;
}

/* Mensagem simples, impressa no terminal normal (fora do ncurses), em vez
 * de desenhada por cima do ecrã -- fica visível no historico do terminal
 * mesmo depois do programa sair, sem ser preciso carregar numa tecla. */
static void print_too_small(int rows, int cols) {
    printf("The terminal window is too small.\n");
    printf("Current size: %d rows x %d columns.\n", rows, cols);
    printf("Needs at least %d rows x %d columns.\n\n", MIN_ROWS, MIN_COLS);
    printf("Enlarge the window, then run the planner again.\n");
}

int main(void) {
    setlocale(LC_ALL, "");

    if (initscr() == NULL) {
        fprintf(stderr, "Error starting ncurses.\n");
        return 1;
    }
    if (!has_colors()) {
        endwin();
        fprintf(stderr, "This terminal does not support colors.\n");
        return 1;
    }
    start_color();
    draw_init_colors();

    cbreak();
    noecho();
    nonl();
    keypad(stdscr, TRUE);
    curs_set(1);
    /* getch() bloqueia até haver uma tecla (ou um SIGWINCH de resize) --
     * zero CPU gasto parado, e sem andar a limpar e redesenhar o ecrã
     * sozinho de x em x tempo (o que piscava). As horas na barra só
     * atualizam quando se carrega numa tecla, tal como no v1/v2. */

    STATE st;
    init_state(&st);

    while (1) {
        getmaxyx(stdscr, st.term_rows, st.term_cols);

        if (!size_ok(&st)) {
            /* Em vez de tentar esperar que a janela volte a crescer e
             * retomar a meio (o que deixava o cursor invisível e a navegação
             * com mazelas), sai do ncurses e mostra o aviso no terminal
             * normal -- é só correr `./planner` outra vez depois de
             * aumentar a janela. Aplica-se tanto ao arranque como a
             * encolher a meio do programa (ambos passam por aqui). */
            endwin();
            print_too_small(st.term_rows, st.term_cols);
            return 0;
        }

        refresh_clock(&st);
        draw_view(&st);
        input_position_cursor(&st);
        refresh();

        int key = getch();
        if (key == ERR) continue;      /* timeout: só atualiza o relógio */
        if (key == KEY_RESIZE) continue;

        if (input_handle(&st, key) == -1) break;
    }

    endwin();
    return 0;
}
