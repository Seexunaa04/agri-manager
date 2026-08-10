#ifndef ORDERS_H
#define ORDERS_H

#include "models.h"

void create_order(void);
void list_orders(void);
void list_user_orders(int user_id);
int count_active_orders_for_user(int user_id);
int find_order_by_id(int id, Order *out_order);
int get_next_order_id(void);
void generate_order_receipt(const Order *ord, const User *usr, const Product *prd);

#endif // ORDERS_H
