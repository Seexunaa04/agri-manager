#ifndef RESERVATIONS_H
#define RESERVATIONS_H

#include "models.h"

void create_reservation(void);
void list_reservations(void);
void list_user_reservations(int user_id);
int get_next_reservation_id(void);
void check_and_activate_pending_reservations(int id_product);

#endif 
