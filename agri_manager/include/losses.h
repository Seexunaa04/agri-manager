#ifndef LOSSES_H
#define LOSSES_H

#include "models.h"

void record_loss(const Loss *loss);
void add_manual_spoilage_loss(void);
void list_losses(void);
int get_next_loss_id(void);
double get_total_losses_amount(void);

#endif 
