#include "../include/reservations.h"
#include "../include/products.h"
#include "../include/auth.h"
#include "../include/utils.h"
#include "../include/reports.h"

int get_next_reservation_id(void) {
    FILE *f = fopen(FILE_RESERVATIONS, "rb");
    if (!f) return 1;
    Reservation r;
    int max_id = 0;
    while (fread(&r, sizeof(Reservation), 1, f) == 1) {
        if (r.id > max_id) max_id = r.id;
    }
    fclose(f);
    return max_id + 1;
}

void check_and_activate_pending_reservations(int id_product) {
    FILE *f = fopen(FILE_RESERVATIONS, "rb+");
    if (!f) return;

    Reservation r;
    long pos = -1;
    int found = 0;

    while (fread(&r, sizeof(Reservation), 1, f) == 1) {
        if (r.idProduit == id_product && strcmp(r.etat, "EN_ATTENTE") == 0) {
            found = 1;
            pos = ftell(f) - (long)sizeof(Reservation);
            break;
        }
    }

    if (found && pos >= 0) {
        strcpy(r.etat, "DISPONIBLE");
        fseek(f, pos, SEEK_SET);
        fwrite(&r, sizeof(Reservation), 1, f);

        User usr;
        if (!find_user_by_id(r.idUtilisateur, &usr)) {
            strcpy(usr.login, "INCONNU");
        }

        Product prd;
        if (!find_product_by_id(r.idProduit, &prd)) {
            strcpy(prd.designation, "Produit");
        }

        char log_msg[200];
        snprintf(log_msg, sizeof(log_msg), "Passage automatique de la reservation ID %d au statut DISPONIBLE pour le produit '%s'",
                 r.id, prd.designation);
        log_operation("SYSTEM", log_msg);

        printf("\n[NOTIF AUTOMATIQUE] La reservation ID %d pour le produit '%s' est maintenant DISPONIBLE !\n",
               r.id, prd.designation);
    }
    fclose(f);
}

void create_reservation(void) {
    print_header("RESERVATION DE RECOLTE FUTURE");

    if (!g_is_logged_in) {
        printf("[ERREUR] Vous devez etre connecte pour effectuer une reservation.\n");
        return;
    }

    list_products();

    int id_product = read_int("\nEntrez l'ID du produit a reserver : ");
    Product prd;
    if (!find_product_by_id(id_product, &prd)) {
        printf("[ERREUR] Produit ID %d introuvable.\n", id_product);
        return;
    }

    if (prd.quantite_disponible > 0) {
        printf("\n[ERREUR CONTRAINTE] Vous ne pouvez pas réserver ce produit car du stock est actuellement DISPONIBLE (%.2f %s disponible(s)).\n",
               prd.quantite_disponible, prd.unite_mesure);
        printf("La réservation de récolte est réservée exclusivement aux produits en rupture de stock (Stock = 0).\n");
        return;
    }

    double qty = read_double("Entrez la quantite a reserver : ");
    if (qty <= 0) {
        printf("[ERREUR] La quantite doit etre superieure a 0.\n");
        return;
    }

    Reservation r;
    memset(&r, 0, sizeof(Reservation));
    r.id = get_next_reservation_id();
    r.idUtilisateur = g_current_user.id;
    r.idProduit = prd.id;
    r.quantite_reservee = qty;
    get_current_date(r.date_reservation, sizeof(r.date_reservation));
    strcpy(r.etat, "EN_ATTENTE");

    FILE *f = fopen(FILE_RESERVATIONS, "ab");
    if (!f) {
        printf("[ERREUR] Impossible d'enregistrer la reservation.\n");
        return;
    }
    fwrite(&r, sizeof(Reservation), 1, f);
    fclose(f);

    char log_msg[200];
    snprintf(log_msg, sizeof(log_msg), "Reservation de recolte ID %d pour le produit '%s' (Qte: %.2f)",
             r.id, prd.designation, qty);
    log_operation(g_current_user.login, log_msg);

    printf("\n[SUCCES] Reservation enregistree avec succes (ID: %d) !\n", r.id);
    printf("Statut : EN_ATTENTE. Dès que le produit sera réapprovisionné, votre réservation deviendra automatiquement DISPONIBLE.\n");
}

void list_reservations(void) {
    print_header("LISTE DE TOUTES LES RESERVATIONS DE RECOLTES");

    FILE *f = fopen(FILE_RESERVATIONS, "rb");
    if (!f) {
        printf("Aucune reservation enregistree.\n");
        return;
    }

    Reservation r;
    printf("%-4s | %-5s | %-5s | %-12s | %-19s | %-12s\n",
           "ID", "USER", "PROD", "QTE RESERV", "DATE RESERV", "STATUT");
    printf("--------------------------------------------------------------------\n");

    int count = 0;
    while (fread(&r, sizeof(Reservation), 1, f) == 1) {
        printf("%-4d | %-5d | %-5d | %-12.1f | %-19s | %-12s\n",
               r.id, r.idUtilisateur, r.idProduit, r.quantite_reservee,
               r.date_reservation, r.etat);
        count++;
    }
    fclose(f);
    printf("--------------------------------------------------------------------\n");
    printf("Total: %d reservation(s)\n", count);
}

void list_user_reservations(int user_id) {
    print_header("MES RESERVATIONS DE RECOLTES");

    FILE *f = fopen(FILE_RESERVATIONS, "rb");
    if (!f) {
        printf("Aucune reservation enregistree.\n");
        return;
    }

    Reservation r;
    printf("%-4s | %-5s | %-12s | %-19s | %-12s\n",
           "ID", "PROD", "QTE RESERV", "DATE RESERV", "STATUT");
    printf("--------------------------------------------------------------------\n");

    int count = 0;
    while (fread(&r, sizeof(Reservation), 1, f) == 1) {
        if (r.idUtilisateur == user_id) {
            printf("%-4d | %-5d | %-12.1f | %-19s | %-12s\n",
                   r.id, r.idProduit, r.quantite_reservee,
                   r.date_reservation, r.etat);
            count++;
        }
    }
    fclose(f);
    printf("--------------------------------------------------------------------\n");
    printf("Total: %d reservation(s)\n", count);
}
