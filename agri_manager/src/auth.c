#include "../include/auth.h"
#include "../include/utils.h"
#include "../include/reports.h"

User g_current_user;
int g_is_logged_in = 0;

int get_next_user_id(void) {
    FILE *f = fopen(FILE_USERS, "rb");
    if (!f) return 1;
    User u;
    int max_id = 0;
    while (fread(&u, sizeof(User), 1, f) == 1) {
        if (u.id > max_id) max_id = u.id;
    }
    fclose(f);
    return max_id + 1;
}

int find_user_by_login(const char *login, User *out_user) {
    FILE *f = fopen(FILE_USERS, "rb");
    if (!f) return 0;
    User u;
    while (fread(&u, sizeof(User), 1, f) == 1) {
        if (strcmp(u.login, login) == 0) {
            if (out_user) *out_user = u;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int find_user_by_id(int id, User *out_user) {
    FILE *f = fopen(FILE_USERS, "rb");
    if (!f) return 0;
    User u;
    while (fread(&u, sizeof(User), 1, f) == 1) {
        if (u.id == id) {
            if (out_user) *out_user = u;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void seed_default_admin(void) {
    FILE *f = fopen(FILE_USERS, "rb");
    if (!f) {
        // Create initial default admin
        User admin;
        memset(&admin, 0, sizeof(User));
        admin.id = 1;
        strcpy(admin.nom, "ADMIN");
        strcpy(admin.prenom, "Systeme");
        strcpy(admin.telephone, "00000000");
        strcpy(admin.adresse, "Centrale AGRI-MANAGER");
        strcpy(admin.email, "admin@agrimanager.org");
        strcpy(admin.login, "ADMINS"); // Exactly 6 uppercase
        encrypt_password("Agri123", admin.password);
        strcpy(admin.role, "ADMIN");
        strcpy(admin.etat, "ACTIF");
        get_current_date(admin.date_creation, sizeof(admin.date_creation));
        strcpy(admin.date_derniere_connexion, "Jamais");
        admin.first_login = 1; // Mandatory pass change

        FILE *fout = fopen(FILE_USERS, "wb");
        if (fout) {
            fwrite(&admin, sizeof(User), 1, fout);
            fclose(fout);
        }
    } else {
        fclose(f);
    }
}

int login_user(const char *login, const char *password) {
    FILE *f = fopen(FILE_USERS, "rb+");
    if (!f) {
        printf("[ERREUR] Impossible d'ouvrir la base utilisateurs.\n");
        return 0;
    }

    User u;
    long pos = -1;
    int found = 0;

    while (fread(&u, sizeof(User), 1, f) == 1) {
        if (strcmp(u.login, login) == 0) {
            found = 1;
            pos = ftell(f) - (long)sizeof(User);
            break;
        }
    }

    if (!found) {
        fclose(f);
        printf("[ERREUR] Login '%s' introuvable.\n", login);
        return 0;
    }

    if (strcmp(u.etat, "BLOQUÉ") == 0) {
        fclose(f);
        printf("[ERREUR] Compte bloqué. Veuillez contacter un administrateur.\n");
        return 0;
    }

    char decrypted[MAX_PASS_LEN];
    decrypt_password(u.password, decrypted);

    if (strcmp(decrypted, password) != 0) {
        fclose(f);
        printf("[ERREUR] Mot de passe incorrect.\n");
        return 0;
    }

    // Update last connection date
    get_current_date(u.date_derniere_connexion, sizeof(u.date_derniere_connexion));
    fseek(f, pos, SEEK_SET);
    fwrite(&u, sizeof(User), 1, f);
    fclose(f);

    g_current_user = u;
    g_is_logged_in = 1;

    // Check first login constraint
    if (u.first_login == 1) {
        printf("\n=======================================================\n");
        printf(" [ATTENTION] Premiere connexion detectee.\n");
        printf(" Vous devez obligatoirement modifier votre mot de passe.\n");
        printf("=======================================================\n\n");
        if (!force_first_login_password_change(&g_current_user)) {
            g_is_logged_in = 0;
            return 0;
        }
    }

    log_operation(g_current_user.login, "Connexion reussie");
    return 1;
}

int force_first_login_password_change(User *user) {
    char new_p1[MAX_PASS_LEN], new_p2[MAX_PASS_LEN];
    while (1) {
        read_password_masked("Nouveau mot de passe : ", new_p1, sizeof(new_p1));
        read_password_masked("Confirmez le nouveau mot de passe : ", new_p2, sizeof(new_p2));

        if (strlen(new_p1) < 4) {
            printf("[ERREUR] Le mot de passe doit contenir au moins 4 caracteres.\n");
            continue;
        }
        if (strcmp(new_p1, "Agri123") == 0) {
            printf("[ERREUR] Le nouveau mot de passe doit etre different du mot de passe par defaut.\n");
            continue;
        }
        if (strcmp(new_p1, new_p2) != 0) {
            printf("[ERREUR] Les mots de passe ne correspondent pas. Reessayez.\n");
            continue;
        }
        break;
    }

    user->first_login = 0;
    return change_password(user, new_p1);
}

int change_password(User *user, const char *new_pass) {
    FILE *f = fopen(FILE_USERS, "rb+");
    if (!f) return 0;

    User u;
    while (fread(&u, sizeof(User), 1, f) == 1) {
        if (u.id == user->id) {
            encrypt_password(new_pass, u.password);
            u.first_login = 0;
            fseek(f, -((long)sizeof(User)), SEEK_CUR);
            fwrite(&u, sizeof(User), 1, f);
            fclose(f);
            *user = u;
            log_operation(user->login, "Modification du mot de passe");
            printf("[SUCCES] Mot de passe modifie avec succes.\n");
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void logout_user(void) {
    if (g_is_logged_in) {
        log_operation(g_current_user.login, "Deconnexion");
        g_is_logged_in = 0;
        memset(&g_current_user, 0, sizeof(User));
        printf("[INFO] Deconnexion effectuee.\n");
    }
}

void admin_register_user(void) {
    print_header("CREATION D'UN NOUVEL UTILISATEUR");

    User u;
    memset(&u, 0, sizeof(User));

    u.id = get_next_user_id();

    read_string("Nom : ", u.nom, sizeof(u.nom));
    read_string("Prenom : ", u.prenom, sizeof(u.prenom));
    read_string("Telephone : ", u.telephone, sizeof(u.telephone));
    read_string("Adresse : ", u.adresse, sizeof(u.adresse));
    read_string("E-mail : ", u.email, sizeof(u.email));

    while (1) {
        read_string("Login (6 lettres majuscules ex: MATTOI) : ", u.login, sizeof(u.login));
        if (!validate_login(u.login)) {
            printf("[ERREUR] Le login doit comporter exactement 6 lettres majuscules (A-Z).\n");
            continue;
        }
        User tmp;
        if (find_user_by_login(u.login, &tmp)) {
            printf("[ERREUR] Le login '%s' existe deja dans le systeme.\n", u.login);
            continue;
        }
        break;
    }

    int choice = read_int("Role (1: USER / Acheteur / Agriculteur, 2: ADMIN) : ");
    if (choice == 2) {
        strcpy(u.role, "ADMIN");
    } else {
        strcpy(u.role, "USER");
    }

    // Default password Agri123
    encrypt_password("Agri123", u.password);
    strcpy(u.etat, "ACTIF");
    get_current_date(u.date_creation, sizeof(u.date_creation));
    strcpy(u.date_derniere_connexion, "Jamais");
    u.first_login = 1;

    FILE *f = fopen(FILE_USERS, "ab");
    if (!f) {
        printf("[ERREUR] Impossible d'enregistrer l'utilisateur.\n");
        return;
    }
    fwrite(&u, sizeof(User), 1, f);
    fclose(f);

    char log_msg[100];
    snprintf(log_msg, sizeof(log_msg), "Creation utilisateur ID %d (Login: %s, Role: %s)", u.id, u.login, u.role);
    log_operation(g_current_user.login, log_msg);

    printf("\n[SUCCES] Utilisateur '%s' cree avec succes !\n", u.login);
    printf("Mot de passe par defaut : Agri123 (Changement obligatoire a la 1ere connexion).\n");
}

void admin_list_users(void) {
    print_header("LISTE DES UTILISATEURS ET ADMINISTRATEURS");

    FILE *f = fopen(FILE_USERS, "rb");
    if (!f) {
        printf("Aucun utilisateur enregistre.\n");
        return;
    }

    User u;
    printf("%-4s | %-7s | %-20s | %-10s | %-8s | %-19s\n", "ID", "LOGIN", "NOM PRENOM", "ROLE", "ETAT", "CREATION");
    printf("-----------------------------------------------------------------------------------\n");

    int count = 0;
    while (fread(&u, sizeof(User), 1, f) == 1) {
        char fullname[100];
        snprintf(fullname, sizeof(fullname), "%s %s", u.nom, u.prenom);
        printf("%-4d | %-7s | %-20.20s | %-10s | %-8s | %-19s\n",
               u.id, u.login, fullname, u.role, u.etat, u.date_creation);
        count++;
    }
    fclose(f);
    printf("-----------------------------------------------------------------------------------\n");
    printf("Total: %d utilisateur(s)\n", count);
}

void admin_toggle_user_status(void) {
    print_header("CHANGER L'ETAT D'UN UTILISATEUR (ACTIF / BLOQUE)");
    admin_list_users();

    int id = read_int("\nEntrez l'ID de l'utilisateur a modifier : ");
    FILE *f = fopen(FILE_USERS, "rb+");
    if (!f) return;

    User u;
    int found = 0;
    while (fread(&u, sizeof(User), 1, f) == 1) {
        if (u.id == id) {
            found = 1;
            if (strcmp(u.login, g_current_user.login) == 0) {
                printf("[ERREUR] Vous ne pouvez pas bloquer votre propre compte !\n");
                fclose(f);
                return;
            }
            if (strcmp(u.etat, "ACTIF") == 0) {
                strcpy(u.etat, "BLOQUÉ");
            } else {
                strcpy(u.etat, "ACTIF");
            }
            fseek(f, -((long)sizeof(User)), SEEK_CUR);
            fwrite(&u, sizeof(User), 1, f);

            char log_msg[100];
            snprintf(log_msg, sizeof(log_msg), "Changement statut utilisateur ID %d (Login %s) -> %s", u.id, u.login, u.etat);
            log_operation(g_current_user.login, log_msg);

            printf("[SUCCES] L'etat de l'utilisateur %s est maintenant: %s\n", u.login, u.etat);
            break;
        }
    }
    fclose(f);
    if (!found) {
        printf("[ERREUR] Utilisateur avec l'ID %d introuvable.\n", id);
    }
}
