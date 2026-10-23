#include <ncurses.h>
#include <string.h>
#include "draw.h"
#include "datecalc.h"

#define PAIR_WHITE   1
#define PAIR_RED     2
#define PAIR_GREEN   3
#define PAIR_YELLOW  4
#define PAIR_CYAN    5
#define PAIR_BLUE    6
#define PAIR_MAGENTA 7

static const char *MONTH_NAMES[12] = {
    "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE",
    "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"
};
static const char *WEEKDAY_NAMES[8] = {
    "", "MONDAY", "TUESDAY", "WEDNESDAY",
    "THURSDAY", "FRIDAY", "SATURDAY", "SUNDAY"
};

void draw_init_colors(void) {
    /* Preto explícito (tal como o v1/v2), não o "default" do terminal (-1):
     * o -1 depende de como cada emulador interpreta "sem cor de fundo", que
     * nem sempre coincide com a cor 0 da palete (o azul-escuro habitual). */
    init_pair(PAIR_WHITE,   COLOR_WHITE,   COLOR_BLACK);
    init_pair(PAIR_RED,     COLOR_RED,     COLOR_BLACK);
    init_pair(PAIR_GREEN,   COLOR_GREEN,   COLOR_BLACK);
    init_pair(PAIR_YELLOW,  COLOR_YELLOW,  COLOR_BLACK);
    init_pair(PAIR_CYAN,    COLOR_CYAN,    COLOR_BLACK);
    init_pair(PAIR_BLUE,    COLOR_BLUE,    COLOR_BLACK);
    init_pair(PAIR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
}

int color_pair_for(int color) {
    switch (color) {
        case 1: return PAIR_RED;
        case 2: return PAIR_GREEN;
        case 3: return PAIR_YELLOW;
        case 4: return PAIR_CYAN;
        case 5: return PAIR_BLUE;
        default: return PAIR_WHITE;
    }
}

static void put(int pair, const char *s) {
    attron(COLOR_PAIR(pair));
    printw("%s", s);
    attroff(COLOR_PAIR(pair));
}

/* Caracteres de desenho de caixa como texto UTF-8 literal, em vez das
 * constantes ACS_* do ncurses (que passam pela tabela "alternate character
 * set" do terminfo do terminal -- em alguns terminais essa tradução falha
 * silenciosamente para certos caracteres, como as linhas horizontais,
 * mesmo quando os verticais aparecem bem). Como o terminal já mostra
 * acentos portugueses sem problema, os literais UTF-8 são mais fiáveis. */
#define BX_H  "─" /* ─ */
#define BX_V  "│" /* │ */
#define BX_TL "┌" /* ┌ */
#define BX_TR "┐" /* ┐ */
#define BX_BL "└" /* └ */
#define BX_BR "┘" /* ┘ */
#define BX_LT "├" /* ├ */
#define BX_RT "┤" /* ┤ */
#define BX_TT "┬" /* ┬ */
#define BX_BT "┴" /* ┴ */
#define BX_PL "┼" /* ┼ */

static void hline_run(int row, int col_start, int col_end) {
    move(row, col_start);
    for (int i = col_start; i < col_end; i++) printw("%s", BX_H);
}

static void draw_rectangle(int x1, int y1, int x2, int y2) {
    attron(COLOR_PAIR(PAIR_WHITE));
    mvprintw(x1, y1, "%s", BX_TL);
    hline_run(x1, y1 + 1, y2);
    mvprintw(x1, y2, "%s", BX_TR);
    for (int i = x1 + 1; i < x2; i++) mvprintw(i, y2, "%s", BX_V);
    mvprintw(x2, y2, "%s", BX_BR);
    hline_run(x2, y1 + 1, y2);
    mvprintw(x2, y1, "%s", BX_BL);
    for (int i = x2 - 1; i > x1; i--) mvprintw(i, y1, "%s", BX_V);
    attroff(COLOR_PAIR(PAIR_WHITE));
}

static const char *func_name(int func) {
    switch (func) {
        case 0: return "CALENDAR AND GOALS";
        case 1: return "EVENTS: JULY TO DECEMBER";
        case 2: return "EVENTS: JANUARY TO JUNE";
        case 3: return "TODAY";
    }
    return "";
}

static void draw_top_bar(STATE *st) {
    char buf[64];
    move(0, 0);
    put(PAIR_GREEN, " | DATE : ");
    snprintf(buf, sizeof(buf), "%d %s (%02d) %d", st->today.tm_mday, MONTH_NAMES[st->today.tm_mon],
              st->today.tm_mon + 1, st->today.tm_year + 1900);
    put(PAIR_YELLOW, buf);
    put(PAIR_GREEN, " | TIME : ");
    snprintf(buf, sizeof(buf), "%02dh%02dmin", st->today.tm_hour, st->today.tm_min);
    put(PAIR_YELLOW, buf);
    put(PAIR_GREEN, " | DAY OF THE WEEK : ");
    put(PAIR_YELLOW, WEEKDAY_NAMES[st->today.tm_wday == 0 ? 7 : st->today.tm_wday]);
    put(PAIR_GREEN, " | VIEW : ");
    put(PAIR_YELLOW, func_name(st->func));
    put(PAIR_GREEN, " | MADE BY : ");
    put(PAIR_YELLOW, "Ana S.O.");
    put(PAIR_GREEN, " |");
    clrtoeol();
}

static void draw_bottom_bar(STATE *st) {
    move(st->term_rows - 1, 0);
    put(PAIR_GREEN, " | QUIT : ");
    put(PAIR_YELLOW, "Q");
    put(PAIR_GREEN, " | SWITCH VIEW : ");
    put(PAIR_YELLOW, "A / S");
    put(PAIR_GREEN, " | MOVE : ");
    put(PAIR_YELLOW, "Arrows");
    put(PAIR_GREEN, " | EDIT / STOP EDITING : ");
    put(PAIR_YELLOW, "ENTER");
    put(PAIR_GREEN, " | DELETE CHARACTER : ");
    put(PAIR_YELLOW, "DELETE");
    put(PAIR_GREEN, " | CLEAR TEXT : ");
    put(PAIR_YELLOW, "CTRL+U");
    put(PAIR_GREEN, " | CHANGE COLOR : ");
    put(PAIR_YELLOW, "SPACEBAR");
    put(PAIR_GREEN, " |");
    clrtoeol();
}

static void draw_month_box(STATE *st, int top, int left, int month0, int year) {
    int month1 = month0 + 1;
    int x1 = top, x2 = top + BLOCK_H - 1;
    int y1 = left, y2 = left + 21;

    attron(COLOR_PAIR(PAIR_WHITE));
    mvprintw(x1, y1, "%s", BX_TL);
    move(x1, y1 + 1);
    for (int i = y1 + 1; i < y2; i++) printw("C");
    mvprintw(x1, y2, "%s", BX_TR);

    for (int i = x1 + 1; i < x2; i++) {
        const char *ch = (i == x1 + 2 || i == x1 + 4) ? BX_RT : BX_V;
        mvprintw(i, y2, "%s", ch);
        const char *chl = (i == x1 + 2 || i == x1 + 4) ? BX_LT : BX_V;
        mvprintw(i, y1, "%s", chl);
    }

    mvprintw(x2, y1, "%s", BX_BL);
    hline_run(x2, y1 + 1, y2);
    mvprintw(x2, y2, "%s", BX_BR);

    hline_run(x1 + 2, y1 + 1, y2);
    hline_run(x1 + 4, y1 + 1, y2);
    for (int k = 1; k <= 6; k++) {
        int col = y2 - 3 * k;
        mvprintw(x1 + 2, col, "%s", BX_TT);
        mvprintw(x1 + 3, col, "%s", BX_V);  /* separador entre "SE","TE","QU"... */
        mvprintw(x1 + 4, col, "%s", BX_PL);
        mvprintw(x2, col, "%s", BX_BT);
        for (int r = x1 + 5; r < x2; r++) mvprintw(r, col, "%s", BX_V);
    }

    move(x1 + 1, y1 + 1);
    char title[24];
    snprintf(title, sizeof(title), "%02d %s %d", month1, MONTH_NAMES[month1 - 1], year);
    int pad = (20 - (int)strlen(title)) / 2;
    for (int i = 0; i < pad; i++) addch(' ');
    printw("%s", title);

    const char *wd[7] = {"MO","TU","WE","TH","FR","SA","SU"};
    for (int i = 0; i < 7; i++) {
        move(x1 + 3, y1 + 1 + 3 * i);
        printw("%s", wd[i]);
    }
    attroff(COLOR_PAIR(PAIR_WHITE));

    int today_month1 = st->today.tm_mon + 1;
    int today_year = st->today.tm_year + 1900;
    int today_day = st->today.tm_mday;

    int dow1 = day_of_week(1, month1, year); /* 1=Mon..7=Sun */
    int row = 0, col = dow1 - 1;
    int n = days_in_month(month1, year);
    for (int d = 1; d <= n; d++) {
        int is_today = (month1 == today_month1 && year == today_year && d == today_day);
        /* o dia de hoje tem sempre prioridade sobre a cor do evento,
         * tal como no v1 (draw_dia_atual_calend é desenhado por cima) */
        int pair = is_today ? PAIR_MAGENTA : color_pair_for(st->day[month0][d - 1].color);
        attron(COLOR_PAIR(pair));
        move(x1 + 5 + row, y1 + 1 + 3 * col);
        printw("%02d", d);
        attroff(COLOR_PAIR(pair));
        col++;
        if (col > 6) { col = 0; row++; }
    }
}

static void draw_calendar_grid(STATE *st) {
    for (int i = 0; i < 6; i++) {
        int month0 = i + 6; /* Jul..Dez */
        draw_month_box(st, GRID_TOP, COL0 + PITCH * i, month0, st->ano1);
    }
    for (int i = 0; i < 6; i++) {
        int month0 = i; /* Jan..Jun */
        draw_month_box(st, GRID_TOP + BLOCK_H + GRID_GAP, COL0 + PITCH * i, month0, st->ano2);
    }
}

static void draw_events_grid(STATE *st, int month0_start) {
    for (int i = 0; i < 6; i++) {
        int month0 = month0_start + i;
        int year = year_for_month0(month0, st->ano1, st->ano2);
        draw_month_box(st, GRID_TOP, COL0 + PITCH * i, month0, year);
    }
}

/* Vista HOJE: caixa do mês atual e do mês seguinte, tal como o
 * draw_daily_func() do v1. */
static void draw_today_grid(STATE *st) {
    int month0 = st->today.tm_mon;
    int year = st->today.tm_year + 1900;
    draw_month_box(st, GRID_TOP, COL0, month0, year);

    int next_month0 = (month0 == 11) ? 0 : month0 + 1;
    int next_year = (month0 == 11) ? year + 1 : year;
    draw_month_box(st, GRID_TOP, COL0 + PITCH, next_month0, next_year);
}

static void draw_listcol(LISTCOL *col, int highlight_row) {
    move(col->top, col->left);
    attron(COLOR_PAIR(PAIR_WHITE));
    printw("%s", col->header);
    attroff(COLOR_PAIR(PAIR_WHITE));

    for (int i = 0; i < col->count; i++) {
        move(col->top + 1 + col->header_gap + i, col->left);
        int label_pair = (i == highlight_row) ? PAIR_MAGENTA : PAIR_WHITE;
        attron(COLOR_PAIR(label_pair));
        printw("%02d. ", col->number_offset + i + 1);
        attroff(COLOR_PAIR(label_pair));

        int pair = color_pair_for(col->items[i].color);
        attron(COLOR_PAIR(pair));
        printw("%s", col->items[i].text);
        attroff(COLOR_PAIR(pair));
        clrtoeol();
    }
}

void draw_view(STATE *st) {
    clear();
    draw_top_bar(st);

    if (st->func == 0) draw_calendar_grid(st);
    if (st->func == 1) draw_events_grid(st, 6);  /* Jul..Dez */
    if (st->func == 2) draw_events_grid(st, 0);  /* Jan..Jun */
    if (st->func == 3) draw_today_grid(st);

    LISTCOL cols[MAX_COLS];
    int hi[MAX_COLS];
    int n = view_build(st, cols, hi);
    for (int i = 0; i < n; i++) draw_listcol(&cols[i], hi[i]);

    draw_bottom_bar(st);
    /* a caixa fica entre as duas barras (tal como o v1: draw_retangulo nas
     * linhas 1..53, barra de baixo na linha 54, uma a seguir à caixa) --
     * não deve englobar a barra de baixo. */
    draw_rectangle(1, 0, st->term_rows - 2, st->term_cols - 1);
}

void draw_func_label(int func) {
    (void) func;
}

void draw_clock(STATE *st) {
    (void) st;
}
