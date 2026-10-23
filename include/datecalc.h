#ifndef PLANNER_DATECALC_H
#define PLANNER_DATECALC_H

int is_leap_year(int year);
int days_in_month(int month /* 1-12 */, int year);
int day_of_week(int day, int month /* 1-12 */, int year); /* 1=Monday .. 7=Sunday */

/* Ano letivo (Julho a Junho): dado o mês/ano atuais, devolve os dois anos
 * civis que o calendário de 12 meses abrange (ano1 = Jul-Dez, ano2 = Jan-Jun). */
void academic_year(int month /* 1-12 */, int year, int *ano1, int *ano2);

/* Ano civil a que pertence o mês 0-indexado (0=Jan..11=Dez) dentro do ano letivo. */
int year_for_month0(int month0, int ano1, int ano2);

#endif
