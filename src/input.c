#include <ncurses.h>
#include <string.h>
#include "input.h"
#include "view.h"
#include "draw.h"
#include "storage.h"

static void clamp_cursor(STATE *st, LISTCOL cols[MAX_COLS], int n) {
    if (st->cursor.col >= n) st->cursor.col = n - 1;
    if (st->cursor.col < 0) st->cursor.col = 0;
    int max_row = cols[st->cursor.col].count - 1;
    if (st->cursor.row > max_row) st->cursor.row = max_row;
    if (st->cursor.row < 0) st->cursor.row = 0;
}

static void move_cursor(STATE *st, int dx, int dy) {
    LISTCOL cols[MAX_COLS];
    int hi[MAX_COLS];
    int n = view_build(st, cols, hi);
    clamp_cursor(st, cols, n);

    if (dx != 0) {
        int new_col = st->cursor.col + dx;
        if (new_col >= 0 && new_col < n) st->cursor.col = new_col;
    }
    if (dy != 0) {
        int max_row = cols[st->cursor.col].count - 1;
        int new_row = st->cursor.row + dy;
        if (new_row >= 0 && new_row <= max_row) st->cursor.row = new_row;
    }
    clamp_cursor(st, cols, n);

    LISTCOL *sel = &cols[st->cursor.col];
    move(sel->top + 1 + sel->header_gap + st->cursor.row,
         sel->left + ITEM_LABEL_W + (int) strlen(sel->items[st->cursor.row].text));
}

static void cycle_color(STATE *st) {
    LISTCOL cols[MAX_COLS];
    int hi[MAX_COLS];
    int n = view_build(st, cols, hi);
    clamp_cursor(st, cols, n);

    LISTCOL *sel = &cols[st->cursor.col];
    ITEM *item = &sel->items[st->cursor.row];
    if (sel->binary_color) {
        item->color = (item->color == 1) ? 2 : 1; /* vermelho <-> verde */
    } else {
        item->color = (item->color + 1) % 6;
    }
    storage_save_all(st);
}

#define KEY_CLEAR_LINE 21 /* Ctrl+U: limpa o campo todo de uma vez */

/* Redesenha só o próprio item, com espaço a seguir para apagar texto
 * antigo mais comprido -- clrtoeol() limpava a linha TODA até ao fim do
 * ecrã, apagando os números das colunas seguintes na mesma linha. */
static void redraw_item_text(int screen_row, int screen_col, const char *text, int pair) {
    move(screen_row, screen_col);
    attron(COLOR_PAIR(pair));
    printw("%-*s", ITEM_LEN - 1, text);
    attroff(COLOR_PAIR(pair));
}

static void edit_item(STATE *st) {
    LISTCOL cols[MAX_COLS];
    int hi[MAX_COLS];
    int n = view_build(st, cols, hi);
    clamp_cursor(st, cols, n);

    LISTCOL *col = &cols[st->cursor.col];
    ITEM *item = &col->items[st->cursor.row];
    int screen_row = col->top + 1 + col->header_gap + st->cursor.row;
    int screen_col = col->left + ITEM_LABEL_W;
    int pair = color_pair_for(item->color);

    int len = (int) strlen(item->text);
    int pos = len; /* começa no fim, como antes -- mas agora dá para navegar */
    curs_set(1);
    move(screen_row, screen_col + pos);
    refresh();

    int key;
    while ((key = getch()) != 13 && key != '\n') {
        if (key == ERR) continue;
        if (key == KEY_RESIZE) break;

        if (key == KEY_LEFT) {
            if (pos > 0) pos--;
        } else if (key == KEY_RIGHT) {
            if (pos < len) pos++;
        } else if (key == KEY_HOME) {
            pos = 0;
        } else if (key == KEY_END) {
            pos = len;
        } else if (key == KEY_CLEAR_LINE) {
            len = 0;
            pos = 0;
            item->text[0] = '\0';
            redraw_item_text(screen_row, screen_col, item->text, pair);
        } else if (key == KEY_BACKSPACE || key == 127 || key == 8) {
            if (pos > 0) {
                memmove(&item->text[pos - 1], &item->text[pos], len - pos + 1);
                pos--;
                len--;
                redraw_item_text(screen_row, screen_col, item->text, pair);
            }
        } else if (key == KEY_DC) { /* Delete: apaga o carácter a seguir ao cursor */
            if (pos < len) {
                memmove(&item->text[pos], &item->text[pos + 1], len - pos);
                len--;
                redraw_item_text(screen_row, screen_col, item->text, pair);
            }
        } else if (key >= 32 && key < 127 && len < ITEM_LEN - 1) {
            memmove(&item->text[pos + 1], &item->text[pos], len - pos + 1);
            item->text[pos] = (char) key;
            pos++;
            len++;
            redraw_item_text(screen_row, screen_col, item->text, pair);
        } else {
            continue;
        }
        move(screen_row, screen_col + pos);
        refresh();
    }
    storage_save_all(st);
}

void input_position_cursor(STATE *st) {
    move_cursor(st, 0, 0);
}

int input_handle(STATE *st, int key) {
    switch (key) {
        case 'Q': case 'q':
            return -1;
        case KEY_UP:    move_cursor(st, 0, -1); break;
        case KEY_DOWN:  move_cursor(st, 0, +1); break;
        case KEY_LEFT:  move_cursor(st, -1, 0); break;
        case KEY_RIGHT: move_cursor(st, +1, 0); break;
        case 'S': case 's':
            st->func = (st->func + 1) % 4;
            st->cursor.row = 0;
            st->cursor.col = 0;
            break;
        case 'A': case 'a':
            st->func = (st->func + 3) % 4;
            st->cursor.row = 0;
            st->cursor.col = 0;
            break;
        case ' ':
            cycle_color(st);
            break;
        case 13: case '\n':
            edit_item(st);
            break;
        default:
            break;
    }
    return 0;
}
