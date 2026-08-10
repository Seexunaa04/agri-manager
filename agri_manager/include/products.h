#ifndef PRODUCTS_H
#define PRODUCTS_H

#include "models.h"

void add_product(void);
void list_products(void);
void edit_product(void);
void restock_product(void);
int find_product_by_id(int id, Product *out_product);
int find_product_by_code(const char *code, Product *out_product);
int get_next_product_id(void);
int update_product_stock(int id_product, double delta_available, double delta_total);

#endif 
