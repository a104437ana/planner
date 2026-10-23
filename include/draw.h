#ifndef PLANNER_DRAW_H
#define PLANNER_DRAW_H

#include "state.h"
#include "view.h"

void draw_init_colors(void);
int color_pair_for(int color); /* 0..5 -> ncurses pair id */

void draw_view(STATE *st);           /* desenho completo da vista atual */
void draw_func_label(int func);      /* só o nome da funcionalidade na barra de topo */
void draw_clock(STATE *st);          /* só a hora/minuto/dia/mes/ano/dia-da-semana */

#endif
