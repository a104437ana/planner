#include <stdio.h>
#include "view.h"
#include "datecalc.h"

static const char *MONTH_NAMES[12] = {
    "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE",
    "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"
};

/* Preenche os campos que toda a coluna precisa (para nunca ficarem por
 * inicializar). */
static void col_defaults(LISTCOL *c) {
    c->number_offset = 0;
    c->binary_color = 0;
}

/* A página do calendário é toda OBJETIVOS: 6 colunas, numeração contínua
 * 01..108, e o SPACEBAR só alterna vermelho (por fazer) / verde (feito) --
 * não o ciclo completo das 6 cores que o resto do planner usa. */
static int build_func0(STATE *st, LISTCOL cols[MAX_COLS], int hi[MAX_COLS]) {
    static const char *obj_header = "GOALS:";
    for (int i = 0; i < GOALS_COLS; i++) {
        col_defaults(&cols[i]);
        cols[i].header = obj_header;
        cols[i].items = &st->goals[i * GOALS_ROWS];
        cols[i].count = GOALS_ROWS;
        cols[i].number_offset = i * GOALS_ROWS;
        cols[i].top = LIST_TOP;
        cols[i].left = COL0 + PITCH * i;
        cols[i].header_gap = 1;
        cols[i].binary_color = 1;
        hi[i] = -1;
    }
    return GOALS_COLS;
}

/* func1 = Julho-Dezembro do ano1; func2 = Janeiro-Junho do ano2.
 * (No v1/v2 chamavam-se "presente"/"futuro", mas qual delas é de facto
 * passado ou futuro depende de em que metade do ano letivo se está --
 * por isso aqui ficam identificadas sempre pelos mesmos 6 meses fixos.) */
static int build_events(STATE *st, LISTCOL cols[MAX_COLS], int hi[MAX_COLS], int month0_start) {
    static char headers[MAX_COLS][24];
    int today_month0 = st->today.tm_mon;
    int today_year = st->today.tm_year + 1900;
    int today_day = st->today.tm_mday;

    for (int i = 0; i < 6; i++) {
        col_defaults(&cols[i]);
        int month0 = month0_start + i;
        int year = year_for_month0(month0, st->ano1, st->ano2);
        /* o v1 repetia o título do mês/ano aqui, em cima da lista, além
         * do título já desenhado dentro da caixa decorativa por cima */
        snprintf(headers[i], sizeof(headers[i]), "%02d %s %d", month0 + 1, MONTH_NAMES[month0], year);
        cols[i].header = headers[i];
        cols[i].items = st->day[month0];
        cols[i].count = days_in_month(month0 + 1, year);
        cols[i].top = LIST_TOP_EVENTS;
        cols[i].left = COL0 + PITCH * i;
        cols[i].header_gap = 1; /* o v1 deixava uma linha em branco aqui */
        hi[i] = (month0 == today_month0 && year == today_year) ? today_day - 1 : -1;
    }
    return 6;
}

/* Tal como o v1 (draw_daily_func): caixa decorativa do mês atual e do mês
 * seguinte (desenhadas por draw_today_grid em draw.c), cada uma com a sua
 * lista de dias por baixo -- e depois 3 colunas de HABITOS DIARIOS + 1 de
 * TAREFAS, a usar o máximo de linhas que cabe no espaço já disponível
 * (começam em LIST_TOP_SIDE, bem mais acima, por não terem caixa em cima). */
static int build_func3(STATE *st, LISTCOL cols[MAX_COLS], int hi[MAX_COLS]) {
    static char headers[2][24];
    int today_month0 = st->today.tm_mon;
    int today_year = st->today.tm_year + 1900;
    int today_day = st->today.tm_mday;

    int month0[2], year[2];
    month0[0] = today_month0;
    year[0] = today_year;
    if (today_month0 == 11) { month0[1] = 0; year[1] = today_year + 1; }
    else { month0[1] = today_month0 + 1; year[1] = today_year; }

    for (int i = 0; i < 2; i++) {
        col_defaults(&cols[i]);
        snprintf(headers[i], sizeof(headers[i]), "%02d %s %d", month0[i] + 1, MONTH_NAMES[month0[i]], year[i]);
        cols[i].header = headers[i];
        cols[i].items = st->day[month0[i]];
        cols[i].count = days_in_month(month0[i] + 1, year[i]);
        cols[i].top = LIST_TOP_EVENTS;
        cols[i].left = COL0 + PITCH * i;
        cols[i].header_gap = 1;
        hi[i] = (month0[i] == today_month0) ? today_day - 1 : -1;
    }

    /* As 3 colunas são independentes entre si (cada uma pensada para um
     * tipo de dia diferente), por isso contam todas 01..40 à parte, em vez
     * de continuarem a numeração de uma coluna para a outra. */
    for (int i = 0; i < HABITS_COLS; i++) {
        int c = 2 + i;
        col_defaults(&cols[c]);
        cols[c].header = "DAILY HABITS:";
        cols[c].items = &st->habits[i * HABITS_ROWS];
        cols[c].count = HABITS_ROWS;
        cols[c].top = LIST_TOP_SIDE;
        cols[c].left = COL0 + PITCH * c;
        cols[c].header_gap = 1;
        cols[c].binary_color = 1; /* vermelho (por fazer) / verde (feito) */
        hi[c] = -1;
    }

    int t = 2 + HABITS_COLS;
    col_defaults(&cols[t]);
    cols[t].header = "TASKS:";
    cols[t].items = st->todos;
    cols[t].count = TODOS_N;
    cols[t].top = LIST_TOP_SIDE;
    cols[t].left = COL0 + PITCH * t;
    cols[t].header_gap = 1;
    cols[t].binary_color = 1;
    hi[t] = -1;

    return t + 1;
}

int view_build(STATE *st, LISTCOL cols[MAX_COLS], int hi[MAX_COLS]) {
    switch (st->func) {
        case 0: return build_func0(st, cols, hi);
        case 1: return build_events(st, cols, hi, 6);  /* Jul (mes0=6) .. Dez */
        case 2: return build_events(st, cols, hi, 0);  /* Jan (mes0=0) .. Jun */
        case 3: return build_func3(st, cols, hi);
    }
    return 0;
}
