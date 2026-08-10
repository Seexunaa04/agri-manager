#ifndef DELIVERIES_H
#define DELIVERIES_H

#include "models.h"

void process_delivery(void);
void list_deliveries(void);
int get_next_delivery_id(void);
void generate_delivery_receipt(const Delivery *del, const Order *ord, const User *usr, const Product *prd);

#endif 
