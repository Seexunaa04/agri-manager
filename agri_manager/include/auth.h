#ifndef AUTH_H
#define AUTH_H

#include "models.h"

extern User g_current_user;
extern int g_is_logged_in;

void seed_default_admin(void);
int login_user(const char *login, const char *password);
void logout_user(void);
int change_password(User *user, const char *new_pass);
int force_first_login_password_change(User *user);

void admin_register_user(void);
void admin_list_users(void);
void admin_toggle_user_status(void);
int find_user_by_id(int id, User *out_user);
int find_user_by_login(const char *login, User *out_user);
int get_next_user_id(void);

#endif 
