#ifndef REPORTS_H
#define REPORTS_H

#include "models.h"

void log_operation(const char *login, const char *operation);
void view_history(void);
void generate_daily_report(void);

#endif // REPORTS_H
