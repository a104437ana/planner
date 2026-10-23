#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "storage.h"
#include "datecalc.h"

static char dir_path[512];

const char *storage_dir(void) {
    const char *home = getenv("HOME");
    if (home == NULL) return NULL;

    char config[480];
    snprintf(config, sizeof(config), "%s/.config", home);
    if (mkdir(config, 0755) != 0 && errno != EEXIST) return NULL;

    snprintf(dir_path, sizeof(dir_path), "%s/.config/planner", home);
    if (mkdir(dir_path, 0755) != 0 && errno != EEXIST) return NULL;

    return dir_path;
}

static void item_path(char *out, size_t out_len, const char *name) {
    snprintf(out, out_len, "%s/%s.txt", storage_dir(), name);
}

static void save_items(const char *name, ITEM *items, int n) {
    char path[600];
    item_path(path, sizeof(path), name);
    FILE *f = fopen(path, "w");
    if (!f) return;
    for (int i = 0; i < n; i++) {
        fprintf(f, "%d\t%s\n", items[i].color, items[i].text);
    }
    fclose(f);
}

static void load_items(const char *name, ITEM *items, int n) {
    char path[600];
    item_path(path, sizeof(path), name);
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[600];
    for (int i = 0; i < n; i++) {
        if (!fgets(line, sizeof(line), f)) break;
        int color = 0;
        char *tab = strchr(line, '\t');
        if (!tab) continue;
        *tab = '\0';
        color = atoi(line);
        if (color < 0 || color > 5) color = 0;
        items[i].color = color;

        char *text = tab + 1;
        size_t len = strlen(text);
        while (len > 0 && (text[len-1] == '\n' || text[len-1] == '\r')) text[--len] = '\0';
        strncpy(items[i].text, text, ITEM_LEN - 1);
        items[i].text[ITEM_LEN - 1] = '\0';
    }
    fclose(f);
}

void storage_save_goals(STATE *st)  { save_items("objetivos", st->goals, GOALS_N); }
void storage_save_habits(STATE *st) { save_items("habitos", st->habits, HABITS_N); }
void storage_save_todos(STATE *st)  { save_items("tarefas", st->todos, TODOS_N); }

void storage_save_days(STATE *st) {
    char path[600];
    item_path(path, sizeof(path), "dias");
    FILE *f = fopen(path, "w");
    if (!f) return;
    for (int m = 0; m < 12; m++) {
        int year = year_for_month0(m, st->ano1, st->ano2);
        int n = days_in_month(m + 1, year);
        for (int d = 0; d < n; d++) {
            fprintf(f, "%d\t%s\n", st->day[m][d].color, st->day[m][d].text);
        }
    }
    fclose(f);
}

static void load_days(STATE *st) {
    char path[600];
    item_path(path, sizeof(path), "dias");
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[600];
    for (int m = 0; m < 12; m++) {
        int year = year_for_month0(m, st->ano1, st->ano2);
        int n = days_in_month(m + 1, year);
        for (int d = 0; d < n; d++) {
            if (!fgets(line, sizeof(line), f)) { fclose(f); return; }
            char *tab = strchr(line, '\t');
            if (!tab) continue;
            *tab = '\0';
            int color = atoi(line);
            if (color < 0 || color > 5) color = 0;
            st->day[m][d].color = color;

            char *text = tab + 1;
            size_t len = strlen(text);
            while (len > 0 && (text[len-1] == '\n' || text[len-1] == '\r')) text[--len] = '\0';
            strncpy(st->day[m][d].text, text, ITEM_LEN - 1);
            st->day[m][d].text[ITEM_LEN - 1] = '\0';
        }
    }
    fclose(f);
}

void storage_save_meta(STATE *st) {
    char path[600];
    item_path(path, sizeof(path), "meta");
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "%d\n%d\n%d\n", st->last_reset_day, st->last_reset_mon, st->last_reset_year);
    fclose(f);
}

static void load_meta(STATE *st) {
    char path[600];
    item_path(path, sizeof(path), "meta");
    FILE *f = fopen(path, "r");
    st->last_reset_day = 0;
    st->last_reset_mon = 0;
    st->last_reset_year = 0;
    if (!f) return;
    if (fscanf(f, "%d %d %d", &st->last_reset_day, &st->last_reset_mon, &st->last_reset_year) != 3) {
        st->last_reset_day = 0;
        st->last_reset_mon = 0;
        st->last_reset_year = 0;
    }
    fclose(f);
}

void storage_save_all(STATE *st) {
    storage_save_goals(st);
    storage_save_habits(st);
    storage_save_todos(st);
    storage_save_days(st);
    storage_save_meta(st);
}

void storage_load(STATE *st) {
    load_items("objetivos", st->goals, GOALS_N);
    load_items("habitos", st->habits, HABITS_N);
    load_items("tarefas", st->todos, TODOS_N);
    load_days(st);
    load_meta(st);

    /* Os hábitos diários são caixas de verificação que devem reiniciar a
     * cada novo dia -- voltam a vermelho (por fazer), tal como começam. */
    int today = st->today.tm_mday;
    int month = st->today.tm_mon + 1;
    int year = st->today.tm_year + 1900;
    if (st->last_reset_day != today || st->last_reset_mon != month || st->last_reset_year != year) {
        for (int i = 0; i < HABITS_N; i++) st->habits[i].color = 1;
        st->last_reset_day = today;
        st->last_reset_mon = month;
        st->last_reset_year = year;
        storage_save_habits(st);
        storage_save_meta(st);
    }
}
