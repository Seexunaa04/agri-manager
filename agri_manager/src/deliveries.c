#include "../include/deliveries.h"
#include "../include/orders.h"
#include "../include/products.h"
#include "../include/auth.h"
#include "../include/utils.h"
#include "../include/losses.h"
#include "../include/reports.h"

#define PENALTY_PER_DAY_DELAY 5000.0 // 5000 FCFA per day of delay

int get_next_delivery_id(void) {
    FILE *f = fopen(FILE_DELIVERIES, "rb");
    if (!f) return 1;
    Delivery d;
    int max_id = 0;
    while (fread(&d, sizeof(Delivery), 1, f) == 1) {
        if (d.idLivraison > max_id) max_id = d.idLivraison;
    }
    fclose(f);
    return max_id + 1;
}

void generate_delivery_receipt(const Delivery *del, const Order *ord, const User *usr, const Product *prd) {
    char date_code[30];
    get_date_code(date_code, sizeof(date_code));

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/DELIVERY_%s_%s.txt", DIR_REPORTS_DELIVERIES, date_code, usr->login);

    FILE *f = fopen(filename, "w");
    if (!f) return;

    fprintf(f, "=======================================================\n");
    fprintf(f, "               AGRI-MANAGER - RECU DE LIVRAISON        \n");
    fprintf(f, "=======================================================\n");
    fprintf(f, "ID Livraison        : LIV_%04d\n", del->idLivraison);
    fprintf(f, "Numero de commande  : %s\n", ord->numero_commande);
    fprintf(f, "Destinataire        : %s %s (Login: %s)\n", usr->nom, usr->prenom, usr->login);
    fprintf(f, "Date reelle de liv. : %s\n", del->date_reelle_livraison);
    fprintf(f, "Date prevue liv.    : %s\n", ord->date_prevue_livraison);
    fprintf(f, "-------------------------------------------------------\n");
    fprintf(f, "Produit livre       : %s (Code: %s)\n", prd->designation, prd->codeProduit);
    fprintf(f, "Quantite livree     : %.2f %s\n", ord->quantite_commandee, prd->unite_mesure);
    fprintf(f, "Jours de retard     : %d jour(s)\n", del->nb_jours_retard);
    fprintf(f, "Penalite appliquee  : %.2f FCFA\n", del->montant_penalite);
    fprintf(f, "=======================================================\n");
    fprintf(f, "Livraison confirmee avec succes !\n");

    fclose(f);

    printf("[RAPPORT GENERATED] Recu de livraison enregistre dans : %s\n", filename);
}

void process_delivery(void) {
    print_header("CONFIRMATION DE LIVRAISON ET RECEPTION");

    list_orders();

    int id_order = read_int("\nEntrez l'ID de la commande a livrer : ");
    Order ord;
    if (!find_order_by_id(id_order, &ord)) {
        printf("[ERREUR] Commande ID %d introuvable.\n", id_order);
        return;
    }

    if (strcmp(ord.etat, "LIVRÉ") == 0) {
        printf("[ATTENTION] Cette commande a deja ete livree !\n");
        return;
    }

    if (strcmp(ord.etat, "ANNULÉ") == 0) {
        printf("[ERREUR] Cette commande a ete annulee.\n");
        return;
    }

    User usr;
    if (!find_user_by_id(ord.idUtilisateur, &usr)) {
        strcpy(usr.login, "INCONNU");
        strcpy(usr.nom, "Inconnu");
        strcpy(usr.prenom, "Utilisateur");
    }

    Product prd;
    if (!find_product_by_id(ord.idProduit, &prd)) {
        strcpy(prd.designation, "Produit inconnu");
        strcpy(prd.codeProduit, "N/A");
        strcpy(prd.unite_mesure, "Unites");
    }

    Delivery del;
    memset(&del, 0, sizeof(Delivery));
    del.idLivraison = get_next_delivery_id();
    del.idCommande = ord.id;
    get_current_date(del.date_reelle_livraison, sizeof(del.date_reelle_livraison));

    // Calculate delay days comparing real delivery date with expected delivery date
    int days_diff = calculate_days_difference(ord.date_prevue_livraison, del.date_reelle_livraison);
    if (days_diff > 0) {
        del.nb_jours_retard = days_diff;
        del.montant_penalite = days_diff * PENALTY_PER_DAY_DELAY;
    } else {
        del.nb_jours_retard = 0;
        del.montant_penalite = 0.0;
    }

    // Save Delivery record
    FILE *fd = fopen(FILE_DELIVERIES, "ab");
    if (!fd) {
        printf("[ERREUR] Impossible d'ouvrir le fichier des livraisons.\n");
        return;
    }
    fwrite(&del, sizeof(Delivery), 1, fd);
    fclose(fd);

    // Update Order state to LIVRÉ
    FILE *fo = fopen(FILE_ORDERS, "rb+");
    if (fo) {
        Order tmp_o;
        while (fread(&tmp_o, sizeof(Order), 1, fo) == 1) {
            if (tmp_o.id == ord.id) {
                strcpy(tmp_o.etat, "LIVRÉ");
                fseek(fo, -((long)sizeof(Order)), SEEK_CUR);
                fwrite(&tmp_o, sizeof(Order), 1, fo);
                break;
            }
        }
        fclose(fo);
    }

    // Update total stock (deduct delivered volume from total stock)
    update_product_stock(prd.id, 0, -ord.quantite_commandee);

    // If there was a delay, record penalty in LOSSES.dat
    if (del.nb_jours_retard > 0) {
        Loss loss;
        memset(&loss, 0, sizeof(Loss));
        loss.idUtilisateur = ord.idUtilisateur;
        loss.idCommande = ord.id;
        loss.nb_jours_retard_ou_quantite_avariee = (double)del.nb_jours_retard;
        loss.montant_financier = del.montant_penalite;
        get_current_date(loss.date_enregistrement, sizeof(loss.date_enregistrement));

        record_loss(&loss);

        printf("\n[PENALITE APPLIQUEE] Retard de %d jour(s) detecte !\n", del.nb_jours_retard);
        printf("Pénalité financiere enregistree : %.2f FCFA (5000 FCFA / jour)\n", del.montant_penalite);
    }

    // Generate Delivery Receipt
    generate_delivery_receipt(&del, &ord, &usr, &prd);

    char log_msg[250];
    snprintf(log_msg, sizeof(log_msg), "Livraison effectuee pour la commande %s (Retard: %d jours, Penalite: %.2f FCFA)",
             ord.numero_commande, del.nb_jours_retard, del.montant_penalite);
    log_operation(g_current_user.login, log_msg);

    printf("\n[SUCCES] Livraison confirmee avec succes pour la commande %s !\n", ord.numero_commande);
}

void list_deliveries(void) {
    print_header("HISTORIQUE DES LIVRAISONS ET RECEPTIONS");

    FILE *f = fopen(FILE_DELIVERIES, "rb");
    if (!f) {
        printf("Aucune livraison enregistree.\n");
        return;
    }

    Delivery d;
    printf("%-5s | %-12s | %-19s | %-10s | %-15s\n",
           "ID LIV", "ID COMMANDE", "DATE LIVRAISON", "RETARD (J)", "PENALITE (FCFA)");
    printf("-----------------------------------------------------------------------\n");

    int count = 0;
    while (fread(&d, sizeof(Delivery), 1, f) == 1) {
        printf("%-5d | %-12d | %-19s | %-10d | %-15.2f\n",
               d.idLivraison, d.idCommande, d.date_reelle_livraison,
               d.nb_jours_retard, d.montant_penalite);
        count++;
    }
    fclose(f);
    printf("-----------------------------------------------------------------------\n");
    printf("Total: %d livraison(s)\n", count);
}
