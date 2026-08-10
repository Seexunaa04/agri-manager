#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <direct.h>
#include <sys/stat.h>

void init_directories(void);
void get_current_date(char *buffer, size_t size);
void get_date_code(char *buffer, size_t size);
void get_date_only(char *buffer, size_t size);
void get_future_date_days(int days, char *buffer, size_t size);
int calculate_days_difference(const char *date_start, const char *date_end);

void encrypt_password(const char *input, char *output);
void decrypt_password(const char *input, char *output);

int validate_login(const char *login);
void read_string(const char *prompt, char *buffer, size_t size);
int read_int(const char *prompt);
double read_double(const char *prompt);
void read_password_masked(const char *prompt, char *buffer, size_t size);

void clear_screen(void);
void pause_screen(void);
void print_header(const char *title);

#endif 
