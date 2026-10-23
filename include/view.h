#ifndef PLANNER_VIEW_H
#define PLANNER_VIEW_H

#include "state.h"

#define MAX_COLS 6

/* Largura do rótulo "NN. " antes do texto de cada item da lista (2 dígitos
 * + ponto + espaço). Partilhada entre o desenho (draw.c) e a navegação
 * /edição (input.c), para o texto e o cursor ficarem sempre alinhados. */
#define ITEM_LABEL_W 4

/* Geometria partilhada pelo desenho e pela navegação: pitch horizontal
 * entre colunas e onde começa a primeira coluna. */
#define COL0 13
#define PITCH 31
#define GRID_TOP 3
#define BLOCK_H 12
#define GRID_GAP 2
#define LIST_TOP (GRID_TOP + 2*BLOCK_H + GRID_GAP + 1)
#define LIST_TOP_NOGRID 3
/* func1/2 (eventos): uma única fiada de caixas-mês decorativas, com a
 * lista editável de dias logo por baixo de cada uma. */
#define LIST_TOP_EVENTS (GRID_TOP + BLOCK_H + GRID_GAP)
/* func3 (HOJE): as colunas sem caixa por cima (hábitos/tarefas/agenda)
 * começam bem mais acima, tal como no v1 (move(4+4,...) -- logo a seguir
 * à barra do topo), não só depois das caixas dos meses. */
#define LIST_TOP_SIDE (GRID_TOP + 4)

/* Linhas/colunas mínimas de terminal necessárias para desenhar tudo sem
 * cortar nada -- derivadas das MESMAS constantes usadas para desenhar, ao
 * contrário do v1 em que o requisito de tamanho (55x204) estava
 * hardcoded e completamente desligado do código de desenho. */
#define MIN_COLS (COL0 + MAX_COLS * PITCH + 4)
/* +1 final: a barra de baixo fica fora da caixa, numa linha própria. */
#define MIN_ROWS_FUNC0 (LIST_TOP + GOALS_ROWS + 3 + 1)
#define MIN_ROWS_LIST (LIST_TOP_NOGRID + 31 + 3 + 1)
#define MIN_ROWS_EVENTS (LIST_TOP_EVENTS + 1 + 31 + 3 + 1) /* +1 pelo header_gap */
/* vista HOJE: colunas de hábitos/tarefas, a começar em LIST_TOP_SIDE */
#define MIN_ROWS_SIDE (LIST_TOP_SIDE + 1 + HABITS_ROWS + 3 + 1)
#define MIN_ROWS_A (MIN_ROWS_FUNC0 > MIN_ROWS_LIST ? MIN_ROWS_FUNC0 : MIN_ROWS_LIST)
#define MIN_ROWS_B (MIN_ROWS_A > MIN_ROWS_EVENTS ? MIN_ROWS_A : MIN_ROWS_EVENTS)
#define MIN_ROWS (MIN_ROWS_B > MIN_ROWS_SIDE ? MIN_ROWS_B : MIN_ROWS_SIDE)

/* Devolve as colunas da vista atual (func 0..3) em `cols`, até MAX_COLS.
 * `highlight_row[i]` fica a -1 ou à linha de hoje (para destacar o dia
 * atual a magenta nas vistas de eventos/HOJE). */
int view_build(STATE *st, LISTCOL cols[MAX_COLS], int highlight_row[MAX_COLS]);

#endif
