#ifndef PRODUCERS_H
#define PRODUCERS_H

#include "models.h"

void add_producer(void);
void list_producers(void);
void edit_producer(void);
void delete_producer(void);
int find_producer_by_id(int id, Producer *out_producer);
int get_next_producer_id(void);
void increment_producer_product_count(int producer_id, int delta);

#endif // PRODUCERS_H
