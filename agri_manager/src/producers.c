#include "../include/producers.h"
#include "../include/utils.h"
#include "../include/auth.h"
#include "../include/reports.h"

int get_next_producer_id(void) {
    FILE *f = fopen(FILE_PRODUCERS, "rb");
    if (!f) return 1;
    Producer p;
    int max_id = 0;
    while (fread(&p, sizeof(Producer), 1, f) == 1) {
        if (p.id > max_id) max_id = p.id;
    }
    fclose(f);
    return max_id + 1;
}

int find_producer_by_id(int id, Producer *out_producer) {
    FILE *f = fopen(FILE_PRODUCERS, "rb");
    if (!f) return 0;
    Producer p;
    while (fread(&p, sizeof(Producer), 1, f) == 1) {
        if (p.id == id) {
            if (out_producer) *out_producer = p;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void increment_producer_product_count(int producer_id, int delta) {
    FILE *f = fopen(FILE_PRODUCERS, "rb+");
    if (!f) return;
    Producer p;
    while (fread(&p, sizeof(Producer), 1, f) == 1) {
        if (p.id == producer_id) {
            p.nb_produits_enregistres += delta;
            if (p.nb_produits_enregistres < 0) p.nb_produits_enregistres = 0;
            fseek(f, -((long)sizeof(Producer)), SEEK_CUR);
            fwrite(&p, sizeof(Producer), 1, f);
            break;
        }
    }
    fclose(f);
}

void add_producer(void) {
    print_header("AJOUT D'UN PRODUCTEUR / FOURNISSEUR");

    Producer p;
    memset(&p, 0, sizeof(Producer));
    p.id = get_next_producer_id();

    read_string("Nom complet ou Nom de la cooperative : ", p.nom_complet, sizeof(p.nom_complet));
    read_string("Region / Localisation : ", p.region, sizeof(p.region));
    read_string("Telephone : ", p.telephone, sizeof(p.telephone));
    read_string("Description des cultures (ex: Mangues, Mais, Tomates) : ", p.description_cultures, sizeof(p.description_cultures));
    p.nb_produits_enregistres = 0;

    FILE *f = fopen(FILE_PRODUCERS, "ab");
    if (!f) {
        printf("[ERREUR] Impossible d'ouvrir la base des producteurs.\n");
        return;
    }
    fwrite(&p, sizeof(Producer), 1, f);
    fclose(f);

    char log_msg[150];
    snprintf(log_msg, sizeof(log_msg), "Ajout du producteur ID %d: %s (%s)", p.id, p.nom_complet, p.region);
    log_operation(g_current_user.login, log_msg);

    printf("\n[SUCCES] Producteur '%s' (ID %d) enregistre avec succes !\n", p.nom_complet, p.id);
}

void list_producers(void) {
    print_header("LISTE DES PRODUCTEURS / FOURNISSEURS");

    FILE *f = fopen(FILE_PRODUCERS, "rb");
    if (!f) {
        printf("Aucun producteur enregistre.\n");
        return;
    }

    Producer p;
    printf("%-4s | %-25s | %-15s | %-15s | %-6s | %-20s\n", "ID", "NOM / COOPERATIVE", "REGION", "TELEPHONE", "NB PRD", "CULTURES");
    printf("--------------------------------------------------------------------------------------------------\n");

    int count = 0;
    while (fread(&p, sizeof(Producer), 1, f) == 1) {
        printf("%-4d | %-25.25s | %-15.15s | %-15.15s | %-6d | %-20.20s\n",
               p.id, p.nom_complet, p.region, p.telephone, p.nb_produits_enregistres, p.description_cultures);
        count++;
    }
    fclose(f);
    printf("--------------------------------------------------------------------------------------------------\n");
    printf("Total: %d producteur(s)\n", count);
}

void edit_producer(void) {
    print_header("MODIFIER UN PRODUCTEUR");
    list_producers();

    int id = read_int("\nEntrez l'ID du producteur a modifier : ");
    FILE *f = fopen(FILE_PRODUCERS, "rb+");
    if (!f) return;

    Producer p;
    int found = 0;
    while (fread(&p, sizeof(Producer), 1, f) == 1) {
        if (p.id == id) {
            found = 1;
            printf("\nModification du producteur '%s' (Laissez vide pour conserver la valeur actuelle)\n", p.nom_complet);
            
            char buf[200];
            read_string("Nouveau nom/cooperative : ", buf, sizeof(buf));
            if (strlen(buf) > 0) strcpy(p.nom_complet, buf);

            read_string("Nouvelle region : ", buf, sizeof(buf));
            if (strlen(buf) > 0) strcpy(p.region, buf);

            read_string("Nouveau telephone : ", buf, sizeof(buf));
            if (strlen(buf) > 0) strcpy(p.telephone, buf);

            read_string("Nouvelle description des cultures : ", buf, sizeof(buf));
            if (strlen(buf) > 0) strcpy(p.description_cultures, buf);

            fseek(f, -((long)sizeof(Producer)), SEEK_CUR);
            fwrite(&p, sizeof(Producer), 1, f);

            char log_msg[150];
            snprintf(log_msg, sizeof(log_msg), "Modification du producteur ID %d (%s)", p.id, p.nom_complet);
            log_operation(g_current_user.login, log_msg);

            printf("[SUCCES] Producteur mis a jour avec succes !\n");
            break;
        }
    }
    fclose(f);
    if (!found) {
        printf("[ERREUR] Producteur ID %d introuvable.\n", id);
    }
}

void delete_producer(void) {
    print_header("SUPPRIMER UN PRODUCTEUR");
    list_producers();

    int id = read_int("\nEntrez l'ID du producteur a supprimer : ");

    Producer p;
    if (!find_producer_by_id(id, &p)) {
        printf("[ERREUR] Producteur ID %d introuvable.\n", id);
        return;
    }

    FILE *fp = fopen(FILE_PRODUCTS, "rb");
    if (fp) {
        Product prd;
        while (fread(&prd, sizeof(Product), 1, fp) == 1) {
            if (prd.idProducteur == id) {
                fclose(fp);
                printf("\n[ERREUR DES CONTRAINTES] Impossible de supprimer le producteur '%s' (ID %d) !\n", p.nom_complet, id);
                printf("Le produit '%s' (Code: %s) lui est associe.\n", prd.designation, prd.codeProduit);
                return;
            }
        }
        fclose(fp);
    }

    FILE *f = fopen(FILE_PRODUCERS, "rb");
    FILE *ftemp = fopen("DATABASE/PRODUCERS_TMP.dat", "wb");
    if (!f || !ftemp) return;

    Producer item;
    while (fread(&item, sizeof(Producer), 1, f) == 1) {
        if (item.id != id) {
            fwrite(&item, sizeof(Producer), 1, ftemp);
        }
    }
    fclose(f);
    fclose(ftemp);

    remove(FILE_PRODUCERS);
    rename("DATABASE/PRODUCERS_TMP.dat", FILE_PRODUCERS);

    char log_msg[150];
    snprintf(log_msg, sizeof(log_msg), "Suppression du producteur ID %d (%s)", id, p.nom_complet);
    log_operation(g_current_user.login, log_msg);

    printf("[SUCCES] Producteur '%s' supprime avec succes.\n", p.nom_complet);
}
