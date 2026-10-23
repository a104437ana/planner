#ifndef PLANNER_INPUT_H
#define PLANNER_INPUT_H

#include "state.h"

/* -1 = sair do programa, 0 = continuar */
int input_handle(STATE *st, int key);

/* Posiciona o cursor do terminal em cima do item selecionado -- chamar
 * depois de qualquer redesenho (draw_view limpa o ecrã, o que também
 * limpa a posição do cursor). */
void input_position_cursor(STATE *st);

#endif
