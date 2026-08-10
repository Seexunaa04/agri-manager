#ifndef CATEGORIES_H
#define CATEGORIES_H

#include "models.h"

void add_category(void);
void list_categories(void);
void edit_category(void);
int find_category_by_id(int id, Category *out_category);
int get_next_category_id(void);

#endif // CATEGORIES_H
