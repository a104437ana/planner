#ifndef PLANNER_STORAGE_H
#define PLANNER_STORAGE_H

#include "state.h"

/* Garante que ~/.config/planner existe e devolve o seu caminho (string
 * estática, válida até à próxima chamada). Devolve NULL em erro. */
const char *storage_dir(void);

/* Carrega/guarda todos os dados persistentes do planner.
 *
 * Formato: um ficheiro de texto por categoria, uma linha por item, cada
 * linha "<cor>\t<texto>\n". Ao contrário do v1 (um ficheiro de texto +
 * um segundo ficheiro só com as cores, por categoria -- 8 ficheiros que
 * podem facilmente dessincronizar-se se um dos dois falhar a escrever) e
 * do v2 (um único ficheiro binário de inteiros ofuscados por XOR para a
 * password), aqui cada categoria vive num único ficheiro de texto simples,
 * fácil de inspecionar ou editar à mão. */
void storage_load(STATE *st);
void storage_save_goals(STATE *st);
void storage_save_habits(STATE *st);
void storage_save_todos(STATE *st);
void storage_save_days(STATE *st);
void storage_save_meta(STATE *st);
void storage_save_all(STATE *st);

#endif
