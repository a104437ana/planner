#ifndef PLANNER_STATE_H
#define PLANNER_STATE_H

#include <time.h>

/* Um item editável (objetivo, hábito, tarefa, bloco de agenda ou nota de um dia).
 * cor: 0 = sem cor (omissão), 1..5 = vermelho/verde/amarelo/ciano/azul.
 * Todos os tipos de item partilham o mesmo limite de texto e o mesmo ciclo de
 * cores (0..5), ao contrário do v1 onde objetivos/tarefas/horas só percorriam
 * 2 cores por um copy-paste mal feito no switch de mudar_cor(). */
#define ITEM_LEN 27

typedef struct {
    int color;
    char text[ITEM_LEN];
} ITEM;

/* A página do calendário (func0) é toda ela OBJETIVOS: 6 colunas, uma por
 * cada coluna de meses da grelha decorativa por cima. */
#define GOALS_COLS 6
#define GOALS_ROWS 18
#define GOALS_N (GOALS_COLS * GOALS_ROWS)

/* Vista HOJE: 3 colunas de hábitos diários, usando o máximo de linhas que
 * cabe no espaço já disponível nessa página (sem precisar de um terminal
 * maior do que o resto do planner já exige). */
#define HABITS_COLS 3
#define HABITS_ROWS 40
#define HABITS_N (HABITS_COLS * HABITS_ROWS)

#define TODOS_N 40

typedef struct {
    int row;
    int col;
} CURSOR;

/* Uma coluna de lista navegável: N itens editáveis com um cabeçalho e uma
 * posição no ecrã. É o bloco reutilizável que desenha e edita objetivos,
 * hábitos, notas, eventos de um mês, tarefas e a agenda -- em vez do v1,
 * que tinha uma cópia quase idêntica deste código espalhada por cada
 * funcionalidade. */
typedef struct {
    const char *header;
    ITEM *items;
    int count;
    int number_offset; /* números mostrados começam em number_offset+1 */
    int top;
    int left;
    int header_gap; /* linhas em branco entre o cabeçalho e o 1º item (0 ou 1) */
    int binary_color; /* 1 = SPACEBAR só alterna vermelho/verde (objetivos) */
} LISTCOL;

typedef struct {
    int func;                 /* 0..3: vista atual */
    CURSOR cursor;             /* posição lógica (coluna, linha) na vista atual */

    struct tm today;

    int ano1, ano2;            /* ano letivo: ano1 = ano de Jul-Dez, ano2 = ano1+1 (Jan-Jun) */

    ITEM day[12][31];          /* notas por dia, indexadas pelo mês real 0-11 */
    ITEM goals[GOALS_N];
    ITEM habits[HABITS_N];     /* reutilizado como "hábitos diários" na vista HOJE */
    ITEM todos[TODOS_N];

    int last_reset_day, last_reset_mon, last_reset_year;

    int term_rows, term_cols;
} STATE;

#endif
