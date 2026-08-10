#include "../include/orders.h"
#include "../include/products.h"
#include "../include/auth.h"
#include "../include/utils.h"
#include "../include/reports.h"

int get_next_order_id(void) {
    FILE *f = fopen(FILE_ORDERS, "rb");
    if (!f) return 1;
    Order o;
    int max_id = 0;
    while (fread(&o, sizeof(Order), 1, f) == 1) {
        if (o.id > max_id) max_id = o.id;
    }
    fclose(f);
    return max_id + 1;
}

int find_order_by_id(int id, Order *out_order) {
    FILE *f = fopen(FILE_ORDERS, "rb");
    if (!f) return 0;
    Order o;
    while (fread(&o, sizeof(Order), 1, f) == 1) {
        if (o.id == id) {
            if (out_order) *out_order = o;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int count_active_orders_for_user(int user_id) {
    FILE *f = fopen(FILE_ORDERS, "rb");
    if (!f) return 0;
    Order o;
    int count = 0;
    while (fread(&o, sizeof(Order), 1, f) == 1) {
        if (o.idUtilisateur == user_id && (strcmp(o.etat, "EN_COURS") == 0 || strcmp(o.etat, "EN_RETARD") == 0)) {
            count++;
        }
    }
    fclose(f);
    return count;
}

void generate_order_receipt(const Order *ord, const User *usr, const Product *prd) {
    char date_code[30];
    get_date_code(date_code, sizeof(date_code));

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/ORDER_%s_%s.txt", DIR_REPORTS_ORDERS, date_code, usr->login);

    FILE *f = fopen(filename, "w");
    if (!f) return;

    fprintf(f, "=======================================================\n");
    fprintf(f, "               AGRI-MANAGER - RECU DE COMMANDE        \n");
    fprintf(f, "=======================================================\n");
    fprintf(f, "Numero de commande  : %s\n", ord->numero_commande);
    fprintf(f, "Acheteur            : %s %s (Login: %s)\n", usr->nom, usr->prenom, usr->login);
    fprintf(f, "Email / Tel         : %s / %s\n", usr->email, usr->telephone);
    fprintf(f, "Date de commande    : %s\n", ord->date_commande);
    fprintf(f, "Livraison prevue au : %s (Max 7j)\n", ord->date_prevue_livraison);
    fprintf(f, "-------------------------------------------------------\n");
    fprintf(f, "Produit             : %s (Code: %s)\n", prd->designation, prd->codeProduit);
    fprintf(f, "Quantite commandee  : %.2f %s\n", ord->quantite_commandee, prd->unite_mesure);
    fprintf(f, "Prix unitaire       : %.2f FCFA\n", prd->prix_unitaire);
    fprintf(f, "MONTANT TOTAL       : %.2f FCFA\n", ord->montant_total);
    fprintf(f, "Statut commande     : %s\n", ord->etat);
    fprintf(f, "=======================================================\n");
    fprintf(f, "Merci pour votre confiance avec AGRI-MANAGER !\n");

    fclose(f);

    printf("[RAPPORT GENERATED] Recu enregistre dans : %s\n", filename);
}

void create_order(void) {
    print_header("PASSER UNE COMMANDE / ACHAT");

    if (!g_is_logged_in) {
        printf("[ERREUR] Vous devez etre connecte pour passer une commande.\n");
        return;
    }

    int active_orders = count_active_orders_for_user(g_current_user.id);
    if (active_orders >= 3) {
        printf("\n[ERREUR CONTRAINTE] Limite atteinte ! Vous avez deja %d commandes actives non reglees.\n", active_orders);
        printf("Vous ne pouvez pas dépasser la limite de 3 commandes actives simultanées.\n");
        return;
    }

    list_products();

    int id_product = read_int("\nEntrez l'ID du produit a commander : ");
    Product prd;
    if (!find_product_by_id(id_product, &prd)) {
        printf("[ERREUR] Produit ID %d introuvable.\n", id_product);
        return;
    }

    if (prd.quantite_disponible <= 0) {
        printf("\n[INFORMATION] Ce produit est actuellement en RUPTURE DE STOCK (Stock dispo: 0).\n");
        printf("Si vous souhaitez réserver la récolte future, utilisez l'option 'Réservations de récoltes'.\n");
        return;
    }

    printf("\nProduit selectionne : %s | Prix unitaire: %.2f FCFA | Dispo: %.2f %s\n",
           prd.designation, prd.prix_unitaire, prd.quantite_disponible, prd.unite_mesure);

    double qty = read_double("Entrez la quantite a commander : ");
    if (qty <= 0) {
        printf("[ERREUR] La quantite doit etre superieure a 0.\n");
        return;
    }

    if (qty > prd.quantite_disponible) {
        printf("[ERREUR] Quantite insuffisante en stock ! Seulement %.2f %s disponible(s).\n",
               prd.quantite_disponible, prd.unite_mesure);
        return;
    }

    Order o;
    memset(&o, 0, sizeof(Order));
    o.id = get_next_order_id();

    char date_code[30];
    get_date_code(date_code, sizeof(date_code));
    snprintf(o.numero_commande, sizeof(o.numero_commande), "CMD_%s", date_code);

    o.idUtilisateur = g_current_user.id;
    o.idProduit = prd.id;
    o.quantite_commandee = qty;
    o.montant_total = qty * prd.prix_unitaire;

    get_current_date(o.date_commande, sizeof(o.date_commande));
    get_future_date_days(7, o.date_prevue_livraison, sizeof(o.date_prevue_livraison));
    strcpy(o.etat, "EN_COURS");

    FILE *f = fopen(FILE_ORDERS, "ab");
    if (!f) {
        printf("[ERREUR] Impossible de sauvegarder la commande.\n");
        return;
    }
    fwrite(&o, sizeof(Order), 1, f);
    fclose(f);

    update_product_stock(prd.id, -qty, 0);

    generate_order_receipt(&o, &g_current_user, &prd);

    char log_msg[200];
    snprintf(log_msg, sizeof(log_msg), "Commande %s du produit %s (Qte: %.2f, Total: %.2f FCFA)",
             o.numero_commande, prd.codeProduit, qty, o.montant_total);
    log_operation(g_current_user.login, log_msg);

    printf("\n[SUCCES] Commande %s validee avec succes !\n", o.numero_commande);
    printf("Montant total : %.2f FCFA. Date limite de livraison : %s\n", o.montant_total, o.date_prevue_livraison);
}

void list_orders(void) {
    print_header("LISTE DE TOUTES LES COMMANDES");

    FILE *f = fopen(FILE_ORDERS, "rb");
    if (!f) {
        printf("Aucune commande enregistree.\n");
        return;
    }

    Order o;
    printf("%-4s | %-20s | %-5s | %-5s | %-8s | %-12s | %-19s | %-10s\n",
           "ID", "NUMERO COMMANDE", "USER", "PROD", "QTE", "MONTANT FCFA", "DATE COMMANDE", "ETAT");
    printf("----------------------------------------------------------------------------------------------------\n");

    int count = 0;
    while (fread(&o, sizeof(Order), 1, f) == 1) {
        printf("%-4d | %-20s | %-5d | %-5d | %-8.1f | %-12.2f | %-19s | %-10s\n",
               o.id, o.numero_commande, o.idUtilisateur, o.idProduit,
               o.quantite_commandee, o.montant_total, o.date_commande, o.etat);
        count++;
    }
    fclose(f);
    printf("----------------------------------------------------------------------------------------------------\n");
    printf("Total: %d commande(s)\n", count);
}

void list_user_orders(int user_id) {
    print_header("MES COMMANDES");

    FILE *f = fopen(FILE_ORDERS, "rb");
    if (!f) {
        printf("Aucune commande enregistree.\n");
        return;
    }

    Order o;
    printf("%-4s | %-20s | %-5s | %-8s | %-12s | %-19s | %-10s\n",
           "ID", "NUMERO COMMANDE", "PROD", "QTE", "MONTANT FCFA", "DATE COMMANDE", "ETAT");
    printf("-------------------------------------------------------------------------------------------\n");

    int count = 0;
    while (fread(&o, sizeof(Order), 1, f) == 1) {
        if (o.idUtilisateur == user_id) {
            printf("%-4d | %-20s | %-5d | %-8.1f | %-12.2f | %-19s | %-10s\n",
                   o.id, o.numero_commande, o.idProduit,
                   o.quantite_commandee, o.montant_total, o.date_commande, o.etat);
            count++;
        }
    }
    fclose(f);
    printf("-------------------------------------------------------------------------------------------\n");
    printf("Total: %d commande(s)\n", count);
}
