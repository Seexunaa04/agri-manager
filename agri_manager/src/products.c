#include "../include/products.h"
#include "../include/producers.h"
#include "../include/categories.h"
#include "../include/utils.h"
#include "../include/auth.h"
#include "../include/reports.h"

void check_and_activate_pending_reservations(int id_product);

int get_next_product_id(void) {
    FILE *f = fopen(FILE_PRODUCTS, "rb");
    if (!f) return 1;
    Product p;
    int max_id = 0;
    while (fread(&p, sizeof(Product), 1, f) == 1) {
        if (p.id > max_id) max_id = p.id;
    }
    fclose(f);
    return max_id + 1;
}

int find_product_by_id(int id, Product *out_product) {
    FILE *f = fopen(FILE_PRODUCTS, "rb");
    if (!f) return 0;
    Product p;
    while (fread(&p, sizeof(Product), 1, f) == 1) {
        if (p.id == id) {
            if (out_product) *out_product = p;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int find_product_by_code(const char *code, Product *out_product) {
    FILE *f = fopen(FILE_PRODUCTS, "rb");
    if (!f) return 0;
    Product p;
    while (fread(&p, sizeof(Product), 1, f) == 1) {
        if (strcmp(p.codeProduit, code) == 0) {
            if (out_product) *out_product = p;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int update_product_stock(int id_product, double delta_available, double delta_total) {
    FILE *f = fopen(FILE_PRODUCTS, "rb+");
    if (!f) return 0;
    Product p;
    int success = 0;
    while (fread(&p, sizeof(Product), 1, f) == 1) {
        if (p.id == id_product) {
            p.quantite_disponible += delta_available;
            p.quantite_totale_stock += delta_total;

            if (p.quantite_disponible < 0) p.quantite_disponible = 0;
            if (p.quantite_totale_stock < 0) p.quantite_totale_stock = 0;

            fseek(f, -((long)sizeof(Product)), SEEK_CUR);
            fwrite(&p, sizeof(Product), 1, f);
            success = 1;

            if (delta_available > 0 || delta_total > 0) {
                check_and_activate_pending_reservations(id_product);
            }
            break;
        }
    }
    fclose(f);
    return success;
}

void add_product(void) {
    print_header("AJOUT D'UN NOUVEAU PRODUIT AGRICOLE");

    Product p;
    memset(&p, 0, sizeof(Product));
    p.id = get_next_product_id();

    while (1) {
        read_string("Code Produit (unique ex: PRD9781234567890) : ", p.codeProduit, sizeof(p.codeProduit));
        if (strlen(p.codeProduit) == 0) {
            printf("[ERREUR] Le code produit est obligatoire.\n");
            continue;
        }
        Product tmp;
        if (find_product_by_code(p.codeProduit, &tmp)) {
            printf("[ERREUR CONTRAINTE] Un produit avec le code '%s' existe deja !\n", p.codeProduit);
            continue;
        }
        break;
    }

    read_string("Designation / Titre (ex: Mangue Kent - 50Kg) : ", p.designation, sizeof(p.designation));

    list_producers();
    while (1) {
        p.idProducteur = read_int("\nID du Producteur : ");
        Producer prod;
        if (!find_producer_by_id(p.idProducteur, &prod)) {
            printf("[ERREUR CONTRAINTE] Aucun producteur avec l'ID %d n'existe.\n", p.idProducteur);
            continue;
        }
        break;
    }

    list_categories();
    while (1) {
        p.idCategorie = read_int("\nID de la Categorie : ");
        Category cat;
        if (!find_category_by_id(p.idCategorie, &cat)) {
            printf("[ERREUR CONTRAINTE] Aucune categorie avec l'ID %d n'existe.\n", p.idCategorie);
            continue;
        }
        break;
    }

    read_string("Unite de mesure (Kg, Tonne, Sac, Caisse) : ", p.unite_mesure, sizeof(p.unite_mesure));
    p.prix_unitaire = read_double("Prix unitaire (FCFA) : ");
    p.quantite_totale_stock = read_double("Quantite totale en stock : ");
    p.quantite_disponible = p.quantite_totale_stock;

    read_string("Emplacement dans l'entrepot (ex: Alveole B12) : ", p.emplacement, sizeof(p.emplacement));
    p.duree_conservation = read_int("Duree de conservation (en jours) : ");
    get_current_date(p.date_recolte, sizeof(p.date_recolte));

    FILE *f = fopen(FILE_PRODUCTS, "ab");
    if (!f) {
        printf("[ERREUR] Impossible d'ouvrir la base de donnees des produits.\n");
        return;
    }
    fwrite(&p, sizeof(Product), 1, f);
    fclose(f);

    increment_producer_product_count(p.idProducteur, 1);

    char log_msg[200];
    snprintf(log_msg, sizeof(log_msg), "Ajout du produit \"%s\" - Code: %s", p.designation, p.codeProduit);
    log_operation(g_current_user.login, log_msg);

    printf("\n[SUCCES] Produit '%s' (Code %s) ajoute avec succes !\n", p.designation, p.codeProduit);

    if (p.quantite_disponible > 0) {
        check_and_activate_pending_reservations(p.id);
    }
}

void list_products(void) {
    print_header("LISTE DES PRODUITS AGRICOLES");

    FILE *f = fopen(FILE_PRODUCTS, "rb");
    if (!f) {
        printf("Aucun produit agricole enregistre.\n");
        return;
    }

    Product p;
    printf("%-4s | %-16s | %-22s | %-10s | %-8s | %-8s | %-12s\n",
           "ID", "CODE", "DESIGNATION", "PRIX (FCFA)", "STOCK TOT", "DISPO", "EMPLACEMENT");
    printf("---------------------------------------------------------------------------------------------\n");

    int count = 0;
    while (fread(&p, sizeof(Product), 1, f) == 1) {
        printf("%-4d | %-16.16s | %-22.22s | %-10.2f | %-8.1f | %-8.1f | %-12.12s\n",
               p.id, p.codeProduit, p.designation, p.prix_unitaire,
               p.quantite_totale_stock, p.quantite_disponible, p.emplacement);
        count++;
    }
    fclose(f);
    printf("---------------------------------------------------------------------------------------------\n");
    printf("Total: %d produit(s)\n", count);
}

void edit_product(void) {
    print_header("MODIFIER UN PRODUIT");
    list_products();

    int id = read_int("\nEntrez l'ID du produit a modifier : ");
    FILE *f = fopen(FILE_PRODUCTS, "rb+");
    if (!f) return;

    Product p;
    int found = 0;
    while (fread(&p, sizeof(Product), 1, f) == 1) {
        if (p.id == id) {
            found = 1;
            printf("\nModification du produit '%s' (Code: %s)\n", p.designation, p.codeProduit);
            
            char buf[200];
            read_string("Nouvelle designation : ", buf, sizeof(buf));
            if (strlen(buf) > 0) strcpy(p.designation, buf);

            read_string("Nouvelle unite de mesure : ", buf, sizeof(buf));
            if (strlen(buf) > 0) strcpy(p.unite_mesure, buf);

            printf("Prix unitaire actuel: %.2f FCFA. Entrez le nouveau prix (0 pour conserver) : ", p.prix_unitaire);
            double np = read_double("");
            if (np > 0) p.prix_unitaire = np;

            read_string("Nouvel emplacement : ", buf, sizeof(buf));
            if (strlen(buf) > 0) strcpy(p.emplacement, buf);

            fseek(f, -((long)sizeof(Product)), SEEK_CUR);
            fwrite(&p, sizeof(Product), 1, f);

            char log_msg[150];
            snprintf(log_msg, sizeof(log_msg), "Modification du produit ID %d (%s)", p.id, p.designation);
            log_operation(g_current_user.login, log_msg);

            printf("[SUCCES] Produit mis a jour avec succes !\n");
            break;
        }
    }
    fclose(f);
    if (!found) {
        printf("[ERREUR] Produit ID %d introuvable.\n", id);
    }
}

void restock_product(void) {
    print_header("REAPPROVISIONNEMENT EN STOCK D'UN PRODUIT");
    list_products();

    int id = read_int("\nEntrez l'ID du produit a réapprovisionner : ");
    Product p;
    if (!find_product_by_id(id, &p)) {
        printf("[ERREUR] Produit ID %d introuvable.\n", id);
        return;
    }

    double qty = read_double("Quantite a ajouter au stock : ");
    if (qty <= 0) {
        printf("[ERREUR] La quantite doit etre superieure a 0.\n");
        return;
    }

    update_product_stock(id, qty, qty);

    char log_msg[150];
    snprintf(log_msg, sizeof(log_msg), "Reapprovisionnement produit \"%s\" (+%.2f %s)", p.designation, qty, p.unite_mesure);
    log_operation(g_current_user.login, log_msg);

    printf("\n[SUCCES] Stock mis a jour ! Nouvelle quantite disponible pour '%s' : %.2f %s\n",
           p.designation, p.quantite_disponible + qty, p.unite_mesure);
}
