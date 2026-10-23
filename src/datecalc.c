#include <time.h>
#include "datecalc.h"

int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int days_in_month(int month, int year) {
    static const int lengths[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 2 && is_leap_year(year)) return 29;
    return lengths[month - 1];
}

/* Usa mktime, tal como o v2 -- mais robusto do que as fórmulas bespoke do
 * v1 (julho1_diaSemana/janeiro1_diaSemana), que só sabiam calcular 1 de
 * Julho e 1 de Janeiro e propagavam o dia da semana manualmente a partir daí. */
int day_of_week(int day, int month, int year) {
    struct tm t = {0};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = 12; /* evita problemas de DST à volta da meia-noite */
    if (mktime(&t) == -1) return -1;
    return t.tm_wday == 0 ? 7 : t.tm_wday;
}

void academic_year(int month, int year, int *ano1, int *ano2) {
    if (month >= 7) {
        *ano1 = year;
        *ano2 = year + 1;
    } else {
        *ano2 = year;
        *ano1 = year - 1;
    }
}

int year_for_month0(int month0, int ano1, int ano2) {
    return month0 >= 6 ? ano1 : ano2;
}
