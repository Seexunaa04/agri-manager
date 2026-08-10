#include "../include/categories.h"
#include "../include/utils.h"
#include "../include/auth.h"
#include "../include/reports.h"

int get_next_category_id(void) {
    FILE *f = fopen(FILE_CATEGORIES, "rb");
    if (!f) return 1;
    Category c;
    int max_id = 0;
    while (fread(&c, sizeof(Category), 1, f) == 1) {
        if (c.id > max_id) max_id = c.id;
    }
    fclose(f);
    return max_id + 1;
}

int find_category_by_id(int id, Category *out_category) {
    FILE *f = fopen(FILE_CATEGORIES, "rb");
    if (!f) return 0;
    Category c;
    while (fread(&c, sizeof(Category), 1, f) == 1) {
        if (c.id == id) {
            if (out_category) *out_category = c;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void add_category(void) {
    print_header("AJOUT D'UNE CATEGORIE DE PRODUITS");

    Category c;
    memset(&c, 0, sizeof(Category));
    c.id = get_next_category_id();

    read_string("Libelle (ex: Legumes, Fruits, Cereales, Tubercules, Epices) : ", c.libelle, sizeof(c.libelle));
    read_string("Description : ", c.description, sizeof(c.description));
    get_current_date(c.date_creation, sizeof(c.date_creation));

    FILE *f = fopen(FILE_CATEGORIES, "ab");
    if (!f) {
        printf("[ERREUR] Impossible d'ouvrir la base des categories.\n");
        return;
    }
    fwrite(&c, sizeof(Category), 1, f);
    fclose(f);

    char log_msg[150];
    snprintf(log_msg, sizeof(log_msg), "Ajout categorie ID %d: %s", c.id, c.libelle);
    log_operation(g_current_user.login, log_msg);

    printf("\n[SUCCES] Categorie '%s' (ID %d) ajoutee avec succes !\n", c.libelle, c.id);
}

void list_categories(void) {
    print_header("LISTE DES CATEGORIES DE PRODUITS");

    FILE *f = fopen(FILE_CATEGORIES, "rb");
    if (!f) {
        printf("Aucune categorie enregistree.\n");
        return;
    }

    Category c;
    printf("%-4s | %-20s | %-19s | %-35s\n", "ID", "LIBELLE", "DATE CREATION", "DESCRIPTION");
    printf("-----------------------------------------------------------------------------------\n");

    int count = 0;
    while (fread(&c, sizeof(Category), 1, f) == 1) {
        printf("%-4d | %-20.20s | %-19s | %-35.35s\n",
               c.id, c.libelle, c.date_creation, c.description);
        count++;
    }
    fclose(f);
    printf("-----------------------------------------------------------------------------------\n");
    printf("Total: %d categorie(s)\n", count);
}

void edit_category(void) {
    print_header("MODIFIER UNE CATEGORIE");
    list_categories();

    int id = read_int("\nEntrez l'ID de la categorie a modifier : ");
    FILE *f = fopen(FILE_CATEGORIES, "rb+");
    if (!f) return;

    Category c;
    int found = 0;
    while (fread(&c, sizeof(Category), 1, f) == 1) {
        if (c.id == id) {
            found = 1;
            printf("\nModification de la categorie '%s'\n", c.libelle);
            
            char buf[200];
            read_string("Nouveau libelle : ", buf, sizeof(buf));
            if (strlen(buf) > 0) strcpy(c.libelle, buf);

            read_string("Nouvelle description : ", buf, sizeof(buf));
            if (strlen(buf) > 0) strcpy(c.description, buf);

            fseek(f, -((long)sizeof(Category)), SEEK_CUR);
            fwrite(&c, sizeof(Category), 1, f);

            char log_msg[150];
            snprintf(log_msg, sizeof(log_msg), "Modification categorie ID %d (%s)", c.id, c.libelle);
            log_operation(g_current_user.login, log_msg);

            printf("[SUCCES] Categorie mise a jour avec succes !\n");
            break;
        }
    }
    fclose(f);
    if (!found) {
        printf("[ERREUR] Categorie ID %d introuvable.\n", id);
    }
}
